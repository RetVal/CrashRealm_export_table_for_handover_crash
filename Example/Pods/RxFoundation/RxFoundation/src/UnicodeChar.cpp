//
//  UnicodeChar.cpp
//  RxFoundation
//
//  Created by closure on 2/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/UnicodeChar.hpp>
#include <RxFoundation/Atomic.hpp>
#include <RxFoundation/ByteOrder.hpp>
#include <RxFoundation/Allocator.hpp>

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#define USE_MACHO_SEGMENT 1
#endif

#if (DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED) && USE_MACHO_SEGMENT
#include <mach-o/getsect.h>
#include <mach-o/dyld.h>
#include <mach-o/ldsyms.h>
#endif

using namespace Rx;

namespace Rx {
    namespace detail {
        enum {
            LastExternalSet = UnicodeChar::NewlineCharacterSet,
            FirstInternalSet = UnicodeChar::CompatibilityDecomposableCharacterSet,
            LastInternalSet = UnicodeChar::GraphemeExtendCharacterSet,
            FirstBitmapSet = UnicodeChar::DecimalDigitCharacterSet
        };
        
        RX_INLINE_VISIBILITY uint32_t MapExternalSetToInternalIndex(uint32_t cset) RX_NOEXCEPT {
            return ((FirstInternalSet <= cset) ? ((cset - FirstInternalSet) + LastExternalSet) : cset) - FirstBitmapSet;
        }
        
        RX_INLINE_VISIBILITY uint32_t MapCompatibilitySetID(uint32_t cset) RX_NOEXCEPT {
            return ((cset == UnicodeChar::ControlCharacterSet) ? UnicodeChar::ControlAndFormatterCharacterSet : (((cset > LastExternalSet) && (cset < FirstInternalSet)) ? ((cset - LastExternalSet) + FirstInternalSet) : cset));
        }
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#define __RSCharacterSetDir "/System/Library/CoreServices"
#elif DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define __RSCharacterSetDir "/usr/local/share/CoreFoundation"
#elif DEPLOYMENT_TARGET_WINDOWS
#define __RSCharacterSetDir "\\Windows\\CoreFoundation"
#endif
#if (DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED) && USE_MACHO_SEGMENT
        
