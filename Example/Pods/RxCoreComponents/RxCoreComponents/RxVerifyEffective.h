//
//  RxVerifyEffective.h
//  RxCoreComponents
//
//  Created by Closure on 2018/11/6.
//  Copyright © 2018 Closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol RxVerifyEffective <NSObject>
@required

- (BOOL)verifyEffective:(BOOL)fullyValidation;

@end

NS_ASSUME_NONNULL_END
