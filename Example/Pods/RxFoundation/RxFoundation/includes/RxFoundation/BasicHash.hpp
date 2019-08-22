//
//  BasicHash.hpp
//  RxFoundation
//
//  Created by closure on 12/16/16.
//  Copyright © 2016 closure. All rights reserved.
//

#ifndef BasicHash_hpp
#define BasicHash_hpp

#include <RxFoundation/RxBase.hpp>
#include <RxFoundation/Math.hpp>
#include <RxFoundation/BasicHashTypeInfo.hpp>
#include <RxFoundation/DebugEpochBase.hpp>
#include <RxFoundation/type_traits.hpp>

namespace Rx {
    
    template<class T = void *>
    bool shouldReverseIterate() {
        return std::is_pointer<T>::value;
    }
    
    namespace detail {
        template <typename KeyTy, typename ValueTy>
        class BucketPair : public std::pair<KeyTy, ValueTy> {
        public:
            KeyTy &getKey() { return std::pair<KeyTy, ValueTy>::first; }
            const KeyTy &getKey() const {
                return std::pair<KeyTy, ValueTy>::first;
            }
            
            ValueTy &getValue() { return std::pair<KeyTy, ValueTy>::second; }
            const ValueTy &getValue() const {
                return std::pair<KeyTy, ValueTy>::second;
            }
            
            KeyTy &getFirst() { return std::pair<KeyTy, ValueTy>::first; }
            KeyTy &getSecond() { return std::pair<KeyTy, ValueTy>::second; }
        };
    }
    
    
    template <typename KeyTy, typename ValueTy, typename KeyInfoTy,
    typename BucketTy, bool isConst>
    class BasicHashTableIterator : DebugEpochBase::HandleBase {
        typedef BasicHashTableIterator<KeyTy, ValueTy, KeyInfoTy, BucketTy, false> Iterator;
        typedef BasicHashTableIterator<KeyTy, ValueTy, KeyInfoTy, BucketTy, true> ConstIterator;
        friend class BasicHashTableIterator<KeyTy, ValueTy, KeyInfoTy, BucketTy, true>;
        friend class BasicHashTableIterator<KeyTy, ValueTy, KeyInfoTy, BucketTy, false>;
        
    public:
        typedef ptrdiff_t difference_type;
        typedef typename std::conditional<isConst, const BucketTy, BucketTy>::type value_type;
        typedef value_type *pointer;
        typedef value_type &reference;
        typedef std::forward_iterator_tag iterator_category;
        
    public:
        BasicHashTableIterator() : _begin(nullptr), _end(nullptr) {}
        BasicHashTableIterator(pointer position,
                               pointer end,
                               const DebugEpochBase &Epoch,
                               bool noAdvance = false) :
        DebugEpochBase::HandleBase(&Epoch),
        _begin(position), _end(end) {
            assert(isHandleInSync() && "invalid construction!");
            if (noAdvance) {
                return;
            }
            if (shouldReverseIterate<KeyTy>()) {
                retreatPastEmptyBuckets();
                return;
            }
            advancePastEmptyBuckets();
        }
        
        template <bool isConstSrc,
        typename = typename std::enable_if<!isConstSrc && isConst>::type>
        BasicHashTableIterator(const BasicHashTableIterator<KeyTy, ValueTy, KeyInfoTy, BucketTy, isConstSrc> &I)
        : DebugEpochBase::HandleBase(I), _begin(I._begin), _end(I._end) {}
        
        reference operator*() const {
            assert(isHandleInSync() && "invalid iterator access!");
            if (shouldReverseIterate<KeyTy>()) {
                return _begin[-1];
            }
            return *_begin;
        }
        
        pointer operator->() const {
            assert(isHandleInSync() && "invalid iterator access!");
            if (shouldReverseIterate<KeyTy>()) {
                return &(_begin[-1]);
            }
            return _begin;
        }
        
        bool operator==(const ConstIterator &RHS) const {
            assert((!_begin || isHandleInSync()) && "handle not in sync!");
            assert((!RHS._begin || RHS.isHandleInSync()) && "handle not in sync!");
            assert(getEpochAddress() == RHS.getEpochAddress() &&
                   "comparing incomparable iterators!");
            return _begin == RHS._begin;
        }
        
        bool operator!=(const ConstIterator &RHS) const {
            assert((!_begin || isHandleInSync()) && "handle not in sync!");
            assert((!RHS._begin || RHS.isHandleInSync()) && "handle not in sync!");
            assert(getEpochAddress() == RHS.getEpochAddress() &&
                   "comparing incomparable iterators!");
            return _begin != RHS._begin;
        }
        
