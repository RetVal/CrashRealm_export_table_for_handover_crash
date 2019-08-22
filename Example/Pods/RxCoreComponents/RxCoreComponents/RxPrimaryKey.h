//
//  RxPrimaryKey.h
//  RxCoreComponents
//
//  Created by Closure on 9/4/17.
//  Copyright © 2017 Closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXPORT NSExceptionName const RxPrimaryKeyException;

typedef int64_t RxIDType;

@protocol RxPrimaryKey <NSObject>

@required
- (NSString *)getInKey;

@optional
- (RxIDType)numericIDKey;

@end

@interface NSString (PK) <RxPrimaryKey>
@end

@interface NSNumber (PK) <RxPrimaryKey>
@end

@interface RxIntPK : NSObject<RxPrimaryKey>
@property (assign, nonatomic) RxIDType ID;
+ (instancetype)intPKWithID:(RxIDType)ID;
- (instancetype)initWithID:(RxIDType)ID;
- (instancetype)initWithCoder:(NSCoder *)aDecoder;
- (void)encodeWithCoder:(NSCoder *)aCoder;
- (NSUInteger)hash;
- (NSString *)getInKey;
- (BOOL)isEqual:(nullable id)object;
@end

@interface RxStringPK : NSObject<RxPrimaryKey>
@property (strong, nonatomic) NSString *token;
- (instancetype)initWithToken:(NSString *)token;
- (instancetype)initWithCoder:(NSCoder *)aDecoder;
- (void)encodeWithCoder:(NSCoder *)aCoder;
- (NSUInteger)hash;
- (NSString *)getInKey;
- (BOOL)isEqual:(nullable id)object;
@end

@interface NSArray (PrimaryIDs)
- (NSArray<id<RxPrimaryKey>> *)ids;
- (NSArray<NSString *> *)getInKeys;
@end

@protocol RxJSONModel <NSObject>
@optional
- (id)jsonObject;
@end

@interface NSNumber (RSJSONModel) <RxJSONModel>
- (id)jsonObject;
@end

@interface NSString (RSJSONModel) <RxJSONModel>
- (id)jsonObject;
@end

@interface NSArray (RSJSONModel) <RxJSONModel>
- (id)jsonObject;
@end

@interface NSDictionary (RSJSONModel) <RxJSONModel>
- (id)jsonObject;
@end


NS_ASSUME_NONNULL_END
