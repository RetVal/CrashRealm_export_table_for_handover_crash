//
//  RxSchedulerToken.h
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//

#import <RxScheduler/RxSchedulerCancellable.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxSchedulerToken : NSObject <RxSchedulerCancellable>

@property (assign, readonly, getter=isCancelled) BOOL cancelled;

@end

NS_ASSUME_NONNULL_END
