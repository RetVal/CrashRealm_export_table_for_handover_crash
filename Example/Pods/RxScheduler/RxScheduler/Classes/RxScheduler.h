//
//  RxScheduler.h
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <RxScheduler/RxSchedulerCancellable.h>

NS_ASSUME_NONNULL_BEGIN

typedef enum : long {
    RxSchedulerPriorityHigh = DISPATCH_QUEUE_PRIORITY_HIGH,
    RxSchedulerPriorityDefault = DISPATCH_QUEUE_PRIORITY_DEFAULT,
    RxSchedulerPriorityLow = DISPATCH_QUEUE_PRIORITY_LOW,
    RxSchedulerPriorityBackground = DISPATCH_QUEUE_PRIORITY_BACKGROUND,
} RxSchedulerPriority;

@interface RxScheduler : NSObject

+ (BOOL)supportRunLoop;

+ (instancetype)immediateScheduler;

+ (instancetype)mainScheduler;

+ (instancetype)schedulerWithPriority:(RxSchedulerPriority)priority;

+ (instancetype)schedulerWithPriority:(RxSchedulerPriority)priority
                                 name:(NSString *)name;

+ (instancetype)scheduler;

+ (nullable instancetype)currentScheduler;

+ (instancetype)schedulerWithRunLoop:(nullable NSRunLoop *)runloop;

+ (instancetype)schedulerWithRunLoop:(nullable NSRunLoop *)runloop
                                name:(NSString *)name;

- (nullable id<RxSchedulerCancellable>)schedule:(dispatch_block_t)block;

- (nullable id<RxSchedulerCancellable>)afterDelay:(NSTimeInterval)delay schedule:(dispatch_block_t)block;

- (nullable id<RxSchedulerCancellable>)after:(NSDate *)date schedule:(dispatch_block_t)block;

- (id<RxSchedulerCancellable>)after:(NSDate *)date repeatingEvery:(NSTimeInterval)interval withLeeway:(NSTimeInterval)leeway schedule:(dispatch_block_t)block;

@end

NS_ASSUME_NONNULL_END
