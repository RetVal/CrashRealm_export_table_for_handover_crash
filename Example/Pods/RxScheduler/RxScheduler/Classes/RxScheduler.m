//
//  RxScheduler.m
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <RxScheduler/RxScheduler.h>
#import <RxScheduler/RxScheduler+SubClasses.h>
#import <RxScheduler/RxRunLoopScheduler+SubClasses.h>

#import "RxMainQueueScheduler.h"
#import "RxTargetQueueScheduler.h"
#import "RxImmediateScheduler.h"

NSString *const RxSchedulerCurrentSchedulerKey = @"RxSchedulerCurrentSchedulerKey";

CF_INLINE RxScheduler *_Nullable RxSchedulerGetCurrent(void) {
    return [[NSThread currentThread] threadDictionary][RxSchedulerCurrentSchedulerKey];
}

CF_INLINE void RxSchedulerSetCurrent(RxScheduler *_Nullable  scheduler) {
    [[NSThread currentThread] threadDictionary][RxSchedulerCurrentSchedulerKey] = scheduler;
}

@interface RxScheduler ()
@property (strong, nonatomic, readonly) NSString *name;
@end

@implementation RxScheduler

+ (BOOL)supportRunLoop {
    return NO;
}

+ (BOOL)isOnMainThread {
    return [NSOperationQueue.currentQueue isEqual:NSOperationQueue.mainQueue] || [NSThread isMainThread];
}

+ (instancetype)immediateScheduler {
    static dispatch_once_t onceToken;
    static RxScheduler *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [[RxImmediateScheduler alloc] init];
    });
    return instance;
}

+ (instancetype)mainScheduler {
    static dispatch_once_t onceToken;
    static RxScheduler *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [[RxMainQueueScheduler alloc] initWithName:@"com.rx.scheduler.main"];
    });
    return instance;
}

+ (instancetype)currentScheduler {
    RxScheduler *scheduler = RxSchedulerGetCurrent();
    if (scheduler) {
        return scheduler;
    }
    if ([self isOnMainThread]) {
        return [self mainScheduler];
    }
    return nil;
}

+ (instancetype)schedulerWithPriority:(RxSchedulerPriority)priority {
    return [self schedulerWithPriority:priority
                                  name:@"com.rx.scheduler.background"];
}

+ (instancetype)schedulerWithPriority:(RxSchedulerPriority)priority
                                 name:(NSString *)name {
    return [[RxTargetQueueScheduler alloc] initWithName:name targetQueue:dispatch_get_global_queue(priority, 0)];
}

+ (instancetype)scheduler {
    return [self schedulerWithPriority:RxSchedulerPriorityDefault];
}

+ (instancetype)schedulerWithRunLoop:(NSRunLoop *)runloop {
    return [self schedulerWithRunLoop:runloop name:@"com.rx.scheduler.rl"];
}

+ (instancetype)schedulerWithRunLoop:(NSRunLoop *)runloop name:(NSString *)name {
    return [[RxRunLoopScheduler alloc] initWithName:name runloop:runloop];
}

- (instancetype)initWithName:(NSString *)name {
    if (self = [super init]) {
        _name = [name copy];
    }
    return self;
}

- (id<RxSchedulerCancellable>)schedule:(dispatch_block_t)block {
    NSCAssert(NO, @"%@ must be implemented by subclasses.", NSStringFromSelector(_cmd));
    return nil;
}

- (id<RxSchedulerCancellable>)afterDelay:(NSTimeInterval)delay schedule:(dispatch_block_t)block {
    return [self after:[NSDate dateWithTimeIntervalSinceNow:delay] schedule:block];
}

- (nullable id<RxSchedulerCancellable>)after:(NSDate *)date schedule:(dispatch_block_t)block {
    NSCAssert(NO, @"%@ must be implemented by subclasses.", NSStringFromSelector(_cmd));
    return nil;
}

- (id<RxSchedulerCancellable>)after:(NSDate *)date repeatingEvery:(NSTimeInterval)interval withLeeway:(NSTimeInterval)leeway schedule:(dispatch_block_t)block {
    NSCAssert(NO, @"%@ must be implemented by subclasses.", NSStringFromSelector(_cmd));
    return nil;
}

- (void)performAsCurrentScheduler:(dispatch_block_t)block {
    NSCParameterAssert(block != NULL);
    RxScheduler *previousScheduler = RxSchedulerGetCurrent();
    RxSchedulerSetCurrent(self);
    @autoreleasepool {
        block();
    }
    RxSchedulerSetCurrent(previousScheduler);
}

- (NSString *)description {
    return [NSString stringWithFormat:@"%@(%@)", [super description], _name];
}

@end
