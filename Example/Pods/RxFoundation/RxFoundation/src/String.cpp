//
//  String.cpp
//  RxFoundation
//
//  Created by closure on 11/30/16.
//  Copyright © 2016 closure. All rights reserved.
//

#include <RxFoundation/String.hpp>
#include <RxFoundation/Array.hpp>
#include <cstdarg>

using namespace Rx;

#include <cstdarg>
#include <memory>
#include <sstream>

static String string_format(const String &fmt, Integer reserved, va_list ap) RX_NOEXCEPT {
    const char *format = fmt.c_str();
    char *ret = nullptr;
    size_t size = vasprintf(&ret, format, ap);
    std::allocator<String> allocator;
    String result(ret, size, allocator);
    if (ret) {
        RxCheck(result.data() != ret);
        RxCheck(strncmp(result.data(), ret, size) == 0);
        free(ret);
    }
    return result;
}

String::String(const String &fmt, Integer reserved, ...) RX_NOEXCEPT {
    va_list ap;
    va_start(ap, reserved);
    String result = string_format(fmt, reserved, ap);
    va_end(ap);
    *this = result;
}

String::String(const String &fmt, Integer reserved, va_list ap) RX_NOEXCEPT {
    *this = string_format(fmt, reserved, ap);
}

Rx::Int32 String::compare(const value_type *value) const RX_NOEXCEPT {
    return base::compare(value);
}

Rx::Int32 String::compare(const String &value) const RX_NOEXCEPT {
    return base::compare(value);
}

bool String::operator<(const String &value) const RX_NOEXCEPT {
    return compare(value) < 0;
}

bool String::operator==(const String &value) const RX_NOEXCEPT {
    return 0 == compare(value);
}

bool String::operator==(const value_type *value) const RX_NOEXCEPT {
    return 0 == compare(value);
}

Array<Range> String::findResults(const String &separator, Range range) const RX_NOEXCEPT {
    Array<Range> ranges;
    if (range.location + range.length > length()) {
        return ranges;
    }
    auto separatorLength = separator.length();
    if (range.length < separatorLength) {
        return ranges;
    }
    auto base = c_str();
    auto current = base + range.location;
    auto find = separator.c_str();
    do {
        auto result = strstr(current, find);
        if (result != nullptr) {
            Range range(result - base, separatorLength);
            ranges.addObject(range);
            current += range.location + range.length; // skip separator
        } else {
            break;
        }
    } while (1);
    return ranges;
}

Array<String> String::componetsSeparatedByString(const String &separator) const RX_NOEXCEPT {
    const Index length = this->length();
    Array<Range> ranges = findResults(separator, Range(0, length));
    Index count = ranges.size();
    if (0 == count) {
        return Array<String>({*this});
    }
    Index startIndex = 0;
    Index numChars = 0;
    Array<String> array;
    Range lastRange = ranges[count - 1];
    array.reserve(count + (lastRange.location + lastRange.length < length ? 1 : 0));
    for (const Range &currentRange : ranges) {
        numChars = currentRange.location - startIndex;
        const String subString = substr(startIndex, numChars);
        array.addObject(subString);
        startIndex = currentRange.location + currentRange.length;
    }
    const String subString = substr(startIndex, length - numChars);
    array.addObject(subString);
    return array;
}

bool String::hasPrefix(const String &value) const RX_NOEXCEPT {
    const auto len = length();
    const auto valueLen = value.length();
    if (len < valueLen) {
        return false;
    }
    return std::equal(begin(),
                      begin() + valueLen,
                      value.begin());
}

bool String::hasSuffix(const String &value) const RX_NOEXCEPT {
    const auto len = length();
    const auto valueLen = value.length();
    if (len < valueLen) {
        return false;
    }
    return std::equal(begin() + len - valueLen,
                      end(),
                      value.begin());
}

String &String::appendFormat(const String &fmt, Integer reserved, ...) RX_NOEXCEPT {
    va_list ap;
    va_start(ap, reserved);
    appendFormat(fmt, reserved, ap);
    va_end(ap);
    return *this;
}

String &String::appendFormat(const String &fmt, Integer reserved, va_list ap) RX_NOEXCEPT {
    *this += String(fmt, reserved, ap);
    return *this;
}

