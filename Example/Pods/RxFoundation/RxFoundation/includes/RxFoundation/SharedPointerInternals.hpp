// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include <RxFoundation/RxBase.hpp>
#include <RxFoundation/Atomic.hpp>

/** Default behavior. */
#define	FORCE_THREADSAFE_SHAREDPTRS 1

namespace Rx {
    
    /**
      *ESPMode is used select between either 'fast' or 'thread safe' shared pointer types.
      *This is only used by templates at compile time to generate one code path or another.
     */
    enum class ESPMode {
        /** Forced to be not thread-safe. */
        NotThreadSafe = 0,
        
        /**
         *	Fast, doesn't ever use atomic interlocks.
         *	Some code requires that all shared pointers are thread-safe.
         *	It's better to change it here, instead of replacing ESPMode::Fast to ESPMode::ThreadSafe throughout the code.
         */
        Fast = FORCE_THREADSAFE_SHAREDPTRS ? 1 : 0,
        
        /** Conditionally thread-safe, never spin locks, but slower */
        ThreadSafe = 1
    };
    
    
    // Forward declarations.  Note that in the interest of fast performance, thread safety
    // features are mostly turned off (Mode = ESPMode::Fast).  If you need to access your
    // object on multiple threads, you should use ESPMode::ThreadSafe!
    template <typename ObjectType, ESPMode Mode = ESPMode::Fast> class SharedRef;
    template <typename ObjectType, ESPMode Mode = ESPMode::Fast> class SharedPtr;
    template <typename ObjectType, ESPMode Mode = ESPMode::Fast> class WeakPtr;
    template <typename ObjectType, ESPMode Mode = ESPMode::Fast> class SharedFromThis;
    
    
    /**
      *SharedPointerInternals contains internal workings of shared and weak pointers.  You should
      *hopefully never have to use anything inside this namespace directly.
     */
    namespace SharedPointerInternals {
        // Forward declarations
        template <ESPMode Mode> class WeakReferencer;
        
        /** Dummy structures used internally as template arguments for typecasts */
        struct StaticCastTag {};
        struct ConstCastTag {};
        struct DynamicCastTag {};
        
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        struct NullTag {};
        
        
        class ReferenceControllerBase {
        public:
            /** Constructor */
            RX_INLINE explicit ReferenceControllerBase(void *InObject) :
            SharedReferenceCount(1),
            WeakReferenceCount(1),
            Object(InObject) {
            }
            
            // NOTE: The primary reason these reference counters are 32-bit values (and not 16-bit to save
            //       memory), is that atomic operations require at least 32-bit objects.
            
            /** Number of shared references to this object.  When this count reaches zero, the associated object
             will be destroyed (even if there are still weak references!) */
            Int32 SharedReferenceCount;
            
            /** Number of weak references to this object.  If there are any shared references, that counts as one
             weak reference too. */
            Int32 WeakReferenceCount;
            
            /** The object associated with this reference counter.  */
            void *Object;
            
            /** Destroys the object associated with this reference counter.  */
            virtual void DestroyObject() = 0;
            
            /** Destroys the object associated with this reference counter.  */
            virtual ~ReferenceControllerBase() {};
            
        private:
            ReferenceControllerBase(ReferenceControllerBase const&);
            ReferenceControllerBase &operator=(ReferenceControllerBase const&);
        };
        
        template <typename ObjectType, typename DeleterType>
        class ReferenceControllerWithDeleter : private DeleterType, public ReferenceControllerBase {
        public:
            explicit ReferenceControllerWithDeleter(void *Object, DeleterType &&Deleter) :
            DeleterType             (std::move(Deleter)),
            ReferenceControllerBase(Object) {
            }
            
            virtual void DestroyObject() {            
                (*static_cast<DeleterType*>(this))((ObjectType*)static_cast<ReferenceControllerBase*>(this)->Object);
            }
            
