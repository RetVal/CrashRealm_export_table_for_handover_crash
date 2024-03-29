//
//  EpochTracker.h
//  RxFoundation
//
//  Created by Closure on 6/23/17.
//  Copyright © 2017 closure. All rights reserved.
//

#ifndef EpochTracker_h
#define EpochTracker_h

#include <RxFoundation/RxBase.hpp>

namespace Rx {
#if LLVM_ENABLE_ABI_BREAKING_CHECKS
    
    /// \brief A base class for data structure classes wishing to make iterators
    /// ("handles") pointing into themselves fail-fast.  When building without
    /// asserts, this class is empty and does nothing.
    ///
    /// DebugEpochBase does not by itself track handles pointing into itself.  The
    /// expectation is that routines touching the handles will poll on
    /// isHandleInSync at appropriate points to assert that the handle they're using
    /// is still valid.
    ///
    class DebugEpochBase {
        uint64_t Epoch;
        
    public:
        DebugEpochBase() : Epoch(0) {}
        
        /// \brief Calling incrementEpoch invalidates all handles pointing into the
        /// calling instance.
        void incrementEpoch() { ++Epoch; }
        
        /// \brief The destructor calls incrementEpoch to make use-after-free bugs
        /// more likely to crash deterministically.
        ~DebugEpochBase() { incrementEpoch(); }
        
        /// \brief A base class for iterator classes ("handles") that wish to poll for
        /// iterator invalidating modifications in the underlying data structure.
        /// When LLVM is built without asserts, this class is empty and does nothing.
        ///
        /// HandleBase does not track the parent data structure by itself.  It expects
        /// the routines modifying the data structure to call incrementEpoch when they
        /// make an iterator-invalidating modification.
        ///
        class HandleBase {
            const uint64_t *EpochAddress;
            uint64_t EpochAtCreation;
            
        public:
            HandleBase() : EpochAddress(nullptr), EpochAtCreation(UINT64_MAX) {}
            
            explicit HandleBase(const DebugEpochBase *Parent)
            : EpochAddress(&Parent->Epoch), EpochAtCreation(Parent->Epoch) {}
            
            /// \brief Returns true if the DebugEpochBase this Handle is linked to has
            /// not called incrementEpoch on itself since the creation of this
            /// HandleBase instance.
            bool isHandleInSync() const { return *EpochAddress == EpochAtCreation; }
            
            /// \brief Returns a pointer to the epoch word stored in the data structure
            /// this handle points into.  Can be used to check if two iterators point
            /// into the same data structure.
            const void *getEpochAddress() const { return EpochAddress; }
        };
    };
    
#else
    
    class DebugEpochBase {
    public:
        void incrementEpoch() {}
        
        class HandleBase {
        public:
            HandleBase() = default;
            explicit HandleBase(const DebugEpochBase *) {}
            bool isHandleInSync() const { return true; }
            const void *getEpochAddress() const { return nullptr; }
        };
    };
    
#endif // LLVM_ENABLE_ABI_BREAKING_CHECKS
}
#endif /* EpochTracker_h */