namespace Rx {
    namespace detail {
        const UInt64 HashEverythingLimit = 96;
        
#define HashNextFourUniChars(accessStart, accessEnd, pointer) \
{result = result * 67503105 + (accessStart 0 accessEnd) * 16974593  + (accessStart 1 accessEnd) * 66049  + (accessStart 2 accessEnd) * 257 + (accessStart 3 accessEnd); pointer += 4;}
        
#define HashNextUniChar(accessStart, accessEnd, pointer) \
{result = result * 257 + (accessStart 0 accessEnd); pointer++;}
        UniChar __CharToUniCharTable[256] = {
            0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
            16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
            32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
            48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
            64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
            80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
            96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
            112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
            128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
            144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
            160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
            176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
            192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
            208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
            224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
            240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
        };
    }
}

HashCode String::hash() const RX_NOEXCEPT {
    const uint8_t *cContents = reinterpret_cast<const uint8_t *>(data());
    HashCode len = length();
    HashCode result = len;
    if (len <= Rx::detail::HashEverythingLimit) {
        const uint8_t *end4 = cContents + (len & ~3);
        const uint8_t *end = cContents + len;
        while (cContents < end4) HashNextFourUniChars(detail::__CharToUniCharTable[cContents[, ]], cContents); 	// First count in fours
        while (cContents < end) HashNextUniChar(detail::__CharToUniCharTable[cContents[, ]], cContents);		// Then for the last <4 chars, count in ones...
    } else {
        const uint8_t *contents, *end;
        contents = cContents;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(detail::__CharToUniCharTable[contents[, ]], contents);
        contents = cContents + (len >> 1) - 16;
        end = contents + 32;
        while (contents < end) HashNextFourUniChars(detail::__CharToUniCharTable[contents[, ]], contents);
        end = cContents + len;
        contents = end - 32;
        while (contents < end) HashNextFourUniChars(detail::__CharToUniCharTable[contents[, ]], contents);
    }
    return result + (result << (len & 31));
}

Integer String::getIntegerValue() const RX_NOEXCEPT {
#if __LP64__ || (TARGET_OS_EMBEDDED && !TARGET_OS_IPHONE) || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
    return getLongValue();
#else
    return getIntValue();
#endif
}

int String::getIntValue() const RX_NOEXCEPT {
    int v = 0;
    if (auto ptr = c_str()) {
        v = atoi(ptr);
    }
    return v;
}

unsigned int String::getUnsignedIntValue() const RX_NOEXCEPT {
    unsigned int v = 0;
    if (auto ptr = c_str()) {
        v = atoi(ptr);
    }
    return v;
}

float String::getFloatValue() const RX_NOEXCEPT {
    float v = 0;
    if (auto ptr = c_str()) {
        v = strtof(ptr, nullptr);
    }
    return v;
}

double String::getDoubleValue() const RX_NOEXCEPT {
    double v = 0;
    if (auto ptr = c_str()) {
        v = strtod(ptr, nullptr);
    }
    return v;
}

long String::getLongValue() const RX_NOEXCEPT {
    long v = 0;
    if (auto ptr = c_str()) {
        v = strtol(ptr, nullptr, 10);
    }
    return v;
}

unsigned long String::getUnsignedLongValue() const RX_NOEXCEPT {
    unsigned long v = 0;
    if (auto ptr = c_str()) {
        v = strtoul(ptr, nullptr, 10);
    }
    return v;
}

long long String::getLongLongValue() const RX_NOEXCEPT {
    long long v = 0;
    if (auto ptr = c_str()) {
        v = strtoll(ptr, nullptr, 10);
    }
    return v;
}

unsigned long long String::getUnsignedLongLongValue() const RX_NOEXCEPT {
    unsigned long long v = 0;
    if (auto ptr = c_str()) {
        v = strtoull(ptr, nullptr, 10);
    }
    return v;
}

bool String::getBooleanValue() const RX_NOEXCEPT {
    if ((*this) == "YES" || (*this) == "true" || (*this) == "True" || (*this) == "yes") {
        return true;
    } else if ((*this) == "NO" || (*this) == "false" || (*this) == "False" || (*this) == "no") {
        return false;
    }
    return getIntValue() == 1;
}