            virtual ~ReferenceControllerWithDeleter() {
            }
        };
        
        
        /** Deletes an object via the standard delete operator */
        template <typename Type>
        struct DefaultDeleter {
            RX_INLINE void operator()(Type *Object) const {
                delete Object;
            }
        };
        
        /** Creates a reference controller which just calls delete */
        template <typename ObjectType>
        inline ReferenceControllerBase *NewDefaultReferenceController(ObjectType *Object) {
            return new ReferenceControllerWithDeleter<ObjectType, DefaultDeleter<ObjectType>>(Object, DefaultDeleter<ObjectType>());
        }
        
        /** Creates a custom reference controller with a specified deleter */
        template <typename ObjectType, typename DeleterType>
        inline ReferenceControllerBase *NewCustomReferenceController(ObjectType *Object, DeleterType &&Deleter) {
            return new ReferenceControllerWithDeleter<ObjectType, typename std::remove_reference<DeleterType>::Type>(Object, std::forward<DeleterType>(Deleter));
        }
        
        
        /** Proxy structure for implicitly converting raw pointers to shared/weak pointers */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename ObjectType>
        struct RawPtrProxy {
            /** The object pointer */
            ObjectType *Object;
            
            /** Reference controller used to destroy the object */
            ReferenceControllerBase *ReferenceController;
            
            /** Construct implicitly from an object */
            RX_INLINE RawPtrProxy(ObjectType *InObject) :
            Object             (InObject),
            ReferenceController(NewDefaultReferenceController(InObject)) {            
            }
            
            /** Construct implicitly from an object and a custom deleter */
            template <typename Deleter>
            RX_INLINE RawPtrProxy(ObjectType *InObject, Deleter &&InDeleter) :
            Object             (InObject),
            ReferenceController(NewCustomReferenceController(InObject, std::forward<Deleter>(InDeleter))) {            
            }
        };
        
        
        /**
          *FReferenceController is a standalone heap-allocated object that tracks the number of references
          *to an object referenced by SharedRef, SharedPtr or WeakPtr objects.
         *
          *It is specialized for different threading modes.
         */
        template <ESPMode Mode>
        struct ReferenceControllerOps;
        
        template <>
        struct ReferenceControllerOps<ESPMode::ThreadSafe> {
            /** Returns the shared reference count */
            static RX_INLINE const Int32 GetSharedReferenceCount(const ReferenceControllerBase *ReferenceController) {
                // This reference count may be accessed by multiple threads
                return static_cast<Int32 const volatile&>(ReferenceController->SharedReferenceCount);
            }
            
            /** Adds a shared reference to this counter */
            static RX_INLINE void AddSharedReference(ReferenceControllerBase *ReferenceController) {
                Atomics::InterlockedIncrement(&ReferenceController->SharedReferenceCount);
            }
            
            /**
              *Adds a shared reference to this counter ONLY if there is already at least one reference
             *
              *@return  True if the shared reference was added successfully
             */
            static bool ConditionallyAddSharedReference(ReferenceControllerBase *ReferenceController) {            
                for (;;) {
                    // Peek at the current shared reference count.  Remember, this value may be updated by
                    // multiple threads.
                    const Int32 OriginalCount = static_cast<Int32 const volatile&>(ReferenceController->SharedReferenceCount);
                    if (OriginalCount == 0) {
                        // Never add a shared reference if the pointer has already expired
                        return false;
                    }
                    
                    // Attempt to increment the reference count.
                    const Int32 ActualOriginalCount = Atomics::InterlockedCompareExchange(&ReferenceController->SharedReferenceCount, OriginalCount + 1, OriginalCount);
                    
                    // We need to make sure that we never revive a counter that has already expired, so if the
                    // actual value what we expected (because it was touched by another thread), then we'll try
                    // again.  Note that only in very unusual cases will this actually have to loop.
                    if (ActualOriginalCount == OriginalCount) {
                        return true;
                    }
                }
            }
            
