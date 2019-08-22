//
//  RxRunLoopScheduler.mm
//  RxScheduler
//
//  Created by Closure on 2019/4/11.
//  Copyright © 2019 retval. All rights reserved.
//

#import <RxScheduler/RxRunLoopScheduler.h>
#import <RxScheduler/RxRunLoopScheduler+SubClasses.h>

#import <RxScheduler/RxScheduler+SubClasses.h>
#import <RxScheduler/RxSchedulerToken.h>

#include <RxFoundation/RxFoundation.h>

#include <pthread/pthread.h>
#include <mach/thread_act.h>

// These two functions are declared in mach/thread_policy.h, but are commented out.
// They are documented here: https://developer.apple.com/library/mac/#releasenotes/Performance/RN-AffinityAPI/_index.html

RxExtern kern_return_t thread_policy_set(thread_t thread,
                                         thread_policy_flavor_t flavor,
                                         thread_policy_t policy_info,
                                         mach_msg_type_number_t count);

RxExtern kern_return_t thread_policy_get(thread_t thread,
                                         thread_policy_flavor_t flavor,
                                         thread_policy_t policy_info,
                                         mach_msg_type_number_t *count,
                                         boolean_t *get_default);

RxExtern void *objc_autoreleasePoolPush(void);

RxExtern void objc_autoreleasePoolPop(void *pool);

static NSString *const RxRunLoopSchedulerAutoleasePoolStackKey = @"RxRunLoopSchedulerAutoleasePoolStackKey";

static const void *PoolStackRetainCallBack(CFAllocatorRef allocator, const void *value) {
    return value;
}

static void PoolStackReleaseCallBack(CFAllocatorRef allocator, const void *value) {
}

static inline void RxRunLoopSchedulerAutoreleasePoolPush() {
    NSMutableDictionary *dictionary =  [[NSThread currentThread] threadDictionary];
    CFMutableArrayRef poolStack = (__bridge CFMutableArrayRef)dictionary[RxRunLoopSchedulerAutoleasePoolStackKey];
    
    if (!poolStack) {
        /*
         do not retain pool on push,
         but release on pop to avoid memory analyze warning
         */
        CFArrayCallBacks callbacks = {0};
        callbacks.retain = PoolStackRetainCallBack;
        callbacks.release = PoolStackReleaseCallBack;
        poolStack = CFArrayCreateMutable(CFAllocatorGetDefault(), 0, &callbacks);
        dictionary[RxRunLoopSchedulerAutoleasePoolStackKey] = (__bridge id)poolStack;
        CFRelease(poolStack);
    }
    void *pool = objc_autoreleasePoolPush(); // create and push
    CFArrayAppendValue(poolStack, pool);
}

static inline void RxRunLoopSchedulerAutoreleasePoolPop() {
    NSMutableDictionary *dictionary =  [[NSThread currentThread] threadDictionary];
    CFMutableArrayRef poolStack = (__bridge CFMutableArrayRef)dictionary[RxRunLoopSchedulerAutoleasePoolStackKey];
    CFIndex count = CFArrayGetCount(poolStack);
    if (count == 0) {
        return;
    }
    void *pool = const_cast<void *>(CFArrayGetValueAtIndex(poolStack, count - 1));
    CFArrayRemoveValueAtIndex(poolStack, count - 1);
    objc_autoreleasePoolPop(pool);
}

static void RxRunLoopSchedulerAutoreleasePoolObserverCallBack(CFRunLoopObserverRef observer,
                                                              CFRunLoopActivity activity,
                                                              void *info) {
    switch (activity) {
        case kCFRunLoopEntry: {
            RxRunLoopSchedulerAutoreleasePoolPush();
            break;
        }
        case kCFRunLoopBeforeWaiting: {
            RxRunLoopSchedulerAutoreleasePoolPop();
            RxRunLoopSchedulerAutoreleasePoolPush();
            break;
        }
        case kCFRunLoopExit: {
            RxRunLoopSchedulerAutoreleasePoolPop();
            break;
        }
        default: break;
    }
}

@interface __RxRunLoopSchedulerTimerTask : NSObject
@property (strong, nonatomic, readonly) RxScheduler *scheduler;
@property (strong, nonatomic, readonly) id<RxSchedulerCancellable> token;
@property (copy, nonatomic, readonly) dispatch_block_t block;

- (instancetype)initWithScheduler:(RxScheduler *)scheduler token:(id<RxSchedulerCancellable>)token block:(dispatch_block_t)block;

@end

@implementation __RxRunLoopSchedulerTimerTask

- (instancetype)initWithScheduler:(RxScheduler *)scheduler token:(id<RxSchedulerCancellable>)token block:(dispatch_block_t)block {
    if (self = [super init]) {
        _scheduler = scheduler;
        _token = token;
        _block = block;
    }
    return self;
}

