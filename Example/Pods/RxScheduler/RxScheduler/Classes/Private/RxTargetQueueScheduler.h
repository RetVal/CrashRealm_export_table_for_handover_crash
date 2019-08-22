//
//  RxTargetQueueScheduler.h
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <RxScheduler/RxQueueScheduler.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxTargetQueueScheduler : RxQueueScheduler

- (instancetype)initWithName:(NSString *)name targetQueue:(dispatch_queue_t)targetQueue;

@end

NS_ASSUME_NONNULL_END
