//
//  RxMessage.m
//  CrashRealm_Example
//
//  Created by Closure on 2019/8/21.
//  Copyright © 2019 retval. All rights reserved.
//

#import "RxMessage.h"

@implementation RxMessage

+ (NSString *)primaryKey {
    return @"ID";
}

- (instancetype)init {
    if (self = [super init]) {
        self.ID = [[NSUUID UUID] UUIDString];
        self.text = [[NSUUID UUID] UUIDString];
    }
    return self;
}

@end
