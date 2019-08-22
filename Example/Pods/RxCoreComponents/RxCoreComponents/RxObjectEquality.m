//
//  RxObjectEquality.m
//  RxCoreComponents
//
//  Created by Closure on 2018/11/23.
//  Copyright © 2018 Closure. All rights reserved.
//

#import <RxCoreComponents/RxObjectEquality.h>
#import <objc/runtime.h>

typedef BOOL (*EqualFuncType)(id, SEL, id);

static EqualFuncType NSStringEqualIMP = nil;
static EqualFuncType NSDataEqualIMP = nil;
static EqualFuncType NSDateEqualIMP = nil;
static EqualFuncType NSNumberEqualIMP = nil;
static EqualFuncType NSValueEqualIMP = nil;
static EqualFuncType NSArrayEqualIMP = nil;
static EqualFuncType NSSetEqualIMP = nil;
static EqualFuncType NSDictionaryEqualIMP = nil;

static Class NSStringClass = nil;
static Class NSDataClass = nil;
static Class NSDateClass = nil;
static Class NSNumberClass = nil;
static Class NSValueClass = nil;
static Class NSArrayClass = nil;
static Class NSSetClass = nil;
static Class NSDictionaryClass = nil;

BOOL RxObjectIsEqual(id val0, id val1) {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSStringClass = [NSString class];
        NSDataClass = [NSData class];
        NSDateClass = [NSDate class];
        NSNumberClass = [NSNumber class];
        NSValueClass = [NSValue class];
        NSArrayClass = [NSArray class];
        NSSetClass = [NSSet class];
        NSDictionaryClass = [NSDictionary class];
        
        NSStringEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSStringClass, @selector(isEqualToString:)));
        NSDataEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSDataClass, @selector(isEqualToData:)));
        NSDateEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSDateClass, @selector(isEqualToDate:)));
        NSNumberEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSNumberClass, @selector(isEqualToNumber:)));
        NSValueEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSValueClass, @selector(isEqualToValue:)));
        NSArrayEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSArrayClass, @selector(isEqualToArray:)));
        NSSetEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSSetClass, @selector(isEqualToSet:)));
        NSDictionaryEqualIMP = (EqualFuncType)method_getImplementation(class_getInstanceMethod(NSDictionaryClass, @selector(isEqualToDictionary:)));
    });
    if (val0 == val1) {
        return YES;
    }
    {
        BOOL isNil0 = val0 == nil;
        BOOL isNil1 = val1 == nil;
        if (isNil0 != isNil1) {
            return NO;
        }
    }
    Class class0 = [val0 class];
    Class class1 = [val1 class];
    
    BOOL isKindOf = [class0 isSubclassOfClass:class1];
    if (!isKindOf) {
        if (![class1 isSubclassOfClass:class0]) {
            return NO;
        }
        isKindOf = YES;
        id v = val1;
        val1 = val0;
        val0 = v;
        
        v = class1;
        class1 = class0;
        class0 = v;
    }
    // class0 is subClassOf class1
    EqualFuncType imp = nil;
    if (class1 == NSStringClass) {
        imp = NSStringEqualIMP;
    } else if (class1 == NSNumberClass) {
        imp = NSNumberEqualIMP;
    } else if (class1 == NSValueClass) {
        imp = NSValueEqualIMP;
    } else if (class1 == NSDateClass) {
        imp = NSDateEqualIMP;
    } else if (class1 == NSArrayClass) {
        imp = NSArrayEqualIMP;
    } else if (class1 == NSSetClass) {
        imp = NSSetEqualIMP;
    } else if (class1 == NSDataClass) {
        imp = NSDataEqualIMP;
    } else if (class1 == NSDictionaryClass) {
        imp = NSDictionaryEqualIMP;
    }
    if (imp) {
        return imp(val0, @selector(isEqual:), val1);
    }
    return [val0 isEqual:val1];
}