- (void)invoke {
    if ([self.token isCancelled]) {
        return;
    }
    [self.scheduler performAsCurrentScheduler:self.block];
}

@end


@interface RxRunLoopScheduler () {
@public
    Rx::SpinLock _lock;
    dispatch_semaphore_t _semaphore;
    NSRunLoop *_runloop;
    CFRunLoopRef _cfRunLoop;
    pthread_t _pthread;
    NSThread *_thread;
    CFRunLoopSourceRef _wakeupSource;
    CFRunLoopObserverRef _autoreleasePoolPushObserver;
    CFRunLoopObserverRef _autoreleasePoolPopObserver;
    //    CFRunLoopSourceContext _sourceContext;
    //    std::atomic_bool _quitWhenIdleReceived;
}
//@property (weak, nonatomic, readonly) id<RxInjector> injector;
@property (assign, nonatomic, readonly) CFRunLoopRef cfRunLoop;

- (void)signal;

@end

typedef struct __RxRunLoopSchedulerSourceInfo {
    std::atomic_int rc;
    //    id<RxInjector> injector;
    __weak RxRunLoopScheduler *worker;
} __RxRunLoopSchedulerSourceInfo;

static const void *__RxRunLoopSchedulerSourceRetain(const void *info) {
    __RxRunLoopSchedulerSourceInfo *sourceInfo = reinterpret_cast<__RxRunLoopSchedulerSourceInfo *>(const_cast<void *>(info));
    sourceInfo->rc++;
    return info;
}

static void __RxRunLoopSchedulerSourceRelease(const void *info) {
    __RxRunLoopSchedulerSourceInfo *sourceInfo = reinterpret_cast<__RxRunLoopSchedulerSourceInfo *>(const_cast<void *>(info));
    if (sourceInfo->rc == 1) {
        sourceInfo->worker = nullptr;
        free(sourceInfo);
    } else {
        sourceInfo->rc--;
    }
}

static void __RxRunLoopSchedulerSourceSchedule(void *info,
                                               CFRunLoopRef rl,
                                               CFRunLoopMode mode) {
}

static void __RxRunLoopSchedulerSourceCancel(void *info,
                                             CFRunLoopRef rl,
                                             CFRunLoopMode mode) {
}

static void __RxRunLoopSchedulerSourcePerform(void *info) {
    __RxRunLoopSchedulerSourceInfo *sourceInfo = reinterpret_cast<__RxRunLoopSchedulerSourceInfo *>(info);
    if (!sourceInfo) {
        return;
    }
    RxRunLoopScheduler *self = sourceInfo->worker;
    if (!self) {
        return;
    }
    @autoreleasepool {
        [self signal];
    }
}

RX_INLINE_VISIBILITY
CFRunLoopSourceRef
__RxRunLoopSchedulerCreateKeepAliveSource(RxRunLoopScheduler *scheduler) RX_NOEXCEPT {
    CFRunLoopSourceContext sourceContext = {0};
    sourceContext.version = 0;
    sourceContext.schedule = __RxRunLoopSchedulerSourceSchedule;
    sourceContext.cancel = __RxRunLoopSchedulerSourceCancel;
    sourceContext.perform = __RxRunLoopSchedulerSourcePerform;
    sourceContext.retain = __RxRunLoopSchedulerSourceRetain;
    sourceContext.release = __RxRunLoopSchedulerSourceRelease;
    __RxRunLoopSchedulerSourceInfo *info = reinterpret_cast<__RxRunLoopSchedulerSourceInfo *>(malloc(sizeof(__RxRunLoopSchedulerSourceInfo)));
    __builtin_memset((void *)info, 0, sizeof(__RxRunLoopSchedulerSourceInfo));
    // info->injector = self.injector;
    __weak RxRunLoopScheduler *weakScheduler = scheduler;
    info->worker = weakScheduler;
    scheduler = nil;
    info->rc = 0; // release
    sourceContext.info = info;
    
    CFRunLoopSourceRef source = CFRunLoopSourceCreate(kCFAllocatorDefault,
                                                      0,
                                                      &sourceContext);
    return source;
}

RX_INLINE_VISIBILITY
void
__RxRunLoopSchedulerCreateAutoreleasePoolObserver(CFRunLoopObserverRef &pushObserver,
                                                  CFRunLoopObserverRef &popObserver) RX_NOEXCEPT {
    pushObserver = CFRunLoopObserverCreate(CFAllocatorGetDefault(),
                                           kCFRunLoopEntry,
                                           true,         // repeat
                                           -0x7FFFFFFF,  // before other observers
                                           RxRunLoopSchedulerAutoreleasePoolObserverCallBack,
                                           nullptr);
    
    popObserver = CFRunLoopObserverCreate(CFAllocatorGetDefault(),
                                          kCFRunLoopBeforeWaiting | kCFRunLoopExit,
                                          true,        // repeat
                                          0x7FFFFFFF,  // after other observers
                                          RxRunLoopSchedulerAutoreleasePoolObserverCallBack,
                                          nullptr);
    return;
}


