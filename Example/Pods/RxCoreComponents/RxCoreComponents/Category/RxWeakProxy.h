//
//  RxWeakProxy.h
//  RxCoreComponents
//
//  Created by Closure on 9/20/17.
//  Copyright © 2017 Closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxWeakProxy : NSProxy
+ (instancetype)proxyWithTarget:(nullable id<NSObject>)target;
- (instancetype)initWithTarget:(nullable id<NSObject>)target;
- (nullable id<NSObject>)target;
@end

NS_ASSUME_NONNULL_END
