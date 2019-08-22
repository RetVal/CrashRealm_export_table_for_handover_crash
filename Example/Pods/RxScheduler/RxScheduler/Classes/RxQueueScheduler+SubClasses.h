//
//  RxQueueScheduler+SubClasses.h
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <RxScheduler/RxQueueScheduler.h>
#import <RxScheduler/RxScheduler+SubClasses.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxQueueScheduler ()

@property (strong, nonatomic, readonly) dispatch_queue_t queue;

- (instancetype)initWithName:(nullable NSString *)name queue:(dispatch_queue_t)queue;

@end

NS_ASSUME_NONNULL_END
