//
//  RxPrimaryKey.m
//  RxCoreComponents
//
//  Created by Closure on 9/4/17.
//  Copyright © 2017 Closure. All rights reserved.
//

#import <RxCoreComponents/RxPrimaryKey.h>

NSExceptionName const RxPrimaryKeyException = @"RxPrimaryKeyException";

@implementation NSString (PK)
- (NSString *)getInKey {
    return self;
}

- (RxIDType)numericIDKey {
    return (RxIDType)[[self getInKey] longLongValue];
}
@end

@implementation NSNumber (PK)
- (NSString *)getInKey {
    return [self description];
}

- (RxIDType)numericIDKey {
    return (RxIDType)[self longLongValue];
}
@end

@implementation RxIntPK

+ (instancetype)intPKWithID:(RxIDType)ID {
    return [[RxIntPK alloc] initWithID:ID];
}

- (instancetype)initWithID:(RxIDType)ID {
    if (self = [super init]) {
        _ID = ID;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    if (self = [super init]) {
        _ID = (RxIDType)[aDecoder decodeInt64ForKey:@"ID"];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeInt64:_ID forKey:@"ID"];
}

- (NSUInteger)hash {
#if __LP64__
    return _ID >> 32;
#else
    return _ID;
#endif
}

- (NSString *)getInKey {
    return [NSString stringWithFormat:@"%lld", (unsigned long long)_ID];
}

- (RxIDType)numericIDKey {
    return _ID;
}

- (BOOL)isEqual:(id)object {
    if ([object isKindOfClass:[self class]]) {
        return [((RxIntPK *)object) ID] == _ID;
    }
    return NO;
}

- (NSString *)description {
    return [self getInKey];
}
@end

@implementation RxStringPK

- (instancetype)initWithToken:(NSString *)token {
    if (self = [super init]) {
        _token = token;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    if (self = [super init]) {
        _token = [aDecoder decodeObjectForKey:@"token"];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeObject:_token forKey:@"token"];
}

- (NSUInteger)hash {
    return [_token hash];
}

- (NSString *)getInKey {
    return _token;
}

- (BOOL)isEqual:(id)object {
    if ([object isKindOfClass:[self class]]) {
        return [((RxStringPK *)object) token] == _token;
    }
    return NO;
}

- (RxIDType)numericIDKey {
    return [_token numericIDKey];
}

- (NSString *)description {
    return [self getInKey];
}
@end

@interface __RxIDArray : NSMutableArray<id<RxPrimaryKey>>
@property (strong, nonatomic, readonly) NSArray<id<RxPrimaryKey>> *values;
@end

@implementation __RxIDArray

+ (instancetype)arrayWithIDs:(NSArray<id<RxPrimaryKey>> *)keys {
    __RxIDArray *result = [[__RxIDArray alloc] init];
    result->_values = [keys copy];
    return result;
}

- (NSArray<id<RxPrimaryKey>> *)ids {
    return self;
}

- (NSArray<id<RxPrimaryKey>> *)getInKeys {
    return self;
}

- (id)copy {
    return self;
}

- (id)mutableCopy {
    return [[NSMutableArray alloc] initWithArray:self];
}

- (id)objectAtIndex:(NSUInteger)index {
    return _values[index];
}

- (NSUInteger)count {
    return [_values count];
}

@end

@implementation NSArray (PrimaryIDs)

- (NSArray<id<RxPrimaryKey>> *)ids {
    NSMutableArray *ids = [[NSMutableArray alloc] init];
    for (id object in self) {
        if ([object respondsToSelector:@selector(numericIDKey)]) {
            [ids addObject:@([object numericIDKey])];
        } else {
            [[NSException exceptionWithName:RxPrimaryKeyException reason:@"element not conform RxPrimaryKey protocol" userInfo:nil] raise];
            return nil;
        }
    }
    return [__RxIDArray arrayWithIDs:ids];
}

- (NSArray<NSString *> *)getInKeys {
    NSMutableArray *ids = [[NSMutableArray alloc] init];
    for (id object in self) {
        if ([object respondsToSelector:@selector(getInKey)]) {
            [ids addObject:[object getInKey]];
        } else {
            [[NSException exceptionWithName:RxPrimaryKeyException reason:@"element not conform RxPrimaryKey protocol" userInfo:nil] raise];
            return nil;
        }
    }
    return (NSArray<NSString *> *)[__RxIDArray arrayWithIDs:ids];
}

@end

@implementation NSString (RSJSONModel)

- (id)jsonObject {
    return self;
}

@end

@implementation NSNumber (RSJSONModel)

- (id)jsonObject {
    return self;
}

@end

@implementation NSArray (RSJSONModel)

- (id)jsonObject {
    NSMutableArray *jsonObjects = [[NSMutableArray alloc] initWithCapacity:[self count]];
    for (id<RxJSONModel> obj in self) {
        [jsonObjects addObject:[obj jsonObject]];
    }
    return jsonObjects;
}

@end

@implementation NSDictionary (RSJSONModel)

- (id)jsonObject {
    NSMutableDictionary *dict = [[NSMutableDictionary alloc] initWithCapacity:[self count]];
    [self enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
        dict[[key jsonObject]] = [obj jsonObject];
    }];
    return dict;
}

@end
