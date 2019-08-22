//
//  NSObject+RxDynamicCast.m
//  RxCoreComponents
//
//  Created by closure on 2/7/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import "NSObject+RxDynamicCast.h"

@implementation NSObject (RxDynamicCast)
- (id<RxDynamicCast>)dynamicCast:(Class)cls {
    return [self isKindOfClass:cls] ? self : nil;
}
@end
