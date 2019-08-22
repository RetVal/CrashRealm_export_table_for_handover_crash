//
//  RxScheduler+SubClasses.h
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <RxScheduler/RxScheduler.h>
#import <RxScheduler/RxSchedulerToken.h>

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXPORT NSString *const RxSchedulerCurrentSchedulerKey;

@interface RxScheduler ()

- (instancetype)initWithName:(nullable NSString *)name;

- (void)performAsCurrentScheduler:(dispatch_block_t)block;

@end

NS_ASSUME_NONNULL_END
