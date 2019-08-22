//
//  RxMainQueueScheduler.m
//  RxScheduler
//
//  Created by Closure on 2019/4/19.
//

#import "RxMainQueueScheduler.h"

#include <pthread/pthread.h>

@implementation RxMainQueueScheduler

+ (BOOL)supportRunLoop {
    return YES;
}

- (instancetype)initWithName:(NSString *)name {
    if (self = [super initWithName:name queue:dispatch_get_main_queue()]) {
    }
    return self;
}

- (id<RxSchedulerCancellable>)schedule:(dispatch_block_t)block {
    NSParameterAssert(block);
    RxSchedulerToken *token = [RxSchedulerToken new];
    if (pthread_main_np()) {
        [self performAsCurrentScheduler:block];
    } else {
        dispatch_async(self.queue, ^{
            if ([token isCancelled]) {
                return;
            }
            [self performAsCurrentScheduler:block];
        });
    }
    return token;
}

@end
