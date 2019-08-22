//
//  RxComparable.h
//  RxCoreComponents
//
//  Created by Closure on 2019/4/2.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol RxComparable <NSObject>

- (NSComparisonResult)compare:(id)object;

@end

@interface NSString (RxComparable) <RxComparable>
@end

@interface NSNumber (RxComparable) <RxComparable>
@end

@interface NSDate (RxComparable) <RxComparable>
@end

@interface NSDecimalNumber (RxComparable) <RxComparable>
@end

@interface NSValue (RxComparable) <RxComparable>
@end

NS_ASSUME_NONNULL_END
