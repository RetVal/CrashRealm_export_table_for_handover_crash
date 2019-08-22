//
//  Data.cpp
//  RxFoundation
//
//  Created by closure on 11/30/16.
//  Copyright © 2016 closure. All rights reserved.
//

#include <RxFoundation/Data.hpp>
#include <RxFoundation/HashableInterface.hpp>
#include <RxFoundation/CrashReporter.hpp>

#include <string.h>
#include <fstream>
#include <algorithm>

using namespace Rx;

namespace Rx {
    namespace detail {
#if DEPLOYMENT_TARGET_ANDROID
        RX_INLINE_VISIBILITY int flsl(long value) {
            return __builtin_ffs(value);
        }
#endif
#if __LP64__
        static const constexpr UInt64 DataMaxSize	= ((1ULL << 42) - 1);
#else
        static const constexpr UInt32 DataMaxSize	= ((1ULL << 31) - 1);
#endif
#if __LP64__
        static const constexpr UInt64 ChunkSize = (1ULL << 29);
        static const constexpr UInt64 LowThreshold = (1ULL << 20);
        static const constexpr UInt64 HighThreshold = (1ULL << 32);
#else
        static const constexpr UInt32 ChunkSize = (1ULL << 26);
        static const constexpr UInt32 LowThreshold = (1ULL << 20);
        static const constexpr UInt32 HighThreshold = (1ULL << 29);
#endif
        enum Flags {
            DontDeallocate = 0x10,
        };
        
        static Index roundUpCapacity(Index capacity) {
            if (capacity < 16) {
                return 16;
            } else if (capacity < LowThreshold) {
                /* Up to 4x */
                long idx = flsl(capacity);
                return (1L << (long)(idx + ((idx % 2 == 0) ? 0 : 1)));
            } else if (capacity < HighThreshold) {
                /* Up to 2x */
                return (1L << (long)flsl(capacity));
            } else {
                /* Round up to next multiple of CHUNK_SIZE */
                unsigned long newCapacity = ChunkSize * (1+(capacity >> ((long)flsl(ChunkSize)-1)));
                return std::min<decltype(DataMaxSize)>(newCapacity, DataMaxSize);
            }
        }
        
        static void *dataAllocate(Index size, bool clear) {
            void *bytes = nullptr;
            if (clear) {
                bytes = calloc(1, size);
            } else {
                bytes = malloc(size);
            }
            return bytes;
        }
        
        static void validateRange(const Data &data, Range range, const char *func) {
            RxCheck(0 <= range.location && range.location <= data.getLength());
            RxCheck(0 <= range.length);
            RxCheck(range.location + range.length <= data.getLength());
        }
        
        static void handleOutOfMemory(const Data &data, Index numBytes) {
            String msg;
            if(0 < numBytes && numBytes <= DataMaxSize) {
                msg = String("Attempt to allocate %ld bytes for Rx::Data failed", 0, numBytes);
            } else {
                msg = String("Attempt to allocate %ld bytes for Rx::Data failed. Maximum size: %lld", 0, numBytes, DataMaxSize);
            }
            CrashReporter::HALT(msg);
        }
    }
}

Data::Data() RX_NOEXCEPT : Data(nullptr, detail::roundUpCapacity(1), false, true) {
}

Data::Data(uint8_t *bytes, Index length, bool copy, bool freeWhenDone) RX_NOEXCEPT :
_bytes(nullptr),
_length(0),
_capacity(0) {
    if (!copy && bytes) {
        _bytes = bytes;
        __setLength(length);
        __setCapacity(length);
        if (!freeWhenDone) {
            _flags |= detail::Flags::DontDeallocate;
        }
    } else {
        _bytes = (UInt8 *)detail::dataAllocate(length * sizeof(UInt8), true);
        if (bytes) {
            __builtin_memcpy(_bytes, bytes, length * sizeof(UInt8));
            __setLength(length);
        }
        __setCapacity(length);
    }
}

Data::Data(const Data &value) RX_NOEXCEPT {
    appendData(value);
}

Data::Data(Data &&value) RX_NOEXCEPT :
_bytes(std::move(value._bytes)),
_length(std::move(value._length)),
_capacity(std::move(value._capacity)) {
}

Data::~Data() RX_NOEXCEPT {
    if (!(_flags.to_ulong() & detail::Flags::DontDeallocate)) {
        if (_bytes) {
            free(_bytes);
            _bytes = nullptr;
        }
        __setLength(0);
        __setCapacity(0);
    }
}

const uint8_t *Data::getBytePtr() const RX_NOEXCEPT {
    return _bytes;
}

uint8_t *Data::getMutableBytePtr() const RX_NOEXCEPT {
    return _bytes;
}

const Index Data::getLength() const RX_NOEXCEPT {
    return _length;
}

Index Data::getLength() RX_NOEXCEPT {
    return _length;
}

const Index Data::getCapacity() const RX_NOEXCEPT {
    return _capacity;
}

Index Data::getCapacity() RX_NOEXCEPT {
    return _capacity;
}

void Data::setLength(Index length) RX_NOEXCEPT {
    __setLength(length);
}

void Data::appendData(const uint8_t *bytes, Index length) RX_NOEXCEPT {
    replaceBytes(Range(getLength(), 0), bytes, length);
}

void Data::appendData(const DataRef data) RX_NOEXCEPT {
    appendData(data.get());
}

void Data::appendData(const Data &data) RX_NOEXCEPT {
    appendData(data.getBytePtr(), data.getLength());
}

void Data::__setCapacity(Index capacity) RX_NOEXCEPT {
    _capacity = capacity;
}

