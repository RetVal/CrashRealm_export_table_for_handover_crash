//
//  CrashReporter.cpp
//  RxFoundation
//
//  Created by closure on 5/9/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/CrashReporter.hpp>
#include <unistd.h>

using namespace Rx;

void CrashReporter::HALT(String content) {
#if defined(__ppc__)
#define __HALT do { asm __volatile__("trap"); kill(getpid(), 9); } while (0)
#elif defined(__i386__) || defined(__x86_64__) || !(DEPLOYMENT_TARGET_ANDROID)
    #if defined(__GNUC__)
        #define __HALT do { asm __volatile__("int3"); kill(getpid(), 9); } while (0)
    #elif defined(_MSC_VER)
        #define __HALT do {DebugBreak(); abort(); } while (0)
    #else
        #error Compiler not supported
    #endif
#endif
#if defined(__arm__)
    #define __HALT do {asm __volatile__("bkpt 0xCF"); kill(getpid(), 9); } while (0)
#elif defined(__arm64__)
    #define __HALT do {asm __volatile__("svc #0x80"); kill(getpid(), 9); } while (0)
#elif !defined(__HALT)
    #define __HALT __assert2(__FILE__, __LINE__, __FUNCTION__, "")
#endif
    __HALT;
#if __has_builtin(__builtin_unreachable)
    __builtin_unreachable();
#endif
#undef __HALT
}

RxPrivate void Rx::__THE_SYSTEM_HAS_NO_PORTS_AVAILABLE__(kern_return_t ret) {
    CrashReporter::HALT(__FUNCTION__);
}

RxPrivate void Rx::__NO_SPACE__(kern_return_t ret) {
    CrashReporter::HALT(__FUNCTION__);
}

RxPrivate void Rx::__RESOURCE_SHORTAGE__(kern_return_t ret) {
    CrashReporter::HALT(__FUNCTION__);
}

RxPrivate void Rx::__THE_SYSTEM_HAS_NO_PORT_SETS_AVAILABLE__(kern_return_t ret) {
    if (ret == KERN_NO_SPACE) {
        Rx::__NO_SPACE__(ret);
    }
    else if (ret == KERN_RESOURCE_SHORTAGE) {
        Rx::__RESOURCE_SHORTAGE__(ret);
    }
    CrashReporter::HALT(__FUNCTION__);
}
