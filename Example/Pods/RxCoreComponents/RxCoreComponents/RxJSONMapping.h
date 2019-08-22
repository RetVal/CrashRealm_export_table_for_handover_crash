//
//  RxJSONMapping.h
//  RxCoreComponents
//
//  Created by closure on 2/7/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <RxCoreComponents/RxSharedObjectsCache.h>

@protocol RxJSONMapping <NSObject>

@required

+ (id)mappingFromDictionary:(NSDictionary<NSString *, id> *)dictionary
         sharedObjectsCache:(RxSharedObjectsCache *)sharedObjectsCache;

@optional

- (id)jsonObject;

@end