        inline BasicHashTableIterator &operator++() const {
            assert(isHandleInSync() && "invalid iterator access!");
            if (shouldReverseIterate<KeyTy>()) {
                --_begin;
                retreatPastEmptyBuckets();
                return *this;
            }
            ++_begin;
            advancePastEmptyBuckets();
            return *this;
        }
        
        BasicHashTableIterator operator++(int v) {
            assert(isHandleInSync() && "invalid iterator access!");
            BasicHashTableIterator tmp = *this;
            ++*this;
            return tmp;
        }
    private:
        void advancePastEmptyBuckets() {
            const KeyTy Empty = KeyInfoTy::getEmptyKey();
            const KeyTy Tombstone = KeyInfoTy::getTombstoneKey();
            
            while (_begin != _end && (KeyInfoTy::isEqual(_begin->getKey(), Empty) ||
                                      KeyInfoTy::isEqual(_begin->getKey(), Tombstone)))
                ++_begin;
        }
        
        void retreatPastEmptyBuckets() {
            assert(_begin >= _end);
            const KeyTy Empty = KeyInfoTy::getEmptyKey();
            const KeyTy Tombstone = KeyInfoTy::getTombstoneKey();
            
            while (_begin != _end && (KeyInfoTy::isEqual(_begin[-1].getKey(), Empty) ||
                                      KeyInfoTy::isEqual(_begin[-1].getKey(), Tombstone)))
                --_begin;
        }
        
        pointer _begin = nullptr, _end = nullptr;
    };
    
    template <typename DerivedType, typename KeyTy, typename ValueTy,
    typename BasicHashInfoType, typename BucketTy>
    class BasicHashTable : public DebugEpochBase {
    public:
        typedef size_t                                      size_type;
        typedef KeyTy                                       key_type;
        typedef ValueTy                                     mapped_type;
        typedef BucketTy                                    value_type;
        typedef BasicHashTableIterator<key_type, mapped_type, BasicHashInfoType, BucketTy, false> iterator;
        typedef BasicHashTableIterator<key_type, mapped_type, BasicHashInfoType, BucketTy, true> const_iterator;
        typedef std::reverse_iterator<iterator>             reverse_iterator;
        typedef std::reverse_iterator<const_iterator>       const_reverse_iterator;
        //    typedef typename allocator_type::reference       reference;
        //    typedef typename allocator_type::const_reference const_reference;
        //    typedef typename allocator_type::pointer         pointer;
        //    typedef typename allocator_type::const_pointer   const_pointer;
        //    typedef typename allocator_type::size_type       size_type;
        //    typedef typename allocator_type::difference_type difference_type;
        
        inline iterator begin() {
            if (empty()) {
                return end();
            }
            if (shouldReverseIterate<KeyTy>()) {
                return makeIterator(getBucketsEnd() - 1, getBuckets(), *this);
            }
            return makeIterator(getBuckets(), getBucketsEnd(), *this);
        }
        
        inline iterator end() {
            return makeIterator(getBucketsEnd(), getBucketsEnd(), *this, true);
        }
        
        inline const_iterator begin() const {
            if (empty()) {
                return end();
            }
            if (shouldReverseIterate<KeyTy>()) {
                return makeConstIterator(getBucketsEnd() - 1, getBuckets(), *this);
            }
            return makeConstIterator(getBuckets(), getBucketsEnd(), *this);
        }
        
        inline const_iterator end() const {
            return makeConstIterator(getBucketsEnd(), getBucketsEnd(), *this, true);
        }
        
        inline bool empty() const { return 0 == size(); }
        
        size_type size() const { return getNumEntries(); }
        
        void reserve(size_type numEntries) {
            auto numBuckets = getMinBucketToReserveForEntries(numEntries);
            incrementEpoch();
            if (numBuckets > getNumBuckets()) {
                grow(numBuckets);
            }
        }
        