static void *__RxRunLoopSchedulerRunLoopEntry(void *ctx) RX_NOEXCEPT __attribute__((noinline));

static void *__RxRunLoopSchedulerRunLoopEntry(void *ctx) RX_NOEXCEPT {
    CFRunLoopRef runloop = CFRunLoopGetCurrent();
    BOOL isMain = pthread_main_np() > 0;
    __strong RxRunLoopScheduler *scheduler = (__bridge RxRunLoopScheduler *)ctx;
    @autoreleasepool {
        Rx::LockGuard<decltype(scheduler->_lock)> lock(scheduler->_lock);
        scheduler->_pthread = pthread_self();
        scheduler->_cfRunLoop = (CFRunLoopRef)CFRetain(runloop);
        scheduler->_runloop = [NSRunLoop currentRunLoop];
        NSThread *currentThread = [NSThread currentThread];
        scheduler->_thread = currentThread;
        
        if (!isMain) {
            NSString *name = [scheduler description];
            pthread_setname_np([name UTF8String]);
            [currentThread setName:name];
            [currentThread threadDictionary][RxSchedulerCurrentSchedulerKey] = scheduler;
        }
        
        // config source context
        {
            scheduler->_wakeupSource = __RxRunLoopSchedulerCreateKeepAliveSource(scheduler);
            CFRunLoopAddSource(runloop, scheduler->_wakeupSource, kCFRunLoopDefaultMode);
        }
        
        // autoreleasepool observer
        if (!isMain) {
            __RxRunLoopSchedulerCreateAutoreleasePoolObserver(scheduler->_autoreleasePoolPushObserver, scheduler->_autoreleasePoolPopObserver);
            if (scheduler->_autoreleasePoolPushObserver && scheduler->_autoreleasePoolPopObserver) {
                CFRunLoopAddObserver(runloop, scheduler->_autoreleasePoolPushObserver, kCFRunLoopCommonModes);
                CFRunLoopAddObserver(runloop, scheduler->_autoreleasePoolPopObserver, kCFRunLoopCommonModes);
            }
        }
        dispatch_semaphore_t sema = scheduler->_semaphore;
        CFRunLoopPerformBlock(runloop, kCFRunLoopDefaultMode, ^{
            dispatch_semaphore_signal(sema);
        });
    }
    if (!isMain) {
        scheduler = nil; // release the strong reference.
        int32_t result = 0;
        do {
            result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e10, true);
        } while (kCFRunLoopRunStopped != result && kCFRunLoopRunFinished != result);
    }
    return nullptr;
}

@implementation RxRunLoopScheduler

+ (BOOL)supportRunLoop {
    return YES;
}

- (instancetype)initWithName:(nullable NSString *)name
                     runloop:(NSRunLoop *)runloop {
    if (self = [super initWithName:name]) {
        _semaphore = dispatch_semaphore_create(0);
        if (runloop != nil) {
            Rx::LockGuard<decltype(self->_lock)> lock(self->_lock);
            _runloop = runloop;
            _cfRunLoop = (CFRunLoopRef)CFRetain([self->_runloop getCFRunLoop]);
            if ([runloop isEqual:[NSRunLoop currentRunLoop]]) {
                self->_pthread = pthread_self();
                self->_thread = [NSThread currentThread];
                dispatch_semaphore_signal(self->_semaphore);
            } else {
                CFRunLoopPerformBlock(_cfRunLoop, kCFRunLoopDefaultMode, ^{
                    self->_pthread = pthread_self();
                    self->_thread = [NSThread currentThread];
                    dispatch_semaphore_signal(self->_semaphore);
                });
            }
        } else {
            pthread_t pthread;
            pthread_attr_t pthreadAttribute;
            pthread_attr_init(&pthreadAttribute);
            //            if (pthread_create_suspended_np(&pthread, &pthreadAttribute,
            //                                            __RxRunLoopSchedulerRunLoopEntry,
            //                                            (__bridge void *)self)) {
            //                abort();
            //            }
            //            thread_t mach_thread = pthread_mach_thread_np(pthread);
            //            thread_affinity_policy_data_t policyData = {
            //                .affinity_tag = 2,
            //            };
            //            thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policyData, 1);
            //            thread_resume(mach_thread);
            if (pthread_create(&pthread, &pthreadAttribute, __RxRunLoopSchedulerRunLoopEntry, (__bridge void *)self)) {
                abort();
            }
        }
        dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
        
        {
            Rx::LockGuard<decltype(self->_lock)> lock(self->_lock);
            NSAssert(self->_cfRunLoop != nil, @"cfRunLoop is nil!");
            NSAssert(self->_runloop != nil, @"runloop is nil!");
        }
    }
    return self;
}

