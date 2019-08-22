//
//  RxSharedObjectsCache.m
//  RxCoreComponents
//
//  Created by Closure on 8/14/18.
//  Copyright © 2018 Closure. All rights reserved.
//

#import "RxSharedObjectsCache.h"

@interface RxSharedObjectsCache ()

@property (strong, nonatomic) NSMutableDictionary<NSString *, NSMutableOrderedSet<id> *> *cache;

@end

static NSString *__RxNSStringFromClass(Class cls) {
    NSString *className = nil;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"
    if ([cls respondsToSelector:@selector(className)]) {
        className = [cls performSelector:@selector(className)];
    } else {
        className = NSStringFromClass(cls);
    }
#pragma clang diagnostic pop
    return className;
}

@implementation RxSharedObjectsCache

- (instancetype)init {
    if (self = [super init]) {
        _cache = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (NSOrderedSet *)allObjectsWithClass:(Class)cls {
    NSString *className = __RxNSStringFromClass(cls);
    NSMutableOrderedSet *objects = self.cache[className];
    if (![objects count]) {
        return [NSOrderedSet orderedSet];
    }
    return [objects copy];
}

- (void)addObject:(id<RxPrimaryKey>)object {
    if (!object) {
        return;
    }
    NSString *className = __RxNSStringFromClass([object class]);
    NSMutableOrderedSet *objects = self.cache[className];
    if (!objects) {
        objects = [[NSMutableOrderedSet alloc] init];
        self.cache[className] = objects;
    }
    [objects addObject:object];
}

- (void)removeObject:(id<RxPrimaryKey>)object {
    if (!object) {
        return;
    }
    NSString *className = __RxNSStringFromClass([object class]);
    NSMutableOrderedSet *objects = self.cache[className];
    if ([objects count] == 0) {
        return;
    }
    [objects removeObject:object];
}

@end