            /** Releases a shared reference to this counter */
            static RX_INLINE void ReleaseSharedReference(ReferenceControllerBase *ReferenceController) {
                RxCheck(ReferenceController->SharedReferenceCount > 0);
                
                if (Atomics::InterlockedDecrement(&ReferenceController->SharedReferenceCount) == 0) {
                    // Last shared reference was released!  Destroy the referenced object.
                    ReferenceController->DestroyObject();
                    
                    // No more shared referencers, so decrement the weak reference count by one.  When the weak
                    // reference count reaches zero, this object will be deleted.
                    ReleaseWeakReference(ReferenceController);
                }
            }
            
            
            /** Adds a weak reference to this counter */
            static RX_INLINE void AddWeakReference(ReferenceControllerBase *ReferenceController) {
                Atomics::InterlockedIncrement(&ReferenceController->WeakReferenceCount);
            }
            
            /** Releases a weak reference to this counter */
            static void ReleaseWeakReference(ReferenceControllerBase *ReferenceController) {            
                RxCheck(ReferenceController->WeakReferenceCount> 0);
                
                if (Atomics::InterlockedDecrement(&ReferenceController->WeakReferenceCount) == 0) {
                    // No more references to this reference count.  Destroy it!
                    delete ReferenceController;
                }
            }
        };
        
        
        template <>
        struct ReferenceControllerOps<ESPMode::NotThreadSafe> {
            /** Returns the shared reference count */
            static RX_INLINE const Int32 GetSharedReferenceCount(const ReferenceControllerBase *ReferenceController) {
                return ReferenceController->SharedReferenceCount;
            }
            
            /** Adds a shared reference to this counter */
            static RX_INLINE void AddSharedReference(ReferenceControllerBase *ReferenceController) {
                ++ReferenceController->SharedReferenceCount;
            }
            
            /**
              *Adds a shared reference to this counter ONLY if there is already at least one reference
             *
              *@return  True if the shared reference was added successfully
             */
            static bool ConditionallyAddSharedReference(ReferenceControllerBase *ReferenceController) {            
                if (ReferenceController->SharedReferenceCount == 0) {
                    // Never add a shared reference if the pointer has already expired
                    return false;
                }
                
                ++ReferenceController->SharedReferenceCount;
                return true;
            }
            
            /** Releases a shared reference to this counter */
            static RX_INLINE void ReleaseSharedReference(ReferenceControllerBase *ReferenceController) {
                RxCheck(ReferenceController->SharedReferenceCount > 0);
                
                if (--ReferenceController->SharedReferenceCount == 0) {
                    // Last shared reference was released!  Destroy the referenced object.
                    ReferenceController->DestroyObject();
                    
                    // No more shared referencers, so decrement the weak reference count by one.  When the weak
                    // reference count reaches zero, this object will be deleted.
                    ReleaseWeakReference(ReferenceController);
                }
            }
            
            /** Adds a weak reference to this counter */
            static RX_INLINE void AddWeakReference(ReferenceControllerBase *ReferenceController) {
                ++ReferenceController->WeakReferenceCount;
            }
            
            /** Releases a weak reference to this counter */
            static void ReleaseWeakReference(ReferenceControllerBase *ReferenceController) {            
                RxCheck(ReferenceController->WeakReferenceCount> 0);
                
                if (--ReferenceController->WeakReferenceCount == 0) {
                    // No more references to this reference count.  Destroy it!
                    delete ReferenceController;
                }
            }
        };
        
        
        /**
          *SharedReferencer is a wrapper around a pointer to a reference controller that is used by either a
          *SharedRef or a SharedPtr to keep track of a referenced object's lifetime
         */
        template <ESPMode Mode>
        class SharedReferencer {
            typedef ReferenceControllerOps<Mode> TOps;
            
        public:
            
