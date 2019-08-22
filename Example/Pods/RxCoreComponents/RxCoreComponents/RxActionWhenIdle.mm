//
//  RxActionWhenIdle.m
//  RxCoreComponents
//
//  Created by Closure on 2019/5/24.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <RxCoreComponents/RxActionWhenIdle.h>

@interface RxActionWhenIdle ()

@property (strong, nonatomic) NSInvocation *invocation;

- (nullable instancetype)initWithInvocation:(NSInvocation *)invocation;

- (void)invoke;
- (void)invalidate;

@end

@implementation RxActionWhenIdle

+ (instancetype)actionWhenIdleWithTarget:(id)target selector:(SEL)selector object:(id)object {
    return [[self alloc] initWithTarget:target selector:selector object:object];
}

- (instancetype)initWithTarget:(id)target selector:(SEL)selector object:(id)object {
    NSMethodSignature *methodSignature = [target methodSignatureForSelector:selector];
    NSAssert([methodSignature numberOfArguments] <= 3, @"%@: method '%@' requires more than 1 argument", target, NSStringFromSelector(selector));
#ifndef DEBUG
    if ([methodSignature numberOfArguments] > 3) {
        return nil;
    }
#endif
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:methodSignature];
    if (!invocation) {
        return nil;
    }
    [invocation setTarget:target];
    [invocation setSelector:selector];
    if ([methodSignature numberOfArguments] == 3) {
        [invocation setArgument:(__bridge void *)object atIndex:2];
    }
    return [self initWithInvocation:invocation];
}

- (instancetype)initWithInvocation:(NSInvocation *)invocation {
    if (self = [super init]) {
        _invocation = invocation;
        [_invocation retainArguments];
        [self addObserverToRunLoop];
    }
    return self;
}

- (void)dealloc {
    _invocation = nil;
}

- (void)addObserverToRunLoop {
    CFRunLoopRef rl = CFRunLoopGetCurrent();
    NSParameterAssert(rl);
    CFRunLoopObserverRef idleObserver = CFRunLoopObserverCreateWithHandler(kCFAllocatorDefault, kCFRunLoopBeforeWaiting, NO, 0, ^(CFRunLoopObserverRef observer, CFRunLoopActivity activity) {
        [self invoke];
    });
    if (idleObserver == NULL) {
        return;
    }
    CFRunLoopAddObserver(rl, idleObserver, kCFRunLoopCommonModes);
    CFRelease(idleObserver);
    idleObserver = NULL;
}

- (void)invoke {
    NSInvocation *invocation = self.invocation;
    self.invocation = nil;
    [invocation invoke];
    invocation = nil;
}

- (BOOL)isValid {
    return self.invocation != nil ? YES : NO;
}

- (void)invalidate {
    self.invocation = nil;
}

@end
