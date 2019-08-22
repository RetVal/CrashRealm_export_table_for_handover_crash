//
//  RxSharedObjectsCache.h
//  RxCoreComponents
//
//  Created by Closure on 8/14/18.
//  Copyright © 2018 Closure. All rights reserved.
//

#import <RxCoreComponents/RxPrimaryKey.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxSharedObjectsCache : NSObject

- (NSOrderedSet *)allObjectsWithClass:(Class)cls;

- (void)addObject:(id<RxPrimaryKey>)object;
- (void)removeObject:(id<RxPrimaryKey>)object;

@end

NS_ASSUME_NONNULL_END
