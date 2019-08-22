//
//  CallStackArray.cpp
//  RxFoundation
//
//  Created by closure on 6/7/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/CallStackArray.hpp>
#if DEPLOYMENT_TARGET_MACOSX
#include <execinfo.h>
#elif DEPLOYMENT_TARGET_ANDROID

#include <unwind.h>
#include <dlfcn.h>

namespace {
    struct BacktraceState {
        void **current;
        void **end;
    };

    static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context *context, void *arg) {
        BacktraceState* state = static_cast<BacktraceState*>(arg);
        uintptr_t pc = _Unwind_GetIP(context);
        if (pc) {
            if (state->current == state->end) {
                return _URC_END_OF_STACK;
            } else {
                *state->current++ = reinterpret_cast<void*>(pc);
            }
        }
        return _URC_NO_REASON;
    }

    static size_t backtrace(void **buffer, int max) {
        BacktraceState state = {buffer, buffer + max};
        _Unwind_Backtrace(unwindCallback, &state);
        return state.current - buffer;
    }

    static char **backtrace_symbols(void* const*buffer, int frames) {
        Rx::String content;
        for (size_t idx = 0; idx < frames; ++idx) {
            const void *addr = buffer[idx];
            const char *symbol = "";

            Dl_info info;
            if (dladdr(addr, &info) && info.dli_sname) {
                symbol = info.dli_sname;
            }
            content.appendFormat("\t#  %ld: %s\n", 0, idx, symbol);
        }
        void *ptr = calloc(1, content.size());
        memcpy(ptr, reinterpret_cast<const void *>(content.data()), content.size());
        return reinterpret_cast<char **>(ptr);
    }
}

#endif

#include <cstdlib>
#include <cxxabi.h>

using namespace Rx;

namespace Rx {
    void getBacktrace(void **stack, int *size) {
        *size = backtrace(stack, *size);
    }
}

namespace {
    static void formatBacktrace(Array<String> &output, void **stack, int size) {
        for (int i = 0; i < size; ++i) {
            const char* mangledName = 0;
            char* cxaDemangled = 0;
            const int frameNumber = i + 1;
            if (mangledName || cxaDemangled) {
                output.addObject(String("%-3d %p %s", 0, frameNumber, stack[i], cxaDemangled ? cxaDemangled : mangledName));
            } else {
                output.addObject(String("%-3d %p", 0, frameNumber, stack[i]));
            }
            free(cxaDemangled);
        }
    }
}

CallStackArray::CallStackArray(void **stack, UInt32 frames) :
_stack(nullptr, std::free),
_infoFrames(nullptr),
_frames(frames) {
    _stack = {
        (void **)calloc(1, sizeof(void *) * frames),
        std::free
    };
    
    __builtin_memcpy(_stack.get(), stack, sizeof(void *) * frames);
    
    std::unique_ptr<char *, void(*)(void *)> infoFrames = {
        backtrace_symbols(_stack.get(), _frames),
        std::free
    };
    
    
    Array<String> demangledStrings;
    
    for (int i = 0; i < _frames; i++) {
        char *begin_name = 0, *end_name = 0, *begin_offset = 0, *end_offset = 0;
        String target;
        
        char *pp = 0;
        UInt32 spaceCount = 0;
        char *ifName = infoFrames.get()[i];
        auto ifLength = strlen(ifName);
        for (char *p = ifName; *p; ++p) {
            if (isspace(*p) && (pp && !isspace(*pp))) {
                spaceCount++;
            } else if (*p == '+' && *(p + 1) == ' ') {
                end_name = p - 1;
                begin_offset = p + 1;
            }
            pp = p;
            if (spaceCount == 3 && *(p) == ' ') {
                begin_name = p + 1;
            }
        }
        if (begin_offset) {
            end_offset = ifName + ifLength;
        }
        
        if (begin_name && end_name &&
            begin_name < end_name &&
            begin_offset && end_offset &&
            begin_name < begin_offset) {
            char end_name_char = *end_name;
            char begin_offset_char = *begin_offset;
            char end_offset_char = *end_offset;
            *end_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';
            
            int status = -1;
            std::unique_ptr<char, void(*)(void*)> result {
                __cxxabiv1::__cxa_demangle(begin_name, NULL, NULL, &status),
                std::free
            };
            
            if (status == 0) {
                target.append(ifName, begin_name - ifName);
                target.append(result.get());
                target.append(1, end_name_char);
                target.append(1, '+');
                target.append(1, ' ');
                target.append(begin_offset);
            } else {
                *end_name-- = end_name_char;
                *end_offset = end_offset_char;
                *begin_offset-- = begin_offset_char;
                target.append(ifName);
            }
        } else {
            target.append(ifName);
        }
        demangledStrings.addObject(target);
    }
    
    _infoFrames = MakeShareable<Array<String>>(demangledStrings);
    formatBacktrace(*this, _stack.get(), _frames);
}

CallStackArray::~CallStackArray() {
}

const String CallStackArray::getDescription() const {
    String description;
    if (_infoFrames) {
        for (UInteger idx = 0; idx < _frames; ++idx) {
            description.append((*_infoFrames)[idx]);
            description.append("\n");
        }
    } else {
        for (auto &symbol : *this) {
            description.append(symbol);
            description.append("\n");
        }
    }
    return description;
}
