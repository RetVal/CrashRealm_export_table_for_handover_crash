//
//  Dictionary.hpp
//  RxFoundation
//
//  Created by closure on 12/16/16.
//  Copyright © 2016 closure. All rights reserved.
//

#ifndef Dictionary_hpp
#define Dictionary_hpp

#include <RxFoundation/BasicHash.hpp>
#include <map>

namespace Rx {
    template <typename KeyTy, typename ValueTy,
    typename KeyInfoTy = BasicHashInfo<KeyTy>,
    typename BucketTy = detail::BucketPair<KeyTy, ValueTy>>
    class Dictionary : public BasicHashTable
    <Dictionary<KeyTy, ValueTy, KeyInfoTy, BucketTy>,
    KeyTy, ValueTy, KeyInfoTy, BucketTy> {
        typedef BasicHashTable<Dictionary, KeyTy, ValueTy, KeyInfoTy, BucketTy> BaseTy;
        friend class BasicHashTable<Dictionary, KeyTy, ValueTy, KeyInfoTy, BucketTy>;
        typedef typename BaseTy::size_type size_type;
    public:
        explicit Dictionary(size_type initialReserve = 0) {
            init(initialReserve);
        }
        
        Dictionary(const Dictionary &other) : BaseTy() {
            init(0);
            copyFrom(other);
        }
        
        Dictionary(Dictionary &&other) : BaseTy() {
            init(0);
            swap(other);
        }
        
        ~Dictionary() {
            this->destroyAll();
            operator delete(_buckets);
            _buckets = nullptr;
        }
        
        void swap(Dictionary &other) {
            this->incrementEpoch();
            other.incrementEpoch();
            std::swap(_buckets, other._buckets);
            std::swap(_numEntries, other._numEntries);
            std::swap(_numTombstones, other._numTombstones);
            std::swap(_numBuckets, other._numBuckets);
        }
        
        Dictionary &operator=(const Dictionary &other) {
            if (&other != this) {
                copyFrom(other);
            }
            return *this;
        }
        
        Dictionary &operator=(const Dictionary &&other) {
            this->destroyAll();
            operator delete(_buckets);
            init(0);
            swap(other);
            return *this;
        }
        
        void copyFrom(const Dictionary &other) {
            this->destroyAll();
            operator delete(_buckets);
            if (_allocatedBuckets(other._numBuckets)) {
                this->BaseTy::copyFrom(other);
            } else {
                _numEntries = 0;
                _numTombstones = 0;
            }
        }
        
        void init(size_type initNumEntries) {
            auto initBuckets = BaseTy::getMinBucketToReserveForEntries(initNumEntries);
            if (_allocatedBuckets(initBuckets)) {
                this->BaseTy::initEmpty();
            } else {
                _numEntries = 0;
                _numTombstones = 0;
            }
        }
        
        void grow(UInteger atLeast) {
            auto oldNumBuckets = _numBuckets;
            auto oldBuckets = _buckets;
            _allocatedBuckets(Math::max<UInteger>(64, static_cast<UInteger>(Math::nextPowerOf2(UInteger(atLeast - 1)))));
            assert(_buckets);
            if (!oldBuckets) {
                this->BaseTy::initEmpty();
                return;
            }
            this->moveFromOldBuckets(oldBuckets, oldBuckets + oldNumBuckets);
            operator delete(oldBuckets);
        }
        
        void shrink_and_clear() {
            size_type oldNumEntries = _numEntries;
            this->destroyAll();
            size_type newNumBuckets = 0;
            if (oldNumEntries) {
                newNumBuckets = Math::min(32, 1 << Math::log2_32_Ceil(((UInt32)oldNumEntries) + 1));
            }
            if (newNumBuckets == _numBuckets) {
                this->BaseTy::initEmpty();
                return;
            }
            operator delete(_buckets);
            init(newNumBuckets);
        }
    private:
        
        size_type getNumEntries() const {
            return _numEntries;
        }
        
        void setNumEntries(size_type num) {
            _numEntries = num;
        }
        
        size_type getNumTombstones() const {
            return _numTombstones;
        }
        
        void setNumTombstones(size_type num) {
            _numTombstones = num;
        }
        
        BucketTy *getBuckets() const {
            return _buckets;
        }
        
        size_type getNumBuckets() const {
            return _numBuckets;
        }
        
        bool _allocatedBuckets(size_type num) {
            _numBuckets = num;
            if (_numBuckets == 0) {
                _buckets = nullptr;
                return false;
            }
            _buckets = static_cast<BucketTy*>(operator new(sizeof(BucketTy) * _numBuckets));
            return true;
        }
        
        BucketTy *_buckets;
        size_type _numEntries;
        size_type _numTombstones;
        size_type _numBuckets;
    };
    
