//
//  RxTuple.m
//  RxCoreComponents
//
//  Created by Closure on 2019/3/27.
//  Copyright © 2019 Closure. All rights reserved.
//w

#import <RxCoreComponents/RxTuple.h>

#ifndef keypath
    #define keypath(a, b) \
        (((void)(NO && ((void)a.b, NO)), # b))
#endif

@implementation RxTupleNull

+ (instancetype)null {
    static dispatch_once_t onceToken;
    static RxTupleNull *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (nonnull id)copyWithZone:(nullable NSZone *)zone {
    return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder {
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    return [self.class null];
}

@end

@interface RxTuple ()

@property (strong, nonatomic, readonly) NSArray *backingStore;

- (instancetype)__initWithBackingStore:(NSArray *)backingStore NS_DESIGNATED_INITIALIZER;

@end

@implementation RxTuple

- (instancetype)init {
    return [self __initWithBackingStore:@[]];
}

- (instancetype)__initWithBackingStore:(NSArray *)backingStore {
    if (self = [super init]) {
        _backingStore = [backingStore copy];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    if (self = [self init]) {
        _backingStore = [aDecoder decodeObjectForKey:@keypath(self, backingStore)];
    }
    return self;
}

- (void)encodeWithCoder:(nonnull NSCoder *)aCoder {
    if (![_backingStore count]) {
        return;
    }
    [aCoder encodeObject:_backingStore forKey:@keypath(self, backingStore)];
}

+ (instancetype)tupleWithObjectsFromArray:(NSArray *)array {
    return [self tupleWithObjectsFromArray:array convertNullsToNils:NO];
}

+ (instancetype)tupleWithObjectsFromArray:(NSArray *)array convertNullsToNils:(BOOL)convert {
    if (!convert) {
        return [[self alloc] __initWithBackingStore:array];
    }
    
    NSMutableArray *newArray = [NSMutableArray arrayWithCapacity:array.count];
    for (id object in array) {
        [newArray addObject:(object == [NSNull null] ? [RxTupleNull null] : object)];
    }
    
    return [[self alloc] __initWithBackingStore:newArray];
}

+ (instancetype)tupleWithObjects:(id)object, ... {
    va_list args;
    va_start(args, object);
    
    NSUInteger count = 0;
    for (id currentObject = object; currentObject != nil; currentObject = va_arg(args, id)) {
        ++count;
    }
    
    va_end(args);
    
    if (count == 0) {
        return [[self alloc] init];
    }
    
    NSMutableArray *objects = [[NSMutableArray alloc] initWithCapacity:count];
    
    va_start(args, object);
    for (id currentObject = object; currentObject != nil; currentObject = va_arg(args, id)) {
        [objects addObject:currentObject];
    }
    
    va_end(args);
    
    return [[self alloc] __initWithBackingStore:objects];
}

- (id)objectAtIndex:(NSUInteger)index {
    if (index >= self.count) {
        return nil;
    }

    id object = self.backingStore[index];
    return (object == [RxTupleNull null] ? nil : object);
}

- (NSArray *)allObjects {
    NSMutableArray *newArray = [NSMutableArray arrayWithCapacity:self.backingStore.count];
    for (id object in self.backingStore) {
        [newArray addObject:(object == [RxTupleNull null] ? [NSNull null] : object)];
    }
    
    return newArray;
}

- (instancetype)tupleByAddingObject:(id)obj {
    NSArray *newArray = [self.backingStore arrayByAddingObject:obj ?: [RxTupleNull null]];
    return [self.class tupleWithObjectsFromArray:newArray];
}

- (NSUInteger)count {
    return self.backingStore.count;
}

- (id)first {
    return self[0];
}

- (id)second {
    return self[1];
}

- (id)third {
    return self[2];
}

- (id)fourth {
    return self[3];
}

- (id)fifth {
    return self[4];
}

- (id)last {
    return self[self.count - 1];
}

- (id)objectAtIndexedSubscript:(NSUInteger)idx {
    return [self objectAtIndex:idx];
}

- (nonnull id)copyWithZone:(nullable NSZone *)zone {
    return self;
}

- (NSUInteger)countByEnumeratingWithState:(nonnull NSFastEnumerationState *)state objects:(id  _Nullable __unsafe_unretained * _Nonnull)buffer count:(NSUInteger)len {
    return [self.backingStore countByEnumeratingWithState:state objects:buffer count:len];
}

@end

@implementation RxOneTuple

- (instancetype)init {
    return [self __initWithBackingStore:@[[RxTupleNull null]]];
}

- (instancetype)__initWithBackingStore:(NSArray *)backingStore {
    NSParameterAssert(backingStore.count == 1);
    return [super __initWithBackingStore:backingStore];
}

- (RxTwoTuple *)tupleByAddingObject:(nullable id)obj {
    return [RxTwoTuple tupleWithObjectsFromArray:@[[self.backingStore arrayByAddingObject:obj ?: [RxTupleNull null]]]];
}

/// Creates a new tuple with the given values.
+ (instancetype)pack:(id)first {
    return [self tupleWithObjectsFromArray:@[first ?: [RxTupleNull null]]];
}

- (BOOL)isEqual:(RxTuple *)object {
    if (object == self) {
        return YES;
    }
    if (![object isKindOfClass:[RxTuple class]]) {
        return NO;
    }
    return [self.backingStore isEqual:object.backingStore];
}

@dynamic first;

@end

@implementation RxTwoTuple

- (instancetype)init {
    return [self __initWithBackingStore:@[[RxTupleNull null], [RxTupleNull null]]];
}

- (instancetype)__initWithBackingStore:(NSArray *)backingStore {
    NSParameterAssert(backingStore.count == 2);
    return [super __initWithBackingStore:backingStore];
}

- (RxThreeTuple *)tupleByAddingObject:(id)obj {
    return [RxThreeTuple tupleWithObjectsFromArray:[self.backingStore arrayByAddingObject:obj ?: [RxTupleNull null]]];
}

+ (instancetype)pack:(id)first :(id)second {
    return [self tupleWithObjectsFromArray:@[first ?: [RxTupleNull null], second ?: [RxTupleNull null]]];
}

- (BOOL)isEqual:(RxTuple *)object {
    if (object == self) {
        return YES;
    }
    if (![object isKindOfClass:[RxTuple class]]) {
        return NO;
    }
    return [self.backingStore isEqual:object.backingStore];
}

@dynamic first;
@dynamic second;

@end

@implementation RxThreeTuple

- (instancetype)init {
    return [self __initWithBackingStore:@[[RxTupleNull null], [RxTupleNull null], [RxTupleNull null]]];
}

- (instancetype)__initWithBackingStore:(NSArray *)backingStore {
    NSParameterAssert(backingStore.count == 3);
    return [super __initWithBackingStore:backingStore];
}

- (RxFourTuple *)tupleByAddingObject:(id)obj {
    return [RxFourTuple tupleWithObjectsFromArray:[self.backingStore arrayByAddingObject:obj ?: [RxTupleNull null]]];
}

+ (instancetype)pack:(id)first :(id)second :(id)third {
    return [self tupleWithObjectsFromArray:@[first ?: [RxTupleNull null], second ?: [RxTupleNull null], third ?: [RxTupleNull null]]];
}

- (BOOL)isEqual:(RxTuple *)object {
    if (object == self) {
        return YES;
    }
    if (![object isKindOfClass:[RxTuple class]]) {
        return NO;
    }
    return [self.backingStore isEqual:object.backingStore];
}

@dynamic first;
@dynamic second;
@dynamic third;

@end

@implementation RxFourTuple

- (instancetype)init {
    return [self __initWithBackingStore:@[[RxTupleNull null], [RxTupleNull null], [RxTupleNull null], [RxTupleNull null]]];
}

- (instancetype)__initWithBackingStore:(NSArray *)backingStore {
    NSParameterAssert(backingStore.count == 4);
    return [super __initWithBackingStore:backingStore];
}

- (RxFiveTuple *)tupleByAddingObject:(id)obj {
    return [RxFiveTuple tupleWithObjectsFromArray:[self.backingStore arrayByAddingObject:obj ?: [RxTupleNull null]]];
}

+ (instancetype)pack:(id)first :(id)second :(id)third :(id)fourth {
    return [self tupleWithObjectsFromArray:@[first ?: [RxTupleNull null], second ?: [RxTupleNull null], third ?: [RxTupleNull null], fourth ?: [RxTupleNull null]]];
}

- (BOOL)isEqual:(RxTuple *)object {
    if (object == self) {
        return YES;
    }
    if (![object isKindOfClass:[RxTuple class]]) {
        return NO;
    }
    return [self.backingStore isEqual:object.backingStore];
}

@dynamic first;
@dynamic second;
@dynamic third;
@dynamic fourth;

@end

@implementation RxFiveTuple

- (instancetype)init {
    return [self __initWithBackingStore:@[[RxTupleNull null], [RxTupleNull null], [RxTupleNull null], [RxTupleNull null], [RxTupleNull null]]];
}

- (instancetype)__initWithBackingStore:(NSArray *)backingStore {
    NSParameterAssert(backingStore.count == 5);
    return [super __initWithBackingStore:backingStore];
}

+ (instancetype)pack:(id)first :(id)second :(id)third :(id)fourth :(id)fifth {
    return [self tupleWithObjectsFromArray:@[first ?: [RxTupleNull null], second ?: [RxTupleNull null], third ?: [RxTupleNull null], fourth ?: [RxTupleNull null] , fifth ?: [RxTupleNull null]]];
}

- (BOOL)isEqual:(RxTuple *)object {
    if (object == self) {
        return YES;
    }
    if (![object isKindOfClass:[RxTuple class]]) {
        return NO;
    }
    return [self.backingStore isEqual:object.backingStore];
}

@dynamic first;
@dynamic second;
@dynamic third;
@dynamic fourth;
@dynamic fifth;

@end

@implementation RxTupleUnpackingTrampoline

#pragma mark Lifecycle

+ (instancetype)trampoline {
    static dispatch_once_t onceToken;
    static id trampoline = nil;
    dispatch_once(&onceToken, ^{
        trampoline = [[self alloc] init];
    });
    
    return trampoline;
}

- (void)setObject:(RxTuple *)tuple forKeyedSubscript:(NSArray *)variables {
    NSCParameterAssert(variables != nil);
    
    [variables enumerateObjectsUsingBlock:^(NSValue *value, NSUInteger index, BOOL *stop) {
        __strong id *ptr = (__strong id *)value.pointerValue;
        *ptr = tuple[index];
    }];
}

@end