        void clear() {
            incrementEpoch();
            const auto onceNumEntries = getNumEntries();
            const auto onceNumTombstones = getNumTombstones();
            const auto onceNumBuckets = getNumBuckets();
            if (onceNumEntries == 0 && onceNumTombstones == 0) {
                return;
            }
            
            if (onceNumEntries * 4 < onceNumBuckets && onceNumBuckets > 64) {
                shrink_and_clear();
                return;
            }
            
            const KeyTy emptyKey = getEmptyKey(), tombstoneKey = getTombstoneKey();
            
            if (std::is_trivially_copyable<KeyTy>::value &&
                std::is_trivially_copyable<ValueTy>::value) {
                // Use a simpler loop when these are trivial types.
                for (BucketTy *current = getBuckets(), *End = getBucketsEnd(); current != End; ++current) {
                    current->getKey() = emptyKey;
                }
            } else {
                size_type numEntries = onceNumEntries;
                for (BucketTy *current = getBuckets(), *End = getBucketsEnd(); current != End; ++current) {
                    if (!BasicHashInfoType::isEqual(current->getKey(), emptyKey)) {
                        if (!BasicHashInfoType::isEqual(current->getKey(), tombstoneKey)) {
                            current->getValue().~ValueTy();
                            --numEntries;
                        }
                        current->getKey() = emptyKey;
                    }
                }
                assert(numEntries == 0 && "Node count imbalance!");
            }
            setNumEntries(0);
            setNumTombstones(0);
        }
        
        size_type count(const KeyTy &val) const {
            BucketTy *bucket;
            return lookupBucketFor(val, bucket) ? 1 : 0;
        }
        
        iterator find(const KeyTy &val) {
            BucketTy *theBucket;
            if (lookupBucketFor(val, theBucket)) {
                return makeIterator(theBucket, getBucketsEnd(), *this, true);
            }
            return end();
        }
        
        const_iterator find(const KeyTy &val) const {
            const BucketTy *theBucket;
            if (lookupBucketFor(val, theBucket)) {
                return makeConstIterator(theBucket, getBucketsEnd(), *this, true);
            }
            return end();
        }
        
        template <typename LookupKeyTy>
        iterator find_as(const LookupKeyTy &val) {
            BucketTy *theBucket;
            if (lookupBucketFor(val, theBucket)) {
                return makeIterator(theBucket, getBucketsEnd(), *this, true);
            }
            return end();
        }
        
        template <typename LookupKeyTy>
        const_iterator find_as(const LookupKeyTy &val) const {
            const BucketTy *theBucket;
            if (lookupBucketFor(val, theBucket)) {
                return makeConstIterator(theBucket, getBucketsEnd(), *this, true);
            }
            return end();
        }
        
        std::pair<iterator, bool> insert(const std::pair<KeyTy, ValueTy> &kv) {
            BucketTy *theBucket;
            if (lookupBucketFor(kv.first, theBucket)) {
                return std::make_pair(makeIterator(theBucket, getBucketsEnd(), *this, true), false);
            }
            theBucket = insertIntoBucket(kv.first, kv.second, theBucket);
            return std::make_pair(makeIterator(theBucket, getBucketsEnd(), *this, true), true);
        }
        
        std::pair<iterator, bool> insert(std::pair<KeyTy, ValueTy> &&kv) {
            BucketTy *theBucket;
            if (lookupBucketFor(kv.first, theBucket)) {
                return std::make_pair(makeIterator(theBucket, getBucketsEnd(), *this, true), false);
            }
            theBucket = insertIntoBucket(std::move(kv.first),
                                         std::move(kv.second),
                                         theBucket);
            return std::make_pair(makeIterator(theBucket, getBucketsEnd(), *this, true), true);
        }
        
        template <typename LookupKeyTy>
        std::pair<iterator, bool> insert_as(std::pair<KeyTy, ValueTy> &&kv,
                                            const LookupKeyTy &val) {
            BucketTy *theBucket;
            if (lookupBucketFor(val, theBucket))
                return std::make_pair(makeIterator(theBucket, getBucketsEnd(), *this, true), false); // Already in map.
            
            // Otherwise, insert the new element.
            theBucket = InsertIntoBucket(std::move(kv.first),
                                         std::move(kv.second),
                                         val,
                                         theBucket);
            return std::make_pair(makeIterator(theBucket, getBucketsEnd(), *this, true),
                                  true);
        }
        
        /// insert - Range insertion of pairs.
        template<typename InputIt>
        void insert(InputIt I, InputIt E) {
            for (; I != E; ++I) {
                insert(*I);
            }
        }
        
        bool erase(const KeyTy &val) {
            BucketTy *theBucket;
            if (!lookupBucketFor(val, theBucket))
                return false; // not in map.
            
            theBucket->getValue().~ValueT();
            theBucket->getKey() = getTombstoneKey();
            decrementNumEntries();
            incrementNumTombstones();
            return true;
        }
        