            /** Constructor for an empty shared referencer object */
            RX_INLINE SharedReferencer() :
            ReferenceController(nullptr) {             }
            
            /** Constructor that counts a single reference to the specified object */
            inline explicit SharedReferencer(ReferenceControllerBase *InReferenceController) :
            ReferenceController(InReferenceController) {             }
            
            /** Copy constructor creates a new reference to the existing object */
            RX_INLINE SharedReferencer(SharedReferencer const &InSharedReference) :
            ReferenceController(InSharedReference.ReferenceController) {            
                // If the incoming reference had an object associated with it, then go ahead and increment the
                // shared reference count
                if (ReferenceController != nullptr) {
                    TOps::AddSharedReference(ReferenceController);
                }
            }
            
            /** Move constructor creates no new references */
            RX_INLINE SharedReferencer(SharedReferencer &&InSharedReference) :
            ReferenceController(InSharedReference.ReferenceController) {            
                InSharedReference.ReferenceController = nullptr;
            }
            
            /** Creates a shared referencer object from a weak referencer object.  This will only result
             in a valid object reference if the object already has at least one other shared referencer. */
            SharedReferencer(WeakReferencer<Mode> const &InWeakReference) :
            ReferenceController(InWeakReference.ReferenceController) {            
                // If the incoming reference had an object associated with it, then go ahead and increment the
                // shared reference count
                if (ReferenceController != nullptr) {
                    // Attempt to elevate a weak reference to a shared one.  For this to work, the object this
                    // weak counter is associated with must already have at least one shared reference.  We'll
                    // never revive a pointer that has already expired!
                    if (!TOps::ConditionallyAddSharedReference(ReferenceController)) {
                        ReferenceController = nullptr;
                    }
                }
            }
            
            /** Destructor. */
            RX_INLINE ~SharedReferencer() {
                if (ReferenceController != nullptr) {
                    // Tell the reference counter object that we're no longer referencing the object with
                    // this shared pointer
                    TOps::ReleaseSharedReference(ReferenceController);
                }
            }
            
            /** Assignment operator adds a reference to the assigned object.  If this counter was previously
             referencing an object, that reference will be released. */
            inline SharedReferencer &operator=(SharedReferencer const &InSharedReference) {            
                // Make sure we're not be reassigned to ourself!
                ReferenceControllerBase *NewReferenceController = InSharedReference.ReferenceController;
                if (NewReferenceController != ReferenceController) {
                    // First, add a shared reference to the new object
                    if (NewReferenceController != nullptr) {
                        TOps::AddSharedReference(NewReferenceController);
                    }
                    
                    // Release shared reference to the old object
                    if (ReferenceController != nullptr) {
                        TOps::ReleaseSharedReference(ReferenceController);
                        ReferenceController = nullptr;
                    }
                    
                    // Assume ownership of the assigned reference counter
//                    ReferenceController = NewReferenceController;
                    Atomics::InterlockedExchangePtr((void **)&ReferenceController, NewReferenceController);
                }
                
                return *this;
            }
            
            /** Move assignment operator adds no references to the assigned object.  If this counter was previously
             referencing an object, that reference will be released. */
            inline SharedReferencer &operator=(SharedReferencer &&InSharedReference) {            
                // Make sure we're not be reassigned to ourself!
                auto NewReferenceController = InSharedReference.ReferenceController;
                auto OldReferenceController = ReferenceController;
                if (NewReferenceController != OldReferenceController) {
                    // Assume ownership of the assigned reference counter
                    InSharedReference.ReferenceController = nullptr;
                    ReferenceController                   = NewReferenceController;
                    
                    // Release shared reference to the old object
                    if (OldReferenceController != nullptr) {
                        TOps::ReleaseSharedReference(OldReferenceController);
                    }
                }
                
                return *this;
            }
            
            /**
              *Tests to see whether or not this shared counter contains a valid reference
             *
              *@return  True if reference is valid
             */
            RX_INLINE const bool isValid() const {
                return ReferenceController != nullptr;
            }
            
