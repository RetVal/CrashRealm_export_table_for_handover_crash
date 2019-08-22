//
//  objcClassReader.cpp
//  RxFoundation
//
//  Created by Closure on 2018/11/15.
//  Copyright © 2018 Closure. All rights reserved.
//

#include <RxFoundation/objcClassReader.hpp>

#import <Foundation/Foundation.h>
#import <objc/runtime.h>

namespace Rx {
namespace ObjC {
    NSArray<Class> *getClasses(bool (^predicate)(Class cls)) {
        unsigned int numClasses = 0;
        std::unique_ptr<__unsafe_unretained Class[], decltype(&free)> classes (objc_copyClassList(&numClasses), free);
        
        NSMutableArray *result = [NSMutableArray array];
        auto classesPtr = classes.get();
        for (auto idx = 0; idx < numClasses; ++idx) {
            Class cls = classesPtr[idx];
            if (predicate(cls)) {
                [result addObject:cls];
            }
        }
        return result;
    }
    
    NSArray<Class> *getSubclasses(Class parentClass) {
        return getClasses(^bool(__unsafe_unretained Class cls) {
            Class superClass = cls;
            do {
                superClass = class_getSuperclass(superClass);
            } while(superClass && superClass != parentClass);
            
            if (superClass == nil) {
                return false;
            }
            return true;
        });
    }
    
    NSArray<Class> *getProtocols(Protocol *protocol) {
        return getClasses(^bool(__unsafe_unretained Class cls) {
            Class superClass = cls;
            do {
                if (class_conformsToProtocol(superClass, protocol)) {
                    return true;
                }
                superClass = class_getSuperclass(superClass);
            } while(superClass != nullptr);
            
            if (superClass == nil) {
                return false;
            }
            return true;
        });
    }
}
}
