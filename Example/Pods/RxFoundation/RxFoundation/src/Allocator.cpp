//
//  Allocator.cpp
//  RxFoundation
//
//  Created by closure on 2/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/Allocator.hpp>

using namespace Rx;

namespace Rx {
    namespace detail {
        
    }
}

Allocator Rx::Allocator::_Default;
Allocator &Rx::Allocator::Default = Rx::Allocator::_Default;

Allocator::Allocator() {}
Allocator::~Allocator() {}