        static const void *GetSectDataPtr(const char *segname, const char *sectname, uint64_t *sizep) {
            uint32_t idx, cnt = _dyld_image_count();
            for (idx = 0; idx < cnt; idx++) {
                void *mh = (void *)_dyld_get_image_header(idx);
                if (mh != &_mh_dylib_header) continue;
#if __LP64__
                const struct section_64 *sect = getsectbynamefromheader_64((struct mach_header_64 *)mh, segname, sectname);
#else
                const struct section *sect = getsectbynamefromheader((struct mach_header *)mh, segname, sectname);
#endif
                if (!sect) break;
                if (sizep) *sizep = (uint64_t)sect->size;
                return (char *)sect->addr + _dyld_get_image_vmaddr_slide(idx);
            }
            if (sizep) *sizep = 0ULL;
            return nullptr;
        }
#endif
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
        static bool LoadFile(const char *bitmapName, const void **bytes, int64_t *fileSize) {
#elif DEPLOYMENT_TARGET_WINDOWS
        static bool LoadFile(const wchar_t *bitmapName, const void **bytes, int64_t *fileSize) {
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
            
#if USE_MACHO_SEGMENT
            *bytes = GetSectDataPtr("__UNICODE", bitmapName, nullptr);
            
            if (nullptr != fileSize) {
                *fileSize = 0;
            }
            
            return *bytes ? true : false;
#else
#if DEPLOYMENT_TARGET_ANDROID
            return false;
#elif DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
            char cpath[MAXPATHLEN];
            CharacterSetPath(cpath);
            strlcat(cpath, bitmapName, MAXPATHLEN);
            return LoadBytesFromFile(cpath, bytes, fileSize);
#elif DEPLOYMENT_TARGET_WINDOWS
            wchar_t wpath[MAXPATHLEN];
            CharacterSetPath(wpath);
            wcsncat(wpath, bitmapName, MAXPATHLEN);
            return LoadBytesFromFile(wpath, bytes, fileSize);
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
#endif
        }

#if USE_MACHO_SEGMENT
        RX_INLINE_VISIBILITY bool SimpleFileSizeVerification(const void *bytes, int64_t fileSize) { return true; }
#elif 1
        // <rdar://problem/8961744> SimpleFileSizeVerification is broken
        static bool SimpleFileSizeVerification(const void *bytes, int64_t fileSize) { return true; }
#else
        static bool SimpleFileSizeVerification(const void *bytes, int64_t fileSize) {
            bool result = true;
            
            if (fileSize > 0) {
                if ((sizeof(uint32_t) * 2) > fileSize) {
                    result = false;
                } else {
                    uint32_t headerSize = RSSwapInt32BigToHost(*((uint32_t *)((char *)bytes + 4)));
                    
                    if ((headerSize < (sizeof(uint32_t) * 4)) || (headerSize > fileSize)) {
                        result = false;
                    } else {
                        const uint32_t *lastElement = (uint32_t *)(((uint8_t *)bytes) + headerSize) - 2;
                        
                        if ((headerSize + RSSwapInt32BigToHost(lastElement[0]) + RSSwapInt32BigToHost(lastElement[1])) > headerSize) result = false;
                    }
                }
            }
            
            if (!result) RSLog(RSLogLevelCritical, RSSTR("File size verification for Unicode database file failed."));
            
            return result;
        }
#endif // USE_MACHO_SEGMENT
        // Bitmap functions
        RX_INLINE_VISIBILITY bool isControl(UTF32Char theChar, uint16_t charset, const void *data) RX_NOEXCEPT {
            // ISO Control
            return (((theChar <= 0x001F) || (theChar >= 0x007F && theChar <= 0x009F)) ? true : false);
        }
        
        RX_INLINE_VISIBILITY bool isWhitespace(UTF32Char theChar, uint16_t charset, const void *data) RX_NOEXCEPT {
            // Space
            return (((theChar == 0x0020) || (theChar == 0x0009) || (theChar == 0x00A0) || (theChar == 0x1680) || (theChar >= 0x2000 && theChar <= 0x200B) || (theChar == 0x202F) || (theChar == 0x205F) || (theChar == 0x3000)) ? true : false);
        }
        
        RX_INLINE_VISIBILITY bool isNewline(UTF32Char theChar, uint16_t charset, const void *data) RX_NOEXCEPT {
            // White space
            return (((theChar >= 0x000A && theChar <= 0x000D) || (theChar == 0x0085) || (theChar == 0x2028) || (theChar == 0x2029)) ? true : false);
        }
        
        RX_INLINE_VISIBILITY bool isWhitespaceAndNewline(UTF32Char theChar, uint16_t charset, const void *data) RX_NOEXCEPT {
            // White space
            return ((isWhitespace(theChar, charset, data) || isNewline(theChar, charset, data)) ? true : false);
        }
        
        typedef struct {
            uint32_t _numPlanes;
            const uint8_t **_planes;
        } BitmapData;
        
        static char UnicodeVersionString[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        
        static uint32_t NumberOfBitmaps = 0;
        static BitmapData *BitmapDataArray = nullptr;
        
        static SpinLock BitmapLock;
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#if !defined(UNICHAR_BITMAP_FILE)
#if USE_MACHO_SEGMENT
#define UNICHAR_BITMAP_FILE "__csbitmaps"
#else
#define UNICHAR_BITMAP_FILE "/RSCharacterSetBitmaps.bitmap"
#endif
#endif
#elif DEPLOYMENT_TARGET_WINDOWS
#if !defined(UNICHAR_BITMAP_FILE)
#define UNICHAR_BITMAP_FILE L"RSCharacterSetBitmaps.bitmap"
#endif
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
        
        static bool LoadBitmapData(void) {
            BitmapData *array;
            uint32_t headerSize;
            uint32_t bitmapSize;
            int numPlanes;
            uint8_t currentPlane;
            const void *bytes;
            const void *bitmapBase;
            const void *bitmap;
            int idx, bitmapIndex;
            int64_t fileSize;
            
            LockGuard<decltype(BitmapLock)> lock(BitmapLock);
            
            if (BitmapDataArray || !LoadFile(UNICHAR_BITMAP_FILE, &bytes, &fileSize) || !SimpleFileSizeVerification(bytes, fileSize)) {
                return false;
            }
            
            for (idx = 0;idx < 4 && ((const uint8_t *)bytes)[idx];idx++) {
                UnicodeVersionString[idx * 2] = ((const uint8_t *)bytes)[idx];
                UnicodeVersionString[idx * 2 + 1] = '.';
            }
            UnicodeVersionString[(idx < 4 ? idx * 2 - 1 : 7)] = '\0';
            
            headerSize = SwapInt32BigToHost(*((uint32_t *)((char *)bytes + 4)));
            
            bitmapBase = (uint8_t *)bytes + headerSize;
            bytes = (uint8_t *)bytes + (sizeof(uint32_t) * 2);
            headerSize -= (sizeof(uint32_t) * 2);
            
            NumberOfBitmaps = headerSize / (sizeof(uint32_t) * 2);
            
            array = Allocator::Default.allocate<BitmapData>(NumberOfBitmaps);
            
            for (idx = 0;idx < (int)NumberOfBitmaps;idx++) {
                bitmap = (uint8_t *)bitmapBase + SwapInt32BigToHost(*((uint32_t *)bytes)); bytes = (uint8_t *)bytes + sizeof(uint32_t);
                bitmapSize = SwapInt32BigToHost(*((uint32_t *)bytes)); bytes = (uint8_t *)bytes + sizeof(uint32_t);
                
                numPlanes = bitmapSize / (8 * 1024);
                numPlanes = *(const uint8_t *)((char *)bitmap + (((numPlanes - 1) * ((8 * 1024) + 1)) - 1)) + 1;
                
                
                
//                array[idx]._planes = (const uint8_t **)RSAllocatorAllocate(RSAllocatorSystemDefault, sizeof(const void *) * numPlanes);
                array[idx]._planes = (const uint8_t **)(Allocator::Default.allocate<uint8_t *>(numPlanes));
                array[idx]._numPlanes = numPlanes;
                
                currentPlane = 0;
                for (bitmapIndex = 0;bitmapIndex < numPlanes;bitmapIndex++) {
                    if (bitmapIndex == currentPlane) {
                        array[idx]._planes[bitmapIndex] = (const uint8_t *)bitmap;
                        bitmap = (uint8_t *)bitmap + (8 * 1024);
#if defined (__cplusplus)
                        currentPlane = *(((const uint8_t*&)bitmap)++);
#else
                        currentPlane = *((const uint8_t *)bitmap++);
#endif
                        
                    } else {
                        array[idx]._planes[bitmapIndex] = nullptr;
                    }
                }
            }
            
            BitmapDataArray = array;
            
            return true;
        }
        
//        RSPrivate const char *GetUnicodeVersionString(void) {
//            if (nil == BitmapDataArray) LoadBitmapData();
//            return UnicodeVersionString;
//        }
    }
}

bool UnicodeChar::isSurrogateHighCharacter() const RX_NOEXCEPT {
    return ((_character >= 0xD800UL) && (_character <= 0xDBFFUL) ? true : false);
}

bool UnicodeChar::isSurrogateLowCharacter() const RX_NOEXCEPT {
    return ((_character >= 0xDC00UL) && (_character <= 0xDFFFUL) ? true : false);
}

bool UnicodeChar::isMemberOf(const uint8_t *bitmap) const RX_NOEXCEPT {
    return (bitmap && (bitmap[(_character) >> BitShiftForByte] & (((uint32_t)1) << (_character & BitShiftForMask))) ? true : false);
}

bool UnicodeChar::isMemberOf(uint32_t charset) const RX_NOEXCEPT {
    auto character = _character;
    charset = detail::MapCompatibilitySetID(charset);
    switch (charset) {
        case WhitespaceCharacterSet:
            return detail::isWhitespace(character, charset, nullptr);
            
        case WhitespaceAndNewlineCharacterSet:
            return detail::isWhitespaceAndNewline(character, charset, nullptr);
            
        case NewlineCharacterSet:
            return detail::isNewline(character, charset, nullptr);
            
        default: {
            uint32_t tableIndex = detail::MapExternalSetToInternalIndex(charset);
            
            if (nullptr == detail::BitmapDataArray) detail::LoadBitmapData();
            
            if (tableIndex < detail::NumberOfBitmaps) {
                detail::BitmapData *data = detail::BitmapDataArray + tableIndex;
                uint8_t planeNo = (character >> 16) & 0xFF;
                
                // The bitmap data for IllegalCharacterSet is actually LEGAL set less Plane 14 ~ 16
                if (charset == IllegalCharacterSet) {
                    if (planeNo == 0x0E) { // Plane 14
                        character &= 0xFF;
                        return (((character == 0x01) || ((character > 0x1F) && (character < 0x80))) ? false : true);
                    } else if (planeNo == 0x0F || planeNo == 0x10) { // Plane 15 & 16
                        return ((character & 0xFF) > 0xFFFD ? true : false);
                    } else {
                        return (planeNo < data->_numPlanes && data->_planes[planeNo] ? !UnicodeChar(character).isMemberOf(data->_planes[planeNo]) : true);
                    }
                } else if (charset == ControlAndFormatterCharacterSet) {
                    if (planeNo == 0x0E) { // Plane 14
                        character &= 0xFF;
                        return (((character == 0x01) || ((character > 0x1F) && (character < 0x80))) ? true : false);
                    } else {
                        return (planeNo < data->_numPlanes && data->_planes[planeNo] ? UnicodeChar(character).isMemberOf(data->_planes[planeNo]) : false);
                    }
                } else {
                    return (planeNo < data->_numPlanes && data->_planes[planeNo] ? UnicodeChar(character).isMemberOf(data->_planes[planeNo]) : false);
                }
            }
            return false;
        }
    }
}

void UnicodeChar::addCharacterToBitmap(uint8_t *bitmap) const RX_NOEXCEPT {
    bitmap[(_character) >> BitShiftForByte] |= (((uint32_t)1) << (_character & BitShiftForMask));
}
