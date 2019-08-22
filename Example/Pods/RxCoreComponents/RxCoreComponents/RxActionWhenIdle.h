//
//  RxActionWhenIdle.h
//  RxCoreComponents
//
//  Created by Closure on 2019/5/24.
//  Copyright © 2019 Closure. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 invoke the action once the current runloop became idle
 
 Example:
 
 [RxActionWhenIdle actionWhenIdleWithTarget:view
                                   selector:@selector(refreshUI)
                                     object:nil];
 */
@interface RxActionWhenIdle : NSObject

+ (nullable instancetype)actionWhenIdleWithTarget:(id)target selector:(SEL)selector object:(nullable id)object;

- (nullable instancetype)initWithTarget:(id)target selector:(SEL)selector object:(nullable id)object;

- (BOOL)isValid;

@end

NS_ASSUME_NONNULL_END
