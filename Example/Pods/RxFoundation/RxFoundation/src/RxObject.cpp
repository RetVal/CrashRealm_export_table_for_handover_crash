//
//  Object.cpp
//  RxFoundation
//
//  Created by closure on 9/30/16.
//  Copyright © 2016 closure. All rights reserved.
//

#include <RxFoundation/RxObject.hpp>
#include <RxFoundation/String.hpp>
#include <RxFoundation/ProcessorInfo.hpp>
#include <RxFoundation/ObjectMeta.hpp>

using namespace Rx;

Object::Object() RX_NOEXCEPT :
_dead(false) {
//    Log("alloc %s %p\n", 0, Rx::getClassName(this).c_str(), this);
}

Object::~Object() RX_NOEXCEPT {
//    const std::type_info &info = typeid(this->_dead);
//    __logTypeInfo(&info);
//    const __cxxabiv1::__class_type_info* dynamic_type = getTypeInfo(this, nullptr);
//    __logTypeInfo(dynamic_type);
    _dead = true;
//    Log("dealloc %s %p\n", 0, Rx::getClassName(this).c_str(), this);
}

String Object::copyDescription() const RX_NOEXCEPT {
    try {
        String description = String("%s<%p>", 0, getClassName((void *)this).c_str(), this);
        return description;
    } catch (...) {
        return String("Object<%p>", 0, this);
    }
}

bool Object::operator<(const Object *rhs) const RX_NOEXCEPT {
    return Pointer(this) < Pointer(rhs);
}

bool Object::operator<(const Object &rhs) const RX_NOEXCEPT {
    return Pointer(this) < Pointer(&rhs);
}

bool Object::operator==(const Object &rhs) const RX_NOEXCEPT {
    return this == &rhs;
}

HashCode Object::hash() const RX_NOEXCEPT {
    return (HashCode)this;
}

void Rx::Log(const String format, Integer reserved, ...) {
    if (!format.length()) {
        return;
    }
    va_list ap;
    va_start(ap, reserved);
    String str = String(format, reserved, ap);
    va_end(ap);
    
    if (!str.length()) {
        return;
    }
    
    str = String("[%05d:%04x] ", 0, OS::getPid(), OS::getTid()) + str.c_str();
    
    auto pos = str.find_last_of("\n");
    if (String::npos == pos || pos != str.length() - String("\n").length()) {
        str += "\n";
    }
    static SpinLock spinLock;
    {
        LockGuard<decltype(spinLock)> lock(spinLock);
        fprintf(stdout, "%s", str.c_str());
    }
}

