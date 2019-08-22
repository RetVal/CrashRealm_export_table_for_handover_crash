//
//  NSObject+DelayedPerforming.h
//  RxCoreComponents
//
//  Created by closure on 1/8/18.
//  Copyright © 2018 Closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSObject (RxDelayedPerforming)

- (void)rx_performSelector:(SEL)aSelector withObject:(nullable id)anArgument afterDelay:(NSTimeInterval)delay inModes:(NSArray<NSRunLoopMode> *)modes;
- (void)rx_performSelector:(SEL)aSelector withObject:(nullable id)anArgument afterDelay:(NSTimeInterval)delay;
+ (void)rx_cancelPreviousPerformRequestsWithTarget:(id)aTarget selector:(SEL)aSelector object:(nullable id)anArgument;
+ (void)rx_cancelPreviousPerformRequestsWithTarget:(id)aTarget;

@end

NS_ASSUME_NONNULL_END
