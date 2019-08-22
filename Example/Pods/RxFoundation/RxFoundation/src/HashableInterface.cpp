//
//  HashableInterface.cpp
//  RxFoundation
//
//  Created by closure on 2/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/HashableInterface.hpp>

using namespace Rx;

#define ELF_STEP(B) T1 = (H << 4) + B; T2 = T1 & 0xF0000000; if (T2) T1 ^= (T2 >> 24); T1 &= (~T2); H = T1;

HashCode Rx::hash(const UInt8 *bytes, Index length) RX_NOEXCEPT {
    /* The ELF hash algorithm, used in the ELF object file format */
    UInt32 H = 0, T1, T2;
    Int32 rem = (Int32)length;
    while (3 < rem)
    {
        ELF_STEP(bytes[length - rem]);
        ELF_STEP(bytes[length - rem + 1]);
        ELF_STEP(bytes[length - rem + 2]);
        ELF_STEP(bytes[length - rem + 3]);
        rem -= 4;
    }
    switch (rem) {
        case 3:  ELF_STEP(bytes[length - 3]);
        case 2:  ELF_STEP(bytes[length - 2]);
        case 1:  ELF_STEP(bytes[length - 1]);
        case 0:  ;
    }
    return H;
}

#undef ELF_STEP
