//
//  RxLock.m
//  RxCoreComponents
//
//  Created by closure on 3/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <RxCoreComponents/RxLock.h>
#include <RxFoundation/RxFoundation.h>

@interface RxLock () {
@private
    Rx::MutexLock _mutex;
}
@end

@implementation RxLock

- (instancetype)init {
    if (self = [super init]) {
    }
    return self;
}

- (void)withCriticalScope:(NS_NOESCAPE void (^)(void))block {
    Rx::LockGuard<Rx::MutexLock> lock(_mutex);
    block();
}

- (void)lock {
    _mutex.lock();
}

- (void)unlock {
    _mutex.unlock();
}

@end
