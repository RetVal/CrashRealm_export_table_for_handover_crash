//
//  RxQueueScheduler.m
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <RxScheduler/RxQueueScheduler.h>
#import <RxScheduler/RxQueueScheduler+SubClasses.h>
#import <RxScheduler/RxSchedulerToken.h>
#include <RxFoundation/Atomic.hpp>

RX_INLINE bool RxQueueSchedulerIsCurrentQueue(RxQueueScheduler *scheduler, dispatch_queue_t q) {
    return dispatch_get_specific((__bridge void *)scheduler) == (__bridge void *)q;
}

@implementation RxQueueScheduler

- (instancetype)initWithName:(NSString *)name queue:(dispatch_queue_t)queue {
    if (self = [super initWithName:name]) {
        _queue = queue;
        dispatch_queue_set_specific(_queue, (__bridge void *)self,
                                    (__bridge void *)_queue, nil);
    }
    return self;
}

- (void)dealloc {
    _queue = nil;
}

+ (dispatch_time_t)wallTimeWithDate:(NSDate *)date {
    NSCParameterAssert(date != nil);
    
    double seconds = 0;
    double frac = modf(date.timeIntervalSince1970, &seconds);
    
    struct timespec walltime = {
        .tv_sec = (time_t)fmin(fmax(seconds, LONG_MIN), LONG_MAX),
        .tv_nsec = (long)fmin(fmax(frac * NSEC_PER_SEC, LONG_MIN), LONG_MAX)
    };
    
    return dispatch_walltime(&walltime, 0);
}

- (id<RxSchedulerCancellable>)schedule:(dispatch_block_t)block {
    NSParameterAssert(block != nil);
    RxSchedulerToken *token = [RxSchedulerToken new];
    if (RxQueueSchedulerIsCurrentQueue(self, _queue)) {
        [self performAsCurrentScheduler:block];
    } else {
        dispatch_async(_queue, ^{
            if ([token isCancelled]) {
                return;
            }
            [self performAsCurrentScheduler:block];
        });
    }
    return token;
}

- (id<RxSchedulerCancellable>)after:(NSDate *)date schedule:(dispatch_block_t)block {
    NSCParameterAssert(date != nil);
    NSCParameterAssert(block != NULL);
    
    id<RxSchedulerCancellable> token = [RxSchedulerToken new];
    
    dispatch_after([self.class wallTimeWithDate:date], self.queue, ^{
        if ([token isCancelled]) {
            return;
        }
        [self performAsCurrentScheduler:block];
    });
    
    return token;
}

- (id<RxSchedulerCancellable>)after:(NSDate *)date repeatingEvery:(NSTimeInterval)interval withLeeway:(NSTimeInterval)leeway schedule:(dispatch_block_t)block {
    NSCParameterAssert(date != nil);
    NSCParameterAssert(interval > 0.0 && interval < INT64_MAX / NSEC_PER_SEC);
    NSCParameterAssert(leeway >= 0.0 && leeway < INT64_MAX / NSEC_PER_SEC);
    NSCParameterAssert(block != NULL);
    
    uint64_t intervalInNanoSecs = (uint64_t)(interval * NSEC_PER_SEC);
    uint64_t leewayInNanoSecs = (uint64_t)(leeway * NSEC_PER_SEC);
    
    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, self.queue);
    dispatch_source_set_timer(timer, [self.class wallTimeWithDate:date], intervalInNanoSecs, leewayInNanoSecs);
    
    __weak __typeof__(timer) weakTimer = timer;
    RxSchedulerToken *token = [RxSchedulerToken whenCancelled:^{
        __typeof__(timer) timer = weakTimer;
        if (!timer) {
            return;
        }
        dispatch_source_cancel(timer);
    }];
    
    dispatch_source_set_event_handler(timer, ^{
        if ([token isCancelled]) {
            return;
        }
        block();
    });
    dispatch_resume(timer);
    
    return token;
}

@end
