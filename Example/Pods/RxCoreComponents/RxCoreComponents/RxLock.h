//
//  RxLock.h
//  RxCoreComponents
//
//  Created by closure on 3/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxLock : NSObject <NSLocking>

@property (strong, nonatomic, readonly, nullable) NSString *name;

- (instancetype)init;
- (void)withCriticalScope:(NS_NOESCAPE void(^)(void))block;

@end

NS_ASSUME_NONNULL_END