            /**
              *Returns the number of shared references to this object (including this reference.)
             *
              *@return  Number of shared references to the object (including this reference.)
             */
            RX_INLINE const Int32 GetSharedReferenceCount() const {
                return ReferenceController != nullptr ? TOps::GetSharedReferenceCount(ReferenceController) : 0;
            }
            
            /**
              *Returns true if this is the only shared reference to this object.  Note that there may be
              *outstanding weak references left.
             *
              *@return  True if there is only one shared reference to the object, and this is it!
             */
            RX_INLINE const bool isUnique() const {
                return GetSharedReferenceCount() == 1;
            }
            
        private:
            
            // Expose access to ReferenceController to WeakReferencer
            template <ESPMode OtherMode> friend class WeakReferencer;
            
        private:
            
            /** Pointer to the reference controller for the object a shared reference/pointer is referencing */
            ReferenceControllerBase *ReferenceController;
        };
        
        
        /**
          *WeakReferencer is a wrapper around a pointer to a reference controller that is used
          *by a WeakPtr to keep track of a referenced object's lifetime.
         */
        template <ESPMode Mode>
        class WeakReferencer {
            typedef ReferenceControllerOps<Mode> TOps;
            
        public:
            
            /** Default constructor with empty counter */
            RX_INLINE WeakReferencer() :
            ReferenceController(nullptr) { }
            
            /** Construct a weak referencer object from another weak referencer */
            RX_INLINE WeakReferencer(WeakReferencer const &InWeakRefCountPointer) :
            ReferenceController(InWeakRefCountPointer.ReferenceController) {            
                // If the weak referencer has a valid controller, then go ahead and add a weak reference to it!
                if (ReferenceController != nullptr) {
                    TOps::AddWeakReference(ReferenceController);
                }
            }
            
            /** Construct a weak referencer object from an rvalue weak referencer */
            RX_INLINE WeakReferencer(WeakReferencer &&InWeakRefCountPointer) :
            ReferenceController(InWeakRefCountPointer.ReferenceController) {            
                InWeakRefCountPointer.ReferenceController = nullptr;
            }
            
            /** Construct a weak referencer object from a shared referencer object */
            RX_INLINE WeakReferencer(SharedReferencer<Mode> const &InSharedRefCountPointer) :
            ReferenceController(InSharedRefCountPointer.ReferenceController) {            
                // If the shared referencer had a valid controller, then go ahead and add a weak reference to it!
                if (ReferenceController != nullptr) {
                    TOps::AddWeakReference(ReferenceController);
                }
            }
            
            /** Destructor. */
            RX_INLINE ~WeakReferencer() {
                if (ReferenceController != nullptr) {
                    // Tell the reference counter object that we're no longer referencing the object with
                    // this weak pointer
                    TOps::ReleaseWeakReference(ReferenceController);
                }
            }
            
            /** Assignment operator from a weak referencer object.  If this counter was previously referencing an
             object, that reference will be released. */
            RX_INLINE WeakReferencer &operator=(WeakReferencer const &InWeakReference) {
                AssignReferenceController(InWeakReference.ReferenceController);
                
                return *this;
            }
            
            /** Assignment operator from an rvalue weak referencer object.  If this counter was previously referencing an
             object, that reference will be released. */
            RX_INLINE WeakReferencer &operator=(WeakReferencer &&InWeakReference) {
                auto OldReferenceController         = ReferenceController;
                ReferenceController                 = InWeakReference.ReferenceController;
                InWeakReference.ReferenceController = nullptr;
                if (OldReferenceController != nullptr) {
                    TOps::ReleaseWeakReference(OldReferenceController);
                }
                
                return *this;
            }
            