        void erase(iterator I) {
            BucketTy *theBucket = &*I;
            theBucket->getValue().~ValueT();
            theBucket->getKey() = getTombstoneKey();
            decrementNumEntries();
            incrementNumTombstones();
        }
        
        value_type& findAndConstruct(const KeyTy &Key) {
            BucketTy *TheBucket;
            if (lookupBucketFor(Key, TheBucket)) {
                return *TheBucket;
            }
            return *insertIntoBucket(Key, ValueTy(), TheBucket);
        }
        
        ValueTy &operator[](const KeyTy &Key) {
            return findAndConstruct(Key).second;
        }
        
        value_type& findAndConstruct(KeyTy &&Key) {
            BucketTy *theBucket;
            if (lookupBucketFor(Key, theBucket))
                return *theBucket;
            
            return *insertIntoBucket(std::move(Key), ValueTy(), theBucket);
        }
        
        ValueTy &operator[](KeyTy &&key) {
            return findAndConstruct(std::move(key)).second;
        }
        
        /// isPointerIntoBucketsArray - Return true if the specified pointer points
        /// somewhere into the DenseMap's array of buckets (i.e. either to a key or
        /// value in the DenseMap).
        bool isPointerIntoBucketsArray(const void *Ptr) const {
            return Ptr >= getBuckets() && Ptr < getBucketsEnd();
        }
        
        /// getPointerIntoBucketsArray() - Return an opaque pointer into the buckets
        /// array.  In conjunction with the previous method, this can be used to
        /// determine whether an insertion caused the DenseMap to reallocate.
        const void *getPointerIntoBucketsArray() const { return getBuckets(); }
        
        size_t getMemorySize() const {
            return getNumBuckets() * sizeof(BucketTy);
        }
        
    protected:
        BasicHashTable() = default;
        
        void destroyAll() {
            if (getNumBuckets() == 0) {
                return;
            }
            const KeyTy emptyKey = getEmptyKey(), tombstoneKey = getTombstoneKey();
            for (BucketTy *B = getBuckets(), *E = getBucketsEnd(); B != E; ++B) {
                if (!BasicHashInfoType::isEqual(B->getKey(), emptyKey) &&
                    !BasicHashInfoType::isEqual(B->getKey(), tombstoneKey)) {
                    B->getValue().~ValueTy();
                }
                B->getKey().~KeyTy();
            }
        }
        
        void initEmpty() {
            setNumEntries(0);
            setNumTombstones(0);
            assert((getNumBuckets() & (getNumBuckets() - 1)) == 0 &&
                   "# initial buckets must be a power of two!");
            const KeyTy emptyKey = getEmptyKey();
            for (BucketTy *B = getBuckets(), *E = getBucketsEnd(); B != E; ++B) {
                ::new (&B->getKey()) KeyTy(emptyKey);
            }
        }
        
        size_type getMinBucketToReserveForEntries(size_type numEntries) {
            if (numEntries == 0) {
                return 0;
            }
            
            UInteger x = numEntries * 4 / 3 + 1;
            
            return Math::nextPowerOf2(x);
        }
        
        void moveFromOldBuckets(BucketTy *oldBucketsBegin, BucketTy *oldBucketsEnd) {
            initEmpty();
            const KeyTy emptyKey = getEmptyKey();
            const KeyTy tombstoneKey = getTombstoneKey();
            for (BucketTy *B = oldBucketsBegin, *E = oldBucketsEnd; B != E; B++) {
                if (!BasicHashInfoType::isEqual(B->getKey(), emptyKey) &&
                    !BasicHashInfoType::isEqual(B->getKey(), tombstoneKey)) {
                    BucketTy *destBucket = nullptr;
                    bool foundValue = lookupBucketFor(B->getKey(), destBucket);
                    (void)foundValue;
                    assert(!foundValue && "Key already in new map?");
                    destBucket->getKey() = std::move(B->getKey());
                    ::new (&destBucket->getValue()) ValueTy(std::move(B->getValue()));
                    incrementNumEntries();
                    B->getValue().~ValueTy();
                }
                B->getKey().~KeyTy();
            }
        }
        
