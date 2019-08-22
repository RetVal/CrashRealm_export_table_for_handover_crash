//
//  IdGenerator.cpp
//  RxFoundation
//
//  Created by closure on 6/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/IdGenerator.hpp>
#include <RxFoundation/Atomic.hpp>
#include <random>

using namespace Rx;

const IntegerIdGenerator::id_type &IntegerIdGenerator::InvalidId = 0;

IntegerIdGenerator::IntegerIdGenerator(id_type initializeId) :
_id(initializeId) {
}

IntegerIdGenerator::~IntegerIdGenerator() {
}

IntegerIdGenerator::id_type IntegerIdGenerator::getNextId() {
    if (_id.load() < std::numeric_limits<id_type>::max()) {
        _id++;
        return _id;
    }
    return InvalidId;
}

const StringIdGenerator::id_type &StringIdGenerator::InvalidId = "";

StringIdGenerator::StringIdGenerator(id_type initializeId) {
    std::rand();
}

StringIdGenerator::~StringIdGenerator() {}

StringIdGenerator::id_type StringIdGenerator::getNextId() {
    id_type v;
    v.resize(64);
    arc4random_buf((void *)v.data(), 64);
    return v;
}
