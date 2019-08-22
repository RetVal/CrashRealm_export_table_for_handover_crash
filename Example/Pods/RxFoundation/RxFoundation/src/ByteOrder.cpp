//
//  ByteOrder.cpp
//  RxFoundation
//
//  Created by closure on 3/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/ByteOrder.hpp>

namespace Rx {
    Int32 SwapInt32BigToHost(Int32 v) {
        return byte_swap<big_endian, host_endian>(v);
    }
}