    template <typename KeyTy, typename ValueTy,
    typename KeyInfoTy = BasicHashInfo<KeyTy>,
    typename BucketTy = detail::BucketPair<KeyTy, ValueTy>>
    using DictionaryPtr = SharedPtr<Dictionary<KeyTy, ValueTy, KeyInfoTy, BucketTy> >;
    
    template <typename KeyTy, typename ValueTy,
    typename KeyInfoTy = BasicHashInfo<KeyTy>,
    typename BucketTy = detail::BucketPair<KeyTy, ValueTy>>
    using DictionaryRef = SharedRef<Dictionary<KeyTy, ValueTy, KeyInfoTy, BucketTy> >;
    
    template <typename KeyTy, typename ValueTy, UInt32 InlineBuckets = 4,
    typename KeyInfoTy = BasicHashInfo<KeyTy>,
    typename BucketTy = detail::BucketPair<KeyTy, ValueTy>>
    class SmallDictionary :
    public BasicHashTable<SmallDictionary<KeyTy, ValueTy, InlineBuckets, KeyInfoTy, BucketTy>, KeyTy, ValueTy, KeyInfoTy, BucketTy> {
        typedef BasicHashTable<SmallDictionary, KeyTy, ValueTy, KeyInfoTy, BucketTy> BaseTy;
        friend class BasicHashTable<SmallDictionary, KeyTy, ValueTy, KeyInfoTy, BucketTy>;
        using size_type = typename BasicHashTable<SmallDictionary<KeyTy, ValueTy, InlineBuckets, KeyInfoTy, BucketTy>, KeyTy, ValueTy, KeyInfoTy, BucketTy>::size_type;
        size_type _small : 1;
        size_type _numEntries : sizeof(size_type) * 8 - 1;
        size_type _numTombstones;
        struct LargeRep {
            BucketTy *_buckets;
            size_type _numBuckets;
        };
        
        AlignedCharArrayUnion<BucketTy[InlineBuckets], LargeRep> _storage;
    public:
        explicit SmallDictionary(size_type numInitBuckets = 0) {
            init(numInitBuckets);
        }
        
        SmallDictionary(const SmallDictionary &other) : BaseTy() {
            init(0);
            copyFrom(other);
        }
        
        SmallDictionary(SmallDictionary &&other) : BaseTy() {
            init(0);
            swap(other);
        }
        
        template <typename InputIt>
        SmallDictionary(const InputIt &I, const InputIt &E) {
            init(Math::nextPowerOf2(std::distance(I, E)));
            this->insert(I, E);
        }
        
        ~SmallDictionary() {
            this->destroyAll();
            deallocateBuckets();
        }
        
        void swap(SmallDictionary &rhs) {
            size_type tmpNumEntries = rhs._numEntries;
            rhs._numEntries = _numEntries;
            _numEntries = tmpNumEntries;
            std::swap(_numTombstones, rhs._numTombstones);
            const KeyTy emptyKey = this->getEmptyKey();
            const KeyTy tombstoneKey = this->getTombstoneKey();
            if (_small && rhs._small) {
                for (size_type i = 0, e = InlineBuckets; i != e; ++i) {
                    BucketTy *lhsb = &getInlineBuckets()[i],
                    *rhsb = rhs.getInlineBuckets()[i];
                    bool hasLHSValue = (!KeyInfoTy::isEqual(lhsb->getKey(), emptyKey) &&
                                        !KeyInfoTy::isEqual(lhsb->getKey(), tombstoneKey));
                    bool hasRHSValue = (!KeyInfoTy::isEqual(rhsb->getKey(), emptyKey) &&
                                        !KeyInfoTy::isEqual(rhsb->getKey(), tombstoneKey));
                    if (hasLHSValue && hasRHSValue) {
                        std::swap(*lhsb, *rhsb);
                        continue;
                    }
                    std::swap(lhsb->getKey(), rhsb->getKey());
                    if (hasLHSValue) {
                        ::new (&rhsb->getValue()) ValueTy(std::move(lhsb->getValue()));
                        lhsb->getValue().~ValueTy();
                    } else if (hasRHSValue) {
                        ::new (&lhsb->getValue()) ValueTy(std::move(rhsb->getValue()));
                        rhsb->getValue().~ValueTy();
                    }
                }
                return;
            }
            if (!_small && !rhs._small) {
                std::swap(getLargeRep()->_buckets, rhs.getLargeRep()->_buckets);
                std::swap(getLargeRep()->_numBuckets, rhs.getLargeRep()->_numBuckets);
                return;
            }
            SmallDictionary &smallSide = _small ? *this : rhs;
            SmallDictionary &largeSide = _small ? rhs : *this;
            LargeRep tmpRep = std::move(*largeSide.getLargeRep());
            largeSide.getLargeRep()->~LargeRep();
            largeSide._small = true;
            for (size_type i = 0, e = InlineBuckets; i != e; ++i) {
                BucketTy *NewB = &largeSide.getInlineBuckets()[i],
                *OldB = &smallSide.getInlineBuckets()[i];
                ::new (&NewB->getKey()) KeyTy(std::move(OldB->getKey()));
                OldB->getKey().~KeyTy();
                if (!KeyInfoTy::isEqual(NewB->getKey(), emptyKey) &&
                    !KeyInfoTy::isEqual(NewB->getKey(), tombstoneKey)) {
                    ::new (&NewB->getValue()) ValueTy(std::move(OldB->getValue()));
                    OldB->getValue().~ValueTy();
                }
            }
            
            smallSide._small = false;
            new (smallSide.getLargeRep()) LargeRep(std::move(tmpRep));
        }
        
        SmallDictionary &operator=(const SmallDictionary &other) {
            if (&other != this) {
                copyFrom(other);
            }
            return *this;
        }
        
        SmallDictionary &operator=(SmallDictionary &&other) {
            this->destroyAll();
            deallocateBuckets();
            init(0);
            swap(other);
            return *this;
        }
        
        void copyFrom(const SmallDictionary &other) {
            this->destroyAll();
            deallocateBuckets();
            _small = true;
            if (other.getNumBuckets() > InlineBuckets) {
                _small = false;
                new (getLargeRep()) LargeRep(allocateBuckets(other.getNumBuckets()));
            }
            this->BaseTy::copyFrom(other);
        }
        
        void init(size_type initBuckets) {
            _small = true;
            if (initBuckets > InlineBuckets) {
                _small = false;
                new (getLargeRep()) LargeRep(allocateBuckets(initBuckets));
            }
            this->BaseTy::initEmpty();
        }
        
        void grow(UInteger atLeast) {
            if (atLeast >= InlineBuckets) {
                atLeast = Math::max<size_type>(64, Math::nextPowerOf2(atLeast - 1));
            }
            if (_small) {
                if (atLeast < InlineBuckets) {
                    return;
                }
                
                AlignedCharArrayUnion<BucketTy[InlineBuckets]> tmpStorage;
                BucketTy *tmpBegin = reinterpret_cast<BucketTy *>(tmpStorage.buffer);
                BucketTy *tmpEnd = tmpBegin;
                
                const KeyTy emptyKey = this->getEmptyKey();
                const KeyTy tombstoneKey = this->getTombstoneKey();
                for (BucketTy *P = getBuckets(), *E = getBuckets() + InlineBuckets; P != E; ++P) {
                    if (!KeyInfoTy::isEqual(P->getKey(), emptyKey) &&
                        !KeyInfoTy::isEqual(P->getKey(), tombstoneKey)) {
                        assert(size_t(tmpEnd - tmpBegin) < InlineBuckets &&
                               "Too many inline buckets!");
                        ::new (&tmpEnd->getKey()) KeyTy(std::move(P->getKey()));
                        ::new (&tmpEnd->getValue()) ValueTy(std::move(P->getValue()));
                        ++tmpEnd;
                        P->getValue().~ValueTy();
                    }
                    P->getKey().~KeyTy();
                }
                _small = false;
                new (getLargeRep()) LargeRep(allocateBuckets(atLeast));
                this->moveFromOldBuckets(tmpBegin, tmpEnd);
                return;
            }
            LargeRep oldRep = std::move(*getLargeRep());
            getLargeRep()->~LargeRep();
            if (atLeast <= InlineBuckets) {
                _small = true;
            } else {
                new (getLargeRep()) LargeRep(allocateBuckets(atLeast));
            }
            this->moveFromOldBuckets(oldRep._buckets, oldRep._buckets + oldRep._numBuckets);
            operator delete(oldRep._buckets);
        }
        
        void shrink_and_clear() {
            size_type oldSize = this->size();
            this->destroyAll();
            size_type newNumBuckets = 0;
            if (oldSize) {
                newNumBuckets = 1 << (Math::log2_32_Ceil(oldSize) + 1);
                if (newNumBuckets > InlineBuckets && newNumBuckets < 64) {
                    newNumBuckets = 64;
                }
            }
            if ((_small && newNumBuckets <= InlineBuckets) ||
                (!_small && newNumBuckets == getLargeRep()->numBuckets)) {
                this->BaseTy::initEmpty();
                return;
            }
            
            deallocateBuckets();
            init(newNumBuckets);
        }
        
    private:
        size_type getNumEntries() const {
            return _numEntries;
        }
        
        void setNumEntries(size_type Num) {
            assert(Num < INT_MAX && "Cannot support more than INT_MAX entries");
            _numEntries = Num;
        }
        
        size_type getNumTombstones() const {
            return _numTombstones;
        }
        
        void setNumTombstones(size_type Num) {
            _numTombstones = Num;
        }
        
        const BucketTy *getInlineBuckets() const {
            assert(_small);
            // Note that this cast does not violate aliasing rules as we assert that
            // the memory's dynamic type is the small, inline bucket buffer, and the
            // 'storage.buffer' static type is 'char *'.
            return reinterpret_cast<const BucketTy *>(_storage.buffer);
        }
        
        BucketTy *getInlineBuckets() {
            return const_cast<BucketTy *>(
                                          const_cast<const SmallDictionary *>(this)->getInlineBuckets());
        }
        
        const LargeRep *getLargeRep() const {
            assert(!_small);
            // Note, same rule about aliasing as with getInlineBuckets.
            return reinterpret_cast<const LargeRep *>(_storage.buffer);
        }
        LargeRep *getLargeRep() {
            return const_cast<LargeRep *>(
                                          const_cast<const SmallDictionary *>(this)->getLargeRep());
        }
        
        const BucketTy *getBuckets() const {
            return _small ? getInlineBuckets() : getLargeRep()->_buckets;
        }
        
        BucketTy *getBuckets() {
            return const_cast<BucketTy *>(
                                          const_cast<const SmallDictionary *>(this)->getBuckets());
        }
        
        size_type getNumBuckets() const {
            return _small ? InlineBuckets : getLargeRep()->_numBuckets;
        }
        
        void deallocateBuckets() {
            if (_small) {
                return;
            }
            
            operator delete(getLargeRep()->_buckets);
            getLargeRep()->~LargeRep();
        }
        
        LargeRep allocateBuckets(size_type Num) {
            assert(Num > InlineBuckets && "Must allocate more buckets than are inline");
            LargeRep Rep = {
                static_cast<BucketTy*>(operator new(sizeof(BucketTy) * Num)), Num
            };
            return Rep;
        }
    };
}

#endif /* Dictionary_hpp */
