//
//  RxPair.m
//  RxCoreComponents
//
//  Created by closure on 2/21/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <RxCoreComponents/RxPair.h>

@implementation RxPair
+ (instancetype)pairWithFirst:(id)first second:(id)second {
    return [[self alloc] initWithFirst:first second:second];
}

- (instancetype)initWithFirst:(id)first second:(id)second {
    if (self = [super init]) {
        _first = first;
        _second = second;
    }
    return self;
}
@end
