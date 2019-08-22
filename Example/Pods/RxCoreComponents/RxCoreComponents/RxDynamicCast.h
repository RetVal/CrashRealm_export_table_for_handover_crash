//
//  RxDynamicCast.h
//  RxCoreComponents
//
//  Created by closure on 2/7/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol RxDynamicCast <NSObject>
@required
- (nullable id)dynamicCast:(Class)cls;
@end

NS_ASSUME_NONNULL_END
