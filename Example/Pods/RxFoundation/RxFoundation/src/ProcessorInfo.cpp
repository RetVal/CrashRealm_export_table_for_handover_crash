//
//  ProcessorInfo.cpp
//  RxFoundation
//
//  Created by closure on 3/9/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/ProcessorInfo.hpp>


#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
#include <sys/sysctl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <mach/mach.h>
#include <pthread.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <mach/mach.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/errno.h>
#include <mach/mach_time.h>
#include <Block.h>
#endif
#if DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace Rx;

namespace Rx {
    namespace detail {
        RX_INLINE_VISIBILITY UInteger activeProcessorCount() RX_NOEXCEPT {
            int32_t pcnt;
#if DEPLOYMENT_TARGET_WINDOWS
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            DWORD_PTR activeProcessorMask = sysInfo.dwActiveProcessorMask;
            // assumes sizeof(DWORD_PTR) is 64 bits or less
            uint64_t v = activeProcessorMask;
            v = v - ((v >> 1) & 0x5555555555555555ULL);
            v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
            v = (v + (v >> 4)) & 0xf0f0f0f0f0f0f0fULL;
            pcnt = (v * 0x0101010101010101ULL) >> ((sizeof(v) - 1) * 8);
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
            int32_t mib[] = {CTL_HW, HW_AVAILCPU};
            size_t len = sizeof(pcnt);
            int32_t result = sysctl(mib, sizeof(mib) / sizeof(int32_t), &pcnt, &len, nullptr, 0);
            if (result != 0) {
                pcnt = 0;
            }
#elif DEPLOYMENT_TARGET_ANDROID
            pcnt = android_getCpuCount();
#else
            // Assume the worst
            pcnt = 1;
#endif
            return pcnt;
        }
    }
}

namespace Rx {
    namespace OS {
        UInteger ProcessorInfo::activeProcessorCount() RX_NOEXCEPT {
            return detail::activeProcessorCount();
        }
        
        ProcessID getPid() {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_LINUX
            return getpid();
#elif DEPLOYMENT_TARGET_WINDOWS
            return GetCurrentProcessId();
#endif
            return 0;
        }
        
        ThreadID getTidWithThread(pthread_t thread) {
#if DEPLOYMENT_TARGET_ANDROID
            return gettid();
#elif DEPLOYMENT_TARGET_MACOSX
            return pthread_mach_thread_np(thread);
#elif DEPLOYMENT_TARGET_LINUX
            return syscall(SYS_gettid);
#elif DEPLOYMENT_TARGET_WINDOWS
            return GetCurrentThreadId();
#endif
            return 0;
        }
        
        ThreadID getTid() {
#if DEPLOYMENT_TARGET_ANDROID
            return gettid();
#elif DEPLOYMENT_TARGET_MACOSX
            return getTidWithThread(pthread_self());
#elif DEPLOYMENT_TARGET_LINUX
            return syscall(SYS_gettid);
#elif DEPLOYMENT_TARGET_WINDOWS
            return GetCurrentThreadId();
#endif
            return 0;
        }
    }
}
