//
//  RxRunLoopScheduler+SubClasses.h
//  RxScheduler
//
//  Created by Closure on 2019/4/11.
//  Copyright © 2019 retval. All rights reserved.
//

#import <RxScheduler/RxRunLoopScheduler.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxRunLoopScheduler ()

@property (strong, nonatomic, readonly) NSRunLoop *runloop;


/**
 create a runloop based scheduler

 @param name the name of scheduler
 @param runloop the based runloop, if pass nil, scheduler will create an internal runloop
 @return return the instance of scheduler
 */
- (instancetype)initWithName:(nullable NSString *)name
                     runloop:(nullable NSRunLoop *)runloop NS_DESIGNATED_INITIALIZER;

@end

NS_ASSUME_NONNULL_END
