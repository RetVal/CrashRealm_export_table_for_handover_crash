//
//  RxMessage.h
//  CrashRealm_Example
//
//  Created by Closure on 2019/8/21.
//  Copyright © 2019 retval. All rights reserved.
//

#import <Realm/Realm.h>

NS_ASSUME_NONNULL_BEGIN

@interface RxMessage : RLMObject
@property NSString *ID;
@property NSString *text;
@property NSString *conversationID;
@end

RLM_ARRAY_TYPE(RxMessage);

NS_ASSUME_NONNULL_END
