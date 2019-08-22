//
//  RxCacnelable.h
//  RxCoreComponents
//
//  Created by closure on 2/21/17.
//  Copyright © 2017 closure. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol RxCancellable <NSObject>
@required
- (void)cancel;
@end