            /** Assignment operator from a shared reference counter.  If this counter was previously referencing an
             object, that reference will be released. */
            RX_INLINE WeakReferencer &operator=(SharedReferencer<Mode> const &InSharedReference) {
                AssignReferenceController(InSharedReference.ReferenceController);
                
                return *this;
            }
            
            /**
              *Tests to see whether or not this weak counter contains a valid reference
             *
              *@return  True if reference is valid
             */
            RX_INLINE const bool isValid() const {
                return ReferenceController != nullptr  &&TOps::GetSharedReferenceCount(ReferenceController)> 0;
            }
            
        private:
            
            /** Assigns a new reference controller to this counter object, first adding a reference to it, then
             releasing the previous object. */
            inline void AssignReferenceController(ReferenceControllerBase *NewReferenceController) {            
                // Only proceed if the new reference counter is different than our current
                if (NewReferenceController != ReferenceController) {
                    // First, add a weak reference to the new object
                    if (NewReferenceController != nullptr) {
                        TOps::AddWeakReference(NewReferenceController);
                    }
                    
                    // Release weak reference to the old object
                    if (ReferenceController != nullptr) {
                        TOps::ReleaseWeakReference(ReferenceController);
                    }
                    
                    // Assume ownership of the assigned reference counter
                    ReferenceController = NewReferenceController;
                }
            }
            
        private:
            
            /** Expose access to ReferenceController to SharedReferencer. */
            template <ESPMode OtherMode> friend class SharedReferencer;
            
        private:
            
            /** Pointer to the reference controller for the object a WeakPtr is referencing */
            ReferenceControllerBase *ReferenceController;
        };
        
        
        /** Templated helper function (const) that creates a shared pointer from an object instance */
        template <typename SharedPtrType, class ObjectType, class OtherType, ESPMode Mode>
        RX_INLINE void EnableSharedFromThis(SharedPtr<SharedPtrType, Mode> const *InSharedPtr, ObjectType const *InObject, SharedFromThis<OtherType, Mode> const *InShareable) {
            if (InShareable != nullptr) {            
                InShareable->UpdateWeakReferenceInternal(InSharedPtr, const_cast<ObjectType*>(InObject));
            }
        }
        
        
        /** Templated helper function that creates a shared pointer from an object instance */
        template <typename SharedPtrType, class ObjectType, class OtherType, ESPMode Mode>
        RX_INLINE void EnableSharedFromThis(SharedPtr<SharedPtrType, Mode> *InSharedPtr, ObjectType const *InObject, SharedFromThis<OtherType, Mode> const *InShareable) {
            if (InShareable != nullptr) {            
                InShareable->UpdateWeakReferenceInternal(InSharedPtr, const_cast<ObjectType*>(InObject));
            }
        }
        
        
        /** Templated helper function (const) that creates a shared reference from an object instance */
        template <typename SharedRefType, class ObjectType, class OtherType, ESPMode Mode>
        RX_INLINE void EnableSharedFromThis(SharedRef<SharedRefType, Mode> const *InSharedRef, ObjectType const *InObject, SharedFromThis<OtherType, Mode> const *InShareable) {
            if (InShareable != nullptr) {            
                InShareable->UpdateWeakReferenceInternal(InSharedRef, const_cast<ObjectType*>(InObject));
            }
        }
        
        
        /** Templated helper function that creates a shared reference from an object instance */
        template <typename SharedRefType, class ObjectType, class OtherType, ESPMode Mode>
        RX_INLINE void EnableSharedFromThis(SharedRef<SharedRefType, Mode> *InSharedRef, ObjectType const *InObject, SharedFromThis<OtherType, Mode> const *InShareable) {
            if (InShareable != nullptr) {            
                InShareable->UpdateWeakReferenceInternal(InSharedRef, const_cast<ObjectType*>(InObject));
            }
        }
        
        
        /** Templated helper catch-all function, accomplice to the above helper functions */
        RX_INLINE void EnableSharedFromThis(...) { }
    }
}