        template <typename OtherBaseTy>
        void copyFrom(const BasicHashTable<OtherBaseTy, KeyTy, ValueTy,
                      BasicHashInfoType, BucketTy> &other) {
            assert(&other != this);
            assert(getNumBuckets() == other.getNumBuckets());
            
            setNumEntries(other.getNumEntries());
            setNumTombstones(other.getNumTombstones());
            
            if (isPodLike<KeyTy>::value && isPodLike<ValueTy>::value) {
                memcpy(getBuckets(), other.getBuckets(),
                       getNumBuckets() * sizeof(BucketTy));
            } else {
                for (size_t i = 0; i < getNumBuckets(); ++i) {
                    ::new (&getBuckets()[i].getKey())
                    KeyTy(other.getBuckets()[i].getKey());
                    if (!BasicHashInfoType::isEqual(getBuckets()[i].getKey(), getEmptyKey()) &&
                        !BasicHashInfoType::isEqual(getBuckets()[i].getKey(), getTombstoneKey()))
                        ::new (&getBuckets()[i].getValue())
                        ValueTy(other.getBuckets()[i].getValue());
                }
            }
        }
        
        static UInt64 getHashValue(const KeyTy &Val) {
            return BasicHashInfoType::getHashValue(Val);
        }
        
        template<typename LookupKeyTy>
        static UInt64 getHashValue(const LookupKeyTy &val) {
            return BasicHashInfoType::getHashValue(val);
        }
        
        static const KeyTy getEmptyKey() {
            return BasicHashInfoType::getEmptyKey();
        }
        
        static const KeyTy getTombstoneKey() {
            return BasicHashInfoType::getTombstoneKey();
        }
        
    private:
        iterator makeIterator(BucketTy *P, BucketTy *E,
                              DebugEpochBase &Epoch,
                              bool NoAdvance = false) {
            if (shouldReverseIterate<KeyTy>()) {
                BucketTy *B = P == getBucketsEnd() ? getBuckets() : P + 1;
                return iterator(B, E, Epoch, NoAdvance);
            }
            return iterator(P, E, Epoch, NoAdvance);
        }
        
        const_iterator makeConstIterator(const BucketTy *P, const BucketTy *E,
                                         const DebugEpochBase &Epoch,
                                         const bool NoAdvance = false) const {
            if (shouldReverseIterate<KeyTy>()) {
                const BucketTy *B = P == getBucketsEnd() ? getBuckets() : P + 1;
                return const_iterator(B, E, Epoch, NoAdvance);
            }
            return const_iterator(P, E, Epoch, NoAdvance);
        }
        
        size_type getNumEntries() const {
            return static_cast<const DerivedType *>(this)->getNumEntries();
        }
        
        void setNumEntries(size_type numEntries) {
            static_cast<DerivedType *>(this)->setNumEntries(numEntries);
        }
        
        void incrementNumEntries() {
            setNumEntries(getNumEntries() + 1);
        }
        
        void decrementNumEntries() {
            setNumEntries(getNumEntries() - 1);
        }
        
        size_type getNumTombstones() const {
            return static_cast<const DerivedType *>(this)->getNumTombstones();
        }
        
        void setNumTombstones(size_type numTombstones) {
            static_cast<DerivedType *>(this)->setNumTombstones(numTombstones);
        }
        
        void incrementNumTombstones() {
            setNumTombstones(getNumTombstones() + 1);
        }
        
        void decrementNumTombstones() {
            setNumTombstones(getNumTombstones() - 1);
        }
        
        const BucketTy *getBuckets() const {
            return static_cast<const DerivedType *>(this)->getBuckets();
        }
        BucketTy *getBuckets() {
            return static_cast<DerivedType *>(this)->getBuckets();
        }
        size_type getNumBuckets() const {
            return static_cast<const DerivedType *>(this)->getNumBuckets();
        }
        BucketTy *getBucketsEnd() {
            return getBuckets() + getNumBuckets();
        }
        const BucketTy *getBucketsEnd() const {
            return getBuckets() + getNumBuckets();
        }
        
        void grow(UInteger atLeast) {
            static_cast<DerivedType *>(this)->grow(atLeast);
        }
        
        void shrink_and_clear() {
            static_cast<DerivedType *>(this)->shrink_and_clear();
        }
        
        BucketTy *insertIntoBucket(const KeyTy &key, const ValueTy &value,
                                   BucketTy *bucket) {
            bucket = insertIntoBucketImpl(key, key, bucket);
            bucket->getKey() = key;
            ::new (&bucket->getValue()) ValueTy(value);
            return bucket;
        }
        
