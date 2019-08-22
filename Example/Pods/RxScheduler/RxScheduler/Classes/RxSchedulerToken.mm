//
//  RxSchedulerToken.m
//  RxScheduler
//
//  Created by Closure on 2019/3/31.
//

#import <RxScheduler/RxSchedulerToken.h>
#include <RxFoundation/Atomic.hpp>

@interface RxSchedulerToken () {
    Rx::SpinLock _spin;
    dispatch_block_t _block;
    BOOL _cancelled;
}
@end

@implementation RxSchedulerToken

+ (instancetype)whenCancelled:(dispatch_block_t)block {
    RxSchedulerToken *token = [[self alloc] init];
    token->_block = [block copy];
    return token;
}

- (instancetype)init {
    if (self = [super init]) {
        _cancelled = NO;
    }
    return self;
}

- (void)dealloc {
    [self cancel];
}

- (BOOL)isCancelled {
    Rx::LockGuard<decltype(_spin)> lock(_spin);
    return _cancelled;
}

- (void)cancel {
    Rx::LockGuard<decltype(_spin)> lock(_spin);
    if (_cancelled) {
        return;
    }
    _cancelled = YES;
    if (_block) {
        _block();
        _block = nil;
    }
}

@end
