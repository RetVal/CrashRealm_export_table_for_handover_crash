//
//  RxWeakProxy.m
//  RxCoreComponents
//
//  Created by Closure on 9/20/17.
//  Copyright © 2017 Closure. All rights reserved.
//

#import <RxCoreComponents/RxWeakProxy.h>

@interface RxWeakProxy ()
@property (weak, nonatomic, readonly) id<NSObject> target;
@end

@implementation RxWeakProxy

+ (instancetype)proxyWithTarget:(nullable id<NSObject>)target {
    return [[self alloc] initWithTarget:target];
}

- (instancetype)initWithTarget:(nullable id<NSObject>)target {
    _target = target;
    return self;
}

- (Class)class {
    return [_target class];
}

- (Class)superclass {
    return [_target superclass];
}

- (BOOL)isEqual:(id)object {
    return [_target isEqual:object];
}

- (NSUInteger)hash {
    return [_target hash];
}

- (instancetype)self {
    return _target;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"

- (id)performSelector:(SEL)aSelector {
    return [_target performSelector:aSelector];
}

- (id)performSelector:(SEL)aSelector withObject:(id)object {
    return [_target performSelector:aSelector withObject:object];
}

- (id)performSelector:(SEL)aSelector withObject:(id)object1 withObject:(id)object2 {
    return [_target performSelector:aSelector withObject:object1 withObject:object2];
}

#pragma clang diagnostic pop

- (BOOL)isProxy {
    return YES;
}

- (BOOL)isKindOfClass:(Class)aClass {
    return [_target isKindOfClass:aClass];
}

- (BOOL)isMemberOfClass:(Class)aClass {
    return [_target isMemberOfClass:aClass];
}

- (BOOL)conformsToProtocol:(Protocol *)aProtocol {
    return [_target conformsToProtocol:aProtocol];
}

- (BOOL)respondsToSelector:(SEL)aSelector {
    return [_target respondsToSelector:aSelector];
}

- (NSString *)description {
    return [_target description];
}

- (NSString *)debugDescription {
    return [_target description];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel {
    return [NSObject instanceMethodSignatureForSelector:@selector(init)];
}

- (void)forwardInvocation:(NSInvocation *)invocation {
    void *nullValue = nil;
    [invocation setReturnValue:&nullValue];
}

- (id)forwardingTargetForSelector:(SEL)aSelector {
    return _target;
}

@end
