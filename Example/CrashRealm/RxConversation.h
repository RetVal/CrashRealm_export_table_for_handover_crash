//
//  RxConversation.h
//  CrashRealm_Example
//
//  Created by Closure on 2019/8/21.
//  Copyright © 2019 retval. All rights reserved.
//

#import "RxMessage.h"

NS_ASSUME_NONNULL_BEGIN

@interface RxConversation : RLMObject

@property NSString *ID;
@property RLMArray<RLMString> *messageIDs;

@end

NS_ASSUME_NONNULL_END
