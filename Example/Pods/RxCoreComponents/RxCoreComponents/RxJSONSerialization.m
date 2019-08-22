//
//  RxJSONSerialization.m
//  RxCoreComponents
//
//  Created by closure on 2/21/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <RxCoreComponents/RxJSONSerialization.h>

@implementation RxJSONSerialization
+ (id)JSONObjectWithData:(NSData *)data options:(NSJSONReadingOptions)opt error:(NSError *__autoreleasing *)error {
    return [self __JSONObjectWithData:data options:opt error:error];
}

+ (id)__JSONObjectWithData:(NSData *)data options:(NSJSONReadingOptions)opt error:(NSError *__autoreleasing *)error {
    id x = [super JSONObjectWithData:data options:opt error:error];
    if (!x) return nil;
    if ([x isKindOfClass:[NSDictionary class]]) {
        x = [self filterDictionary:x options:opt];
    } else if ([x isKindOfClass:[NSArray class]]) {
        x = [self filterArray:x options:opt];
    }
    return x;
}

+ (id)filterDictionary:(id)dict options:(NSJSONReadingOptions)opt {
    NSMutableDictionary *md = [[NSMutableDictionary alloc] init];
    [dict enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        if (!([key isKindOfClass:[NSNull class]] || [obj isKindOfClass:[NSNull class]])) {
            if ([obj isKindOfClass:[NSDictionary class]]) {
                id v = [self filterDictionary:obj options:opt];
                if (v)
                    md[key] = v;
            } else if ([obj isKindOfClass:[NSArray class]]) {
                id v = [self filterArray:obj options:opt];
                if (v)
                    md[key] = v;
            } else {
                md[key] = obj;
            }
        }
    }];
    if ((opt & NSJSONReadingMutableContainers) == NSJSONReadingMutableContainers) {
        return md;
    }
    return [md copy];
}

+ (id)filterArray:(id)array options:(NSJSONReadingOptions)opt {
    NSMutableArray *ma = [[NSMutableArray alloc] init];
    [array enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if (![obj isKindOfClass:[NSNull class]]) {
            if ([obj isKindOfClass:[NSArray class]]) {
                id v = [self filterArray:obj options:opt];
                if (v) [ma addObject:v];
            } else if ([obj isKindOfClass:[NSDictionary class]]) {
                id v = [self filterDictionary:obj options:opt];
                if (v) [ma addObject:v];
            } else {
                [ma addObject:obj];
            }
        }
    }];
    if ((opt & NSJSONReadingMutableContainers) == NSJSONReadingMutableContainers) {
        return ma;
    }
    return [ma copy];
}

@end
