//
//  NSObject+DelayedPerforming.m
//  RxCoreComponents
//
//  Created by closure on 1/8/18.
//  Copyright © 2018 Closure. All rights reserved.
//

#import <RxCoreComponents/NSObject+RxDelayedPerforming.h>
#import <RxCoreComponents/NSObject+Deallocating.h>
#import <RxCoreComponents/NSObject+RxDynamicCast.h>
#import <RxCoreComponents/RxWeakProxy.h>

static NSString *const __RxDelayPerformWithWeakProxyThreadEntryKey = @"RxDelayPerformWithWeakProxy";

@interface RxActionWeakProxy : RxWeakProxy
@property (nonatomic, readonly) SEL action;
@property (nonatomic, readonly) id object;
+ (instancetype)actionWeakProxyWithTarget:(id<NSObject>)target action:(SEL)action object:(id)object;
- (void)invoke;
@end

@implementation RxActionWeakProxy

+ (instancetype)actionWeakProxyWithTarget:(id<NSObject>)target action:(SEL)action object:(id)object {
    RxActionWeakProxy *proxy = [RxActionWeakProxy proxyWithTarget:target];
    proxy->_action = action;
    proxy->_object = object;
    return proxy;
}

- (void)invoke {
    if (self.target) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored"-Warc-performSelector-leaks"
        [self.target performSelector:_action withObject:_object];
#pragma clang diagnostic pop
    }
}

@end

@implementation NSObject (RxDelayedPerforming)

+ (void)rx_cancelPreviousPerformRequestsWithTarget:(id)aTarget selector:(SEL)aSelector object:(id)anArgument {
    NSMutableDictionary *threadDictionary = [[NSThread currentThread] threadDictionary];
    NSMapTable *weakToStrongTargetMapTable = [threadDictionary[__RxDelayPerformWithWeakProxyThreadEntryKey] dynamicCast:[NSMapTable class]];
    if (!weakToStrongTargetMapTable) {
        return;
    }
    NSMapTable *selectorTable = [[weakToStrongTargetMapTable objectForKey:aTarget] dynamicCast:[NSMapTable class]];
    if (![selectorTable count]) {
        return;
    }
    NSString *key = NSStringFromSelector(aSelector);
    NSTimer *timer = [[selectorTable objectForKey:key] dynamicCast:[NSTimer class]];
    if (!timer) {
        return;
    }
    [timer invalidate];
    [selectorTable removeObjectForKey:key];
}

+ (void)rx_cancelPreviousPerformRequestsWithTarget:(id)aTarget {
    NSMutableDictionary *threadDictionary = [[NSThread currentThread] threadDictionary];
    NSMapTable *weakToStrongTargetMapTable = [threadDictionary[__RxDelayPerformWithWeakProxyThreadEntryKey] dynamicCast:[NSMapTable class]];
    if (!weakToStrongTargetMapTable) {
        return;
    }
    NSMapTable *selectorTable = [[weakToStrongTargetMapTable objectForKey:aTarget] dynamicCast:[NSMapTable class]];
    if (selectorTable) {
        [weakToStrongTargetMapTable removeObjectForKey:aTarget];
        if ([selectorTable count]) {
            @autoreleasepool {
                [[[selectorTable dictionaryRepresentation] allValues] makeObjectsPerformSelector:@selector(invalidate)];
            }
        }
    }
}

- (void)rx_performSelector:(SEL)aSelector withObject:(nullable id)anArgument afterDelay:(NSTimeInterval)delay inModes:(NSArray<NSRunLoopMode> *)modes {
    NSObject *target = self;
    if ([target _isDeallocating]) {
        return;
    }
    NSMutableDictionary *threadDictionary = [[NSThread currentThread] threadDictionary];
    NSMapTable *weakToStrongTargetMapTable = [threadDictionary[__RxDelayPerformWithWeakProxyThreadEntryKey] dynamicCast:[NSMapTable class]];
    if (!weakToStrongTargetMapTable) {
        weakToStrongTargetMapTable = [NSMapTable weakToStrongObjectsMapTable];
        threadDictionary[__RxDelayPerformWithWeakProxyThreadEntryKey] = weakToStrongTargetMapTable;
    }
    NSMapTable *selectorTable = [[weakToStrongTargetMapTable objectForKey:target] dynamicCast:[NSMapTable class]];
    if (!selectorTable) {
        selectorTable = [NSMapTable strongToWeakObjectsMapTable];
        [weakToStrongTargetMapTable setObject:selectorTable forKey:target];
    }
    
    NSString *key = NSStringFromSelector(aSelector);
    NSTimer *timer = [[selectorTable objectForKey:key] dynamicCast:[NSTimer class]];
    
    if (timer) {
        [timer invalidate];
    }
    RxActionWeakProxy *proxy = [RxActionWeakProxy actionWeakProxyWithTarget:target action:aSelector object:anArgument];
    timer = [NSTimer timerWithTimeInterval:delay target:proxy selector:@selector(invoke) userInfo:nil repeats:NO];
    [selectorTable setObject:timer forKey:key];
    NSRunLoop *rl = [NSRunLoop currentRunLoop];
    for (NSRunLoopMode rlm in modes) {
        [rl addTimer:timer forMode:rlm];
    }
}

- (void)rx_performSelector:(SEL)aSelector withObject:(id)anArgument afterDelay:(NSTimeInterval)delay {
    [self rx_performSelector:aSelector withObject:anArgument afterDelay:delay inModes:@[NSDefaultRunLoopMode]];
}

@end