        BucketTy *insertIntoBucket(const KeyTy &key, ValueTy &&value,
                                   BucketTy *bucket) {
            bucket = insertIntoBucketImpl(key, key, bucket);
            bucket->getKey() = key;
            ::new (&bucket->getValue()) ValueTy(std::move(value));
            return bucket;
        }
        
        BucketTy *insertIntoBucket(KeyTy &&key, ValueTy &&value,
                                   BucketTy *bucket) {
            bucket = insertIntoBucketImpl(key, key, bucket);
            bucket->getKey() = std::move(key);
            ::new (&bucket->getValue()) ValueTy(std::move(value));
            return bucket;
        }
        
        template <typename LookupKeyTy>
        BucketTy *insertIntoBucket(KeyTy &&key, ValueTy &&value, LookupKeyTy &lookup,
                                   BucketTy *bucket) {
            bucket = insertIntoBucketImpl(key, lookup, bucket);
            bucket->getKey() = std::move(key);
            ::new (&bucket->getValue()) ValueTy(std::move(value));
            return bucket;
        }
        
        template <typename LookupKeyTy>
        BucketTy *insertIntoBucketImpl(const KeyTy &key, LookupKeyTy &lookup,
                                       BucketTy *bucket) {
            incrementEpoch();
            size_type newNumEntries = getNumEntries() + 1;
            size_type numBuckets = getNumBuckets();
            if (newNumEntries * 4 >= numBuckets * 3) {
                this->grow(numBuckets * 2);
                lookupBucketFor(lookup, bucket);
                numBuckets = getNumBuckets();
            } else if (numBuckets - (newNumEntries + getNumTombstones()) <=
                       numBuckets / 8) {
                this->grow(numBuckets);
                lookupBucketFor(lookup, bucket);
            }
            assert(bucket);
            incrementNumEntries();
            const KeyTy emptyKey = getEmptyKey();
            if (!BasicHashInfoType::isEqual(bucket->getKey(), emptyKey)) {
                decrementNumTombstones();
            }
            return bucket;
        }
        
        template <typename LookupKeyTy>
        bool lookupBucketFor(const LookupKeyTy &value,
                             const BucketTy *&foundBucket) const {
            const BucketTy *bucketsPtr = getBuckets();
            const size_type numBuckets = getNumBuckets();
            if (numBuckets == 0) {
                foundBucket = nullptr;
                return false;
            }
            
            const BucketTy *foundTombstone = nullptr;
            const KeyTy emptyKey = getEmptyKey();
            const KeyTy tombstonKey = getTombstoneKey();
            assert(!BasicHashInfoType::isEqual(value, emptyKey) &&
                   !BasicHashInfoType::isEqual(value, tombstonKey) &&
                   "Empty/Tombston value shouldn't be inserted into map");
            size_type bucketIndex = getHashValue(value) & (numBuckets - 1);
            size_type probeAmt = 1;
            while (1) {
                const BucketTy *currentBucket = bucketsPtr + bucketIndex;
                if (BasicHashInfoType::isEqual(value, currentBucket->getKey())) {
                    foundBucket = currentBucket;
                    return true;
                }
                if (BasicHashInfoType::isEqual(emptyKey, currentBucket->getKey())) {
                    foundBucket = foundTombstone ? foundTombstone : currentBucket;
                    return false;
                }
                if (BasicHashInfoType::isEqual(tombstonKey, currentBucket->getKey()) &&
                    !foundTombstone) {
                    foundTombstone = currentBucket;
                }
                bucketIndex += probeAmt++;
                bucketIndex &= (numBuckets - 1);
            }
        }
        
        template <typename LookupKeyTy>
        bool lookupBucketFor(const LookupKeyTy &value,
                             BucketTy *&foundBucket) {
            const BucketTy *constFoundBucket = nullptr;
            bool isFound = const_cast<const BasicHashTable *>(this)->lookupBucketFor(value, constFoundBucket);
            foundBucket = const_cast<BucketTy *>(constFoundBucket);
            return isFound;
        }
    };
    
    template <typename DerivedTy, typename KeyTy, typename ValueTy, typename KeyInfoTy,
    typename BucketTy>
    bool operator!=(const BasicHashTable<DerivedTy, KeyTy, ValueTy, KeyInfoTy, BucketTy> &LHS,
                    const BasicHashTable<DerivedTy, KeyTy, ValueTy, KeyInfoTy, BucketTy> &RHS) {
        return !(LHS == RHS);
    }
}

#endif /* BasicHash_hpp */
