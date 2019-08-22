//
//  EnumAsByte.hpp
//  RxFoundation
//
//  Created by closure on 11/30/16.
//  Copyright © 2016 closure. All rights reserved.
//

#ifndef EnumAsByte_h
#define EnumAsByte_h

namespace Rx {
    /**
     * Template to store enumeration values as bytes in a type-safe way.
     */
    template<class TEnum>
    class EnumAsByte {
    public:
        typedef TEnum EnumType;
        
        /** Default Constructor (no initialization). */
        RX_INLINE_VISIBILITY EnumAsByte() { }
        
        /**
         * Copy constructor.
         *
         * @param InValue value to construct with.
         */
        RX_INLINE_VISIBILITY EnumAsByte( const EnumAsByte &InValue )
        : Value(InValue.Value)
        { }
        
        /**
         * Constructor, initialize to the enum value.
         *
         * @param InValue value to construct with.
         */
        RX_INLINE_VISIBILITY EnumAsByte( TEnum InValue )
        : Value(static_cast<uint8_t>(InValue))
        { }
        
        /**
         * Constructor, initialize to the Int32 value.
         *
         * @param InValue value to construct with.
         */
        explicit RX_INLINE_VISIBILITY EnumAsByte( int32_t InValue )
        : Value(static_cast<uint8_t>(InValue))
        { }
        
        /**
         * Constructor, initialize to the Int32 value.
         *
         * @param InValue value to construct with.
         */
        explicit RX_INLINE_VISIBILITY EnumAsByte( uint8_t InValue )
        : Value(InValue)
        { }
        
    public:
        
        /**
         * Assignment operator.
         *
         * @param InValue value to Set.
         * @return This instance.
         */
        RX_INLINE_VISIBILITY EnumAsByte& operator=( EnumAsByte InValue )
        {
            Value = InValue.Value;
            return *this;
        }
        /**
         * Assignment operator.
         *
         * @param InValue value to Set.
         * @return This instance.
         */
        RX_INLINE_VISIBILITY EnumAsByte& operator=( TEnum InValue )
        {
            Value = static_cast<uint8_t>(InValue);
            return *this;
        }
        
        /**
         * Compares two enumeration values for equality.
         *
         * @param InValue The value to compare with.
         * @return true if the two values are equal, false otherwise.
         */
        bool operator==( TEnum InValue ) const
        {
            return static_cast<TEnum>(Value) == InValue;
        }
        
        /**
         * Compares two enumeration values for equality.
         *
         * @param InValue The value to compare with.
         * @return true if the two values are equal, false otherwise.
         */
        bool operator==(EnumAsByte InValue) const
        {
            return Value == InValue.Value;
        }
        
        /** Implicit conversion to TEnum. */
        operator TEnum() const
        {
            return (TEnum)Value;
        }
        
    public:
        
        /**
         * Gets the enumeration value.
         *
         * @return The enumeration value.
         */
        TEnum getValue() const
        {
            return (TEnum)Value;
        }
        
    private:
        
        /** Holds the value as a byte. **/
        uint8_t Value;
    };
}

#endif /* EnumAsByte_h */
