//
//  Exception.cpp
//  RxFoundation
//
//  Created by closure on 3/6/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/Exception.hpp>
#include <RxFoundation/CallStackArray.hpp>

using namespace Rx;

Exception::Exception(const String &name, const String reason) RX_NOEXCEPT :
_name(name),
_reason(reason) {
    static const int framesToShow = 31;
    static const int framesToSkip = 2;
    void *samples[framesToShow + framesToSkip];
    int frames = framesToShow + framesToSkip;
    getBacktrace(samples, &frames);
    _callStackSymbols = MakeShareable<CallStackArray>(samples, frames);
}

Exception::~Exception() RX_NOEXCEPT {
}

const char *Exception::what() const RX_NOEXCEPT {
    return copyDescription().c_str();
}

const String Exception::copyDescription() const {
    String description;
    String callStackSymbolsDescription = StaticCastSharedPtr<CallStackArray>(_callStackSymbols)->getDescription();
    description = String("<%s> - %s\n%s", 0, getName().c_str(), getReason().c_str(), callStackSymbolsDescription.c_str());
    return description;
}

const Array<String> Exception::getCallStackSymbols() const {
    return *StaticCastSharedPtr<CallStackArray>(_callStackSymbols);
}