- (void)dealloc {
    if (_autoreleasePoolPushObserver) {
        CFRelease(_autoreleasePoolPushObserver);
        _autoreleasePoolPushObserver = nullptr;
    }
    
    if (_autoreleasePoolPopObserver) {
        CFRelease(_autoreleasePoolPopObserver);
        _autoreleasePoolPopObserver = nullptr;
    }
    if (_wakeupSource != nil) {
        //        CFRunLoopStop(_cfRunLoop); // stop atomatically when all sources removed
        CFRunLoopSourceRef source = _wakeupSource;
        _wakeupSource = nil;
        if (!pthread_equal(pthread_self(), _pthread)) {
            CFRunLoopSourceInvalidate(source);
            CFRelease(source);
            CFRunLoopRef rl = (CFRunLoopRef)CFRetain(_cfRunLoop);
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                CFRunLoopPerformBlock(rl, kCFRunLoopDefaultMode, ^{
                });
                CFRunLoopWakeUp(rl);
                CFRelease(rl);
            });
        } else {
            CFRunLoopSourceInvalidate(source);
            CFRelease(source);
            CFRunLoopWakeUp(_cfRunLoop);
        }
        CFRelease(_cfRunLoop);
        _cfRunLoop = nil;
    }
    _runloop = nil;
    _thread = nil;
    _pthread = nil;
}

- (id<RxSchedulerCancellable>)schedule:(dispatch_block_t)block {
    NSParameterAssert(block != nil);
    CFRunLoopRef rl = nil;
    BOOL runOnSameThread = NO;
    do {
        Rx::LockGuard<decltype(_lock)> lock(_lock);
        runOnSameThread = pthread_equal(pthread_self(), _pthread) != 0;
        if (!_cfRunLoop) {
            return nil;
        }
        rl = (CFRunLoopRef)CFRetain(_cfRunLoop);
    } while (0);
    id<RxSchedulerCancellable> token = [RxSchedulerToken new];
    if (runOnSameThread) {
        [self performAsCurrentScheduler:block];
    } else {
        CFRunLoopPerformBlock(rl, kCFRunLoopDefaultMode, ^{
            if ([token isCancelled]) {
                return;
            }
            [self performAsCurrentScheduler:block];
        });
        CFRunLoopWakeUp(rl);
        CFRelease(rl);
    }
    return token;
}

- (nullable id<RxSchedulerCancellable>)after:(NSDate *)date schedule:(dispatch_block_t)block {
    NSParameterAssert(date != nil);
    NSParameterAssert(block != nil);
    __block NSTimer *timer = nil;
    id<RxSchedulerCancellable> token = [RxSchedulerToken whenCancelled:^{
        [timer invalidate];
    }];
    timer = [[NSTimer alloc] initWithFireDate:date interval:0 target:[[__RxRunLoopSchedulerTimerTask alloc] initWithScheduler:(RxScheduler *)[RxWeakProxy proxyWithTarget:self] token:token block:block] selector:@selector(invoke) userInfo:nil repeats:NO];
    {
        Rx::LockGuard<decltype(_lock)> lock(_lock);
        [self.runloop addTimer:timer forMode:NSRunLoopCommonModes];
    }
    return token;
}

- (id<RxSchedulerCancellable>)after:(NSDate *)date repeatingEvery:(NSTimeInterval)interval withLeeway:(NSTimeInterval)leeway schedule:(dispatch_block_t)block {
    NSParameterAssert(date != nil);
    NSParameterAssert(block != nil);
    __block NSTimer *timer = nil;
    id<RxSchedulerCancellable> token = [RxSchedulerToken whenCancelled:^{
        [timer invalidate];
    }];
    __weak id<RxSchedulerCancellable> weakToken = token;
    // just ignored the leeway when using NSTimer!
    timer = [[NSTimer alloc] initWithFireDate:date interval:interval target:[[__RxRunLoopSchedulerTimerTask alloc] initWithScheduler:(RxScheduler *)[RxWeakProxy proxyWithTarget:self] token:token block:block] selector:@selector(invoke) userInfo:nil repeats:YES];
    {
        Rx::LockGuard<decltype(_lock)> lock(_lock);
        [self.runloop addTimer:timer forMode:NSRunLoopCommonModes];
    }
    return token;
}

- (void)signal {
}

@end
