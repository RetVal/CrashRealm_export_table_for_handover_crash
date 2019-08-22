//
//  RxPair.h
//  RxCoreComponents
//
//  Created by closure on 2/21/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxPair<FirstType, SecondType> : NSObject
@property (strong, nonatomic, nullable) FirstType first;
@property (strong, nonatomic, nullable) SecondType second;
+ (instancetype)pairWithFirst:(FirstType _Nullable)first second:(SecondType _Nullable)second;
- (instancetype)initWithFirst:(FirstType _Nullable)first second:(SecondType _Nullable)second;
- (instancetype)init NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
