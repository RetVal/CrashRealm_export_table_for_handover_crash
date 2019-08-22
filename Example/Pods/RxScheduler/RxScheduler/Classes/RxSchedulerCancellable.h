//
//  RxSchedulerToken.h
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//

#import <RxCoreComponents/RxCoreComponents.h>

NS_ASSUME_NONNULL_BEGIN

@protocol RxSchedulerCancellable <RxCancellable>
@property (assign, readonly, getter=isCancelled) BOOL cancelled;
+ (instancetype)whenCancelled:(dispatch_block_t)block;
@end

NS_ASSUME_NONNULL_END