void Data::__setLength(Index length) RX_NOEXCEPT {
    _length = length;
}

void Data::replaceBytes(Range range, const uint8_t *newBytes, Index newLength) RX_NOEXCEPT {
    detail::validateRange(*this, range, __PRETTY_FUNCTION__);
    RxCheck(0 <= newLength);
    
    Index len = getLength();
    if (len < 0 || range.length < 0 || newLength < 0) {
        CrashReporter::HALT();
    }
    Index newCount = len - range.length + newLength;
    if (newCount < 0) {
        CrashReporter::HALT();
    }
    
    uint8_t *bytePtr = getMutableBytePtr();
    uint8_t *srcBuf = (uint8_t *)newBytes;
    if (getCapacity() < newCount) {
        if (bytePtr && newBytes && newBytes < bytePtr + getCapacity() && bytePtr < newBytes + newLength) {
            srcBuf = (uint8_t *)malloc(newLength * sizeof(uint8_t));
            memmove(srcBuf, newBytes, newLength * sizeof(uint8_t));
        }
        __dataGrow(*this, newLength - range.length, false);
        bytePtr = getMutableBytePtr();
    }
    if (newLength != range.length && range.location + range.length < len) {
        memmove(bytePtr + range.location + newLength, bytePtr + range.location + range.length, (len - range.location - range.length) * sizeof(uint8_t));
    }
    if (0 < newLength && srcBuf) {
        memmove(bytePtr + range.location, srcBuf, newLength * sizeof(uint8_t));
    }
    if (srcBuf != newBytes) {
        free(srcBuf);
    }
    __setLength(newCount);
}

void Data::deleteBytes(Range range) RX_NOEXCEPT {
    replaceBytes(range, nullptr, 0);
}

bool Data::writeToFile(String path, bool automatically) RX_NOEXCEPT {
//    std::ofstream ofs;
//    std::ios_base::fmtflags flag = std::ios_base::out | std::ios_base::binary;
//    if (automatically) {
//        flag |= std::ios_base::trunc;
//    } else {
//        flag |= std::ios_base::app;
//    }
//    
//    ofs.open(path.c_str(), flag);
//    bool success = ofs.is_open();
//    if (success) {
//        ofs.write((char *)Ptr, Length);
//    }
//    return success;
    return false;
}

bool Data::operator==(const Data &rhs) const RX_NOEXCEPT {
    auto length = getLength();
    if (length != rhs.getLength()) {
        return false;
    }
    auto *ptr0 = getBytePtr();
    auto *ptr1 = rhs.getBytePtr();
    return 0 == __builtin_memcmp(ptr0, ptr1, length);
}

String Data::copyDescription() const RX_NOEXCEPT {
    String result;
    Index idx;
    Index len;
    const uint8_t *bytes = nullptr;
    len = getLength();
    bytes = getBytePtr();
    result.appendFormat("<Rx::Data %p>{length = %lu, capacity = %lu, bytes = 0x", 0, this, (unsigned long)len, (unsigned long)getCapacity());
    if (24 < len) {
        for (idx = 0; idx < 16; idx += 4) {
            result.appendFormat("%02x%02x%02x%02x", 0, bytes[idx], bytes[idx + 1], bytes[idx + 2], bytes[idx + 3]);
        }
        result += " ... ";
        for (idx = len - 8; idx < len; idx += 4) {
            result.appendFormat("%02x%02x%02x%02x", 0, bytes[idx], bytes[idx + 1], bytes[idx + 2], bytes[idx + 3]);
        }
    } else {
        for (idx = 0; idx < len; idx++) {
            result.appendFormat("%02x", 0, bytes[idx]);
        }
    }
    result += "}";
    return result;
}

HashCode Data::hash() const RX_NOEXCEPT {
    return Rx::hash(getBytePtr(), std::min<decltype(_length)>(80, getLength()));
}

/* Allocates new block of data with at least numNewValues more bytes than the current length. If clear is true, the new bytes up to at least the new length with be zeroed. */
void Data::__dataGrow(Data &data, Index numNewValues, bool clear) RX_NOEXCEPT {
    Index oldLength = data.getLength();
    Index newLength = oldLength + numNewValues;
    if (newLength > detail::DataMaxSize || newLength < 0) {
        detail::handleOutOfMemory(data, newLength * sizeof(uint8_t));
    }
    Index capacity = detail::roundUpCapacity(newLength);
    Index numBytes = capacity;
    void *bytes = NULL;
    void *oldBytes = data.getMutableBytePtr();
    bool allocateCleared = clear;
    if (allocateCleared && (oldLength == 0 || (newLength / oldLength) > 4)) {
        // If the length that needs to be zeroed is significantly greater than the length of the data, then calloc/memmove is probably more efficient than realloc/memset.
        bytes = detail::dataAllocate(numBytes * sizeof(uint8_t), true);
        if (NULL != bytes) {
            memmove(bytes, oldBytes, oldLength);
            free(oldBytes);
        }
    }
    if (bytes == NULL) {
        // If the calloc/memmove approach either failed or was never attempted, then realloc.
        allocateCleared = false;
        bytes = realloc(oldBytes, numBytes * sizeof(uint8_t));
    }
    if (NULL == bytes) {
        detail::handleOutOfMemory(data, numBytes * sizeof(uint8_t));
    }
    data.__setCapacity(capacity);
    if (clear && !allocateCleared && oldLength < newLength) {
        memset((uint8_t *)bytes + oldLength, 0, newLength - oldLength);
    }
    *((void **)&data._bytes) = bytes;
}
