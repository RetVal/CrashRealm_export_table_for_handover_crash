//
//  DebugEpochBase.hpp
//  RxFoundation
//
//  Created by Closure on 2019/3/12.
//  Copyright © 2019 Closure. All rights reserved.
//

#ifndef DebugEpochBase_hpp
#define DebugEpochBase_hpp

#include <RxFoundation/RxBase.hpp>

namespace Rx {
    class DebugEpochBase {
        uint64_t Epoch;
    public:
        DebugEpochBase() : Epoch(0) {}
        /// Calling incrementEpoch invalidates all handles pointing into the
        /// calling instance.
        void incrementEpoch() { ++Epoch; }
        
        /// The destructor calls incrementEpoch to make use-after-free bugs
        /// more likely to crash deterministically.
        ~DebugEpochBase() { incrementEpoch(); }
        
        /// A base class for iterator classes ("handles") that wish to poll for
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
            
            /// Returns true if the DebugEpochBase this Handle is linked to has
            /// not called incrementEpoch on itself since the creation of this
            /// HandleBase instance.
            bool isHandleInSync() const { return *EpochAddress == EpochAtCreation; }
            
            /// Returns a pointer to the epoch word stored in the data structure
            /// this handle points into.  Can be used to check if two iterators point
            /// into the same data structure.
            const void *getEpochAddress() const { return EpochAddress; }
        };
    };
}

#endif /* DebugEpochBase_hpp */

