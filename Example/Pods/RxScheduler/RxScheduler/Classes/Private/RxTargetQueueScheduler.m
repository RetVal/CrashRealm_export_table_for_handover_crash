//
//  RxTargetQueueScheduler.m
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//  Copyright © 2019 Closure. All rights reserved.
//

#import "RxTargetQueueScheduler.h"
#import <RxScheduler/RxQueueScheduler+SubClasses.h>

@implementation RxTargetQueueScheduler

- (instancetype)initWithName:(NSString *)name targetQueue:(dispatch_queue_t)targetQueue {
    NSCParameterAssert(targetQueue != NULL);
    
    if (name == nil) {
        name = [NSString stringWithFormat:@"com.rx.scheduler.RxTargetQueueScheduler(%s)", dispatch_queue_get_label(targetQueue)];
    }
    
    dispatch_queue_t queue = dispatch_queue_create(name.UTF8String, DISPATCH_QUEUE_SERIAL);
    if (queue == NULL) {
        return nil;
    }
    
    dispatch_set_target_queue(queue, targetQueue);
    
    return [super initWithName:name queue:queue];
}

@end
