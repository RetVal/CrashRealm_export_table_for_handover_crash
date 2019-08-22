//
//  RxImmediateScheduler.m
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//

#import "RxImmediateScheduler.h"
#import <RxScheduler/RxScheduler+SubClasses.h>

@implementation RxImmediateScheduler

- (instancetype)init {
    return [super initWithName:@"com.rx.scheduler.immediate"];
}

- (id<RxSchedulerCancellable>)schedule:(dispatch_block_t)block {
    NSCParameterAssert(block != nil);
    block();
    return nil;
}

- (id<RxSchedulerCancellable>)after:(NSDate *)date schedule:(dispatch_block_t)block {
    NSCParameterAssert(block != nil);
    [NSThread sleepUntilDate:date];
    block();
    return nil;
}

- (id<RxSchedulerCancellable>)after:(NSDate *)date repeatingEvery:(NSTimeInterval)interval withLeeway:(NSTimeInterval)leeway schedule:(dispatch_block_t)block {
    NSCAssert(NO, @"%@ was not supported by %@.", NSStringFromSelector(_cmd), NSStringFromClass(self.class));
    return nil;
}

@end
