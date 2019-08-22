// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 *	SharedPointer - Unreal smart pointer library
 *
 *	This is a smart pointer library consisting of shared references (SharedRef), shared pointers (SharedPtr),
 *	weak pointers (WeakPtr) as well as related helper functions and classes.  This implementation is modeled
 *	after the C++0x standard library's shared_ptr as well as Boost smart pointers.
 *
 *	Benefits of using shared references and pointers:
 *
 *		Clean syntax.  You can copy, dereference and compare shared pointers just like regular C++ pointers.
 *		Prevents memory leaks.  Resources are destroyed automatically when there are no more shared references.
 *		Weak referencing.  Weak pointers allow you to safely RxCheck when an object has been destroyed.
 *		Thread safety.  Includes "thread safe" version that can be safely accessed from multiple threads.
 *		Ubiquitous.  You can create shared pointers to virtually *any *type of object.
 *		Runtime safety.  Shared references are never null and can always be dereferenced.
 *		No reference cycles.  Use weak pointers to break reference cycles.
 *		Confers intent.  You can easily tell an object *owner *from an *observer*.
 *		Performance.  Shared pointers have minimal overhead.  All operations are constant-time.
 *		Robust features.  Supports 'const', forward declarations to incomplete types, type-casting, etc.
 *		Memory.  Only twice the size of a C++ pointer in 64-bit (plus a shared 16-byte reference controller.)
 *
 *
 *	This library contains the following smart pointers:
 *
 *		SharedRef - Non-nullable, reference counted non-intrusive authoritative smart pointer
 *		SharedPtr - Reference counted non-intrusive authoritative smart pointer
 *		WeakPtr - Reference counted non-intrusive weak pointer reference
 *
 *
 *	Additionally, the following helper classes and functions are defined:
 *
 *		MakeShareable() - Used to initialize shared pointers from C++ pointers (enables implicit conversion)
 *		SharedFromThis - You can derive your own class from this to acquire a SharedRef from "this"
 *		StaticCastSharedRef() - Static cast utility function, typically used to downcast to a derived type.
 *		ConstCastSharedRef() - Converts a 'const' reference to 'mutable' smart reference
 *		StaticCastSharedPtr() - Dynamic cast utility function, typically used to downcast to a derived type.
 *		ConstCastSharedPtr() - Converts a 'const' smart pointer to 'mutable' smart pointer
 *
 *
 *	Examples:
 *		- Please see 'SharedPointerTesting.h' for various examples of shared pointers and references!
 *
 *
 *	Tips:
 *		- Use SharedRef instead of SharedPtr whenever possible -- it can never be nullptr!
 *		- You can call SharedPtr::reset() to release a reference to your object (and potentially deallocate)
 *		- Use the MakeShareable() helper function to implicitly convert to TSharedRefs or TSharedPtrs
 *		- You can never reset a SharedRef or assign it to nullptr, but you can assign it a new object
 *		- Shared pointers assume ownership of objects -- no need to call delete yourself!
 *		- Usually you should "operator new" when passing a C++ pointer to a new shared pointer
 *		- Use SharedRef or SharedPtr when passing smart pointers as function parameters, not WeakPtr
 *		- The "thread-safe" versions of smart pointers are a bit slower -- only use them when needed
 *		- You can forward declare shared pointers to incomplete types, just how you'd expect to!
 *		- Shared pointers of compatible types will be converted implicitly (e.g. upcasting)
 *		- You can create a typedef to SharedRef<MyClass> to make it easier to type
 *		- For best performance, minimize calls to WeakPtr::pin (or conversions to SharedRef/SharedPtr)
 *		- Your class can return itself as a shared reference if you derive from SharedFromThis
 *		- To downcast a pointer to a derived object class, to the StaticCastSharedPtr function
 *		- 'const' objects are fully supported with shared pointers!
 *		- You can make a 'const' shared pointer mutable using the ConstCastSharedPtr function
 *
 *
 *	Limitations:
 *
 *		- Shared pointers are not compatible with Unreal objects (UObject classes)!
 *		- Currently only types with that have regular destructors (no custom deleters)
 *		- Dynamically-allocated arrays are not supported yet (e.g. MakeSharable(new Int32[20]))
 *		- Implicit conversion of SharedPtr/SharedRef to bool is not supported yet
 *
 *
 *	Differences from other implementations (e.g. boost:shared_ptr, std::shared_ptr):
 *
 *		- Type names and method names are more consistent with Unreal's codebase
 *		- You must use pin() to convert weak pointers to shared pointers (no explicit constructor)
 *		- Thread-safety features are optional instead of forced
 *		- SharedFromThis returns a shared *reference*, not a shared *pointer*
 *		- Some features were omitted (e.g. use_count(), unique(), etc.)
 *		- No exceptions are allowed (all related features have been omitted)
 *		- Custom allocators and custom delete functions are not supported yet
 *		- Our implementation supports non-nullable smart pointers (SharedRef)
 *		- Several other new features added, such as MakeShareable and nullptr assignment
 *
 *
 *	Why did we write our own Unreal shared pointer instead of using available alternatives?
 *
 *		- std::shared_ptr (and even tr1::shared_ptr) is not yet available on all platforms
 *		- Allows for a more consistent implementation on all compilers and platforms
 *		- Can work seamlessly with other Unreal containers and types
 *		- Better control over platform specifics, including threading and optimizations
 *		- We want thread-safety features to be optional (for performance)
 *		- We've added our own improvements (MakeShareable, assign to nullptr, etc.)
 *		- Exceptions were not needed nor desired in our implementation
 *		- We wanted more control over performance (inlining, memory, use of virtuals, etc.)
 *		- Potentially easier to debug (liberal code comments, etc.)
 *		- Prefer not to introduce new third party dependencies when not needed
 *
 */

// SharedPointerInternals.h contains the implementation of reference counting structures we need
#include <RxFoundation/SharedPointerInternals.hpp>

namespace Rx {
    template <typename From, typename To>
    struct TPointerIsConvertibleFromTo {
    private:
        static UInt8  Test(...);
        static UInt16 Test(To*);
        
    public:
        enum { Value  = sizeof(Test((From*)nullptr)) - 1 };
    };
    
    class TPointerIsConvertibleFromTo_TestBase {
    };
    
    class TPointerIsConvertibleFromTo_TestDerived : public TPointerIsConvertibleFromTo_TestBase {
    };
    
    class TPointerIsConvertibleFromTo_Unrelated {
    };
    
    static_assert(TPointerIsConvertibleFromTo<bool, bool>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<void, void>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<bool, void>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<const bool, const void>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<TPointerIsConvertibleFromTo_TestDerived, TPointerIsConvertibleFromTo_TestBase>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<TPointerIsConvertibleFromTo_TestDerived, const TPointerIsConvertibleFromTo_TestBase>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<const TPointerIsConvertibleFromTo_TestDerived, const TPointerIsConvertibleFromTo_TestBase>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<TPointerIsConvertibleFromTo_TestBase, TPointerIsConvertibleFromTo_TestBase>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(TPointerIsConvertibleFromTo<TPointerIsConvertibleFromTo_TestBase, void>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    
    static_assert(!TPointerIsConvertibleFromTo<TPointerIsConvertibleFromTo_TestBase, TPointerIsConvertibleFromTo_TestDerived>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(!TPointerIsConvertibleFromTo<TPointerIsConvertibleFromTo_Unrelated, TPointerIsConvertibleFromTo_TestBase>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(!TPointerIsConvertibleFromTo<bool, TPointerIsConvertibleFromTo_TestBase>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(!TPointerIsConvertibleFromTo<void, TPointerIsConvertibleFromTo_TestBase>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(!TPointerIsConvertibleFromTo<TPointerIsConvertibleFromTo_TestBase, bool>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    static_assert(!TPointerIsConvertibleFromTo<void, bool>::Value, "Platform TPointerIsConvertibleFromTo test failed.");
    /**
      *Casts a shared reference of one type to another type. (static_cast)  Useful for down-casting.
     *
      *@param  InSharedRef  The shared reference to cast
     */
    template <typename CastToType, class CastFromType, ESPMode Mode>
    RX_INLINE_VISIBILITY SharedRef<CastToType, Mode> StaticCastSharedRef(SharedRef<CastFromType, Mode> const &InSharedRef) {
        return SharedRef<CastToType, Mode>(InSharedRef, SharedPointerInternals::StaticCastTag());
    }
    
    template <typename CastToType, class CastFromType, ESPMode Mode>
    RX_INLINE_VISIBILITY SharedRef<CastToType, Mode> DynamicCastSharedRef(SharedRef<CastFromType, Mode> const &InSharedRef) {
        return SharedRef<CastToType, Mode>(InSharedRef, SharedPointerInternals::DynamicCastTag());
    }
    
    /**
      *SharedRef is a non-nullable, non-intrusive reference-counted authoritative object reference.
     *
      *This shared reference will be conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
     */
    // NOTE: SharedRef is an Unreal extension to standard smart pointer feature set
    template <typename ObjectType, ESPMode Mode>
    class SharedRef {
        
    public:
        using value_type = ObjectType;
        // NOTE: SharedRef has no default constructor as it does not support empty references.  You must
        //		 initialize your SharedRef to a valid object at construction time.
        
        /**
          *Constructs a shared reference that owns the specified object.  Must not be nullptr.
         *
          *@param  InObject  Object this shared reference to retain a reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY explicit SharedRef(OtherType *InObject) :
        Object(InObject),
        SharedReferenceCount(SharedPointerInternals::NewDefaultReferenceController(InObject)) {
            Init(InObject);
        }
        
        /**
          *Constructs a shared reference that owns the specified object.  Must not be nullptr.
         *
          *@param  InObject   Object this shared pointer to retain a reference to
          *@param  InDeleter  Deleter object used to destroy the object when it is no longer referenced.
         */
        template <typename OtherType, class DeleterType>
        RX_INLINE_VISIBILITY SharedRef(OtherType *InObject, DeleterType &&InDeleter) :
        Object(InObject),
        SharedReferenceCount(SharedPointerInternals::NewCustomReferenceController(InObject, std::forward<DeleterType>(InDeleter))) {
            Init(InObject);
        }
        
#if WITH_HOT_RELOAD_CTORS
        /**
          *Constructs default shared reference that owns the default object for specified type.
         *
          *Used internally only. Please do not use!
         */
        SharedRef() :
        Object(new ObjectType()),
        SharedReferenceCount(SharedPointerInternals::NewDefaultReferenceController(Object)) {
            EnsureRetrievingVTablePtrDuringCtor(TEXT("SharedRef()"));
            Init(Object);
        }
#endif // WITH_HOT_RELOAD_CTORS
        
        /**
          *Constructs a shared reference using a proxy reference to a raw pointer. (See MakeShareable())
          *Must not be nullptr.
         *
          *@param  InRawPtrProxy  Proxy raw pointer that contains the object that the new shared reference will reference
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedRef(SharedPointerInternals::RawPtrProxy<OtherType> const &InRawPtrProxy) :
        Object(InRawPtrProxy.Object),
        SharedReferenceCount(InRawPtrProxy.ReferenceController) {
            // If the following assert goes off, it means a SharedRef was initialized from a nullptr object pointer.
            // Shared references must never be nullptr, so either pass a valid object or consider using SharedPtr instead.
            RxCheck(InRawPtrProxy.Object != nullptr);
            
            // If the object happens to be derived from SharedFromThis, the following method
            // will prime the object with a weak pointer to itself.
            SharedPointerInternals::EnableSharedFromThis(this, InRawPtrProxy.Object, InRawPtrProxy.Object);
        }
        
        /**
          *Constructs a shared reference as a reference to an existing shared reference's object.
          *This constructor is needed so that we can implicitly upcast to base classes.
         *
          *@param  InSharedRef  The shared reference whose object we should create an additional reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedRef(SharedRef<OtherType, Mode> const &InSharedRef) :
        Object(InSharedRef.Object),
        SharedReferenceCount(InSharedRef.SharedReferenceCount) {
        }
        
        /**
          *Special constructor used internally to statically cast one shared reference type to another.  You
          *should never call this constructor directly.  Instead, use the StaticCastSharedRef() function.
          *This constructor creates a shared reference as a shared reference to an existing shared reference after
          *statically casting that reference's object.  This constructor is needed for static casts.
         *
          *@param  InSharedRef  The shared reference whose object we should create an additional reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedRef(SharedRef<OtherType, Mode> const &InSharedRef, SharedPointerInternals::StaticCastTag) :
        Object(static_cast<ObjectType*>(InSharedRef.Object)),
        SharedReferenceCount(InSharedRef.SharedReferenceCount) {
        }
        
        /**
          *Special constructor used internally to cast a 'const' shared reference a 'mutable' reference.  You
          *should never call this constructor directly.  Instead, use the ConstCastSharedRef() function.
          *This constructor creates a shared reference as a shared reference to an existing shared reference after
          *const casting that reference's object.  This constructor is needed for const casts.
         *
          *@param  InSharedRef  The shared reference whose object we should create an additional reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedRef(SharedRef<OtherType, Mode> const &InSharedRef, SharedPointerInternals::ConstCastTag) :
        Object(const_cast<ObjectType*>(InSharedRef.Object)),
        SharedReferenceCount(InSharedRef.SharedReferenceCount) {
        }
        
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedRef(SharedRef<OtherType, Mode> const &InSharedRef, SharedPointerInternals::DynamicCastTag) :
        Object(dynamic_cast<ObjectType*>(InSharedRef.Object)),
        SharedReferenceCount(InSharedRef.SharedReferenceCount) {
        }
        
        /**
          *Special constructor used internally to create a shared reference from an existing shared reference,
          *while using the specified object reference instead of the incoming shared reference's object
          *pointer.  This is used by with the SharedFromThis feature (by UpdateWeakReferenceInternal)
         *
          *@param  OtherSharedRef  The shared reference whose reference count
          *@param  InObject  The object pointer to use (instead of the incoming shared reference's object)
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedRef(SharedRef<OtherType, Mode> const &OtherSharedRef, ObjectType *InObject) :
        Object(InObject),
        SharedReferenceCount(OtherSharedRef.SharedReferenceCount) {
        }
        
        RX_INLINE_VISIBILITY SharedRef(SharedRef const &InSharedRef) :
        Object(InSharedRef.Object),
        SharedReferenceCount(InSharedRef.SharedReferenceCount) {
        }
        
        RX_INLINE_VISIBILITY SharedRef(SharedRef &&InSharedRef) :
        Object(InSharedRef.Object),
        SharedReferenceCount(InSharedRef.SharedReferenceCount) {
            // We're intentionally not moving here, because we don't want to leave InSharedRef in a
            // null state, because that breaks the class invariant.  But we provide a move constructor
            // anyway in case the compiler complains that we have a move assign but no move construct.
        }
        
        /**
          *Assignment operator replaces this shared reference with the specified shared reference.  The object
          *currently referenced by this shared reference will no longer be referenced and will be deleted if
          *there are no other referencers.
         *
          *@param  InSharedRef  Shared reference to replace with
         */
        RX_INLINE_VISIBILITY SharedRef &operator=(SharedRef const &InSharedRef) {
            SharedReferenceCount = InSharedRef.SharedReferenceCount;
            Object = InSharedRef.Object;
            return *this;
        }
        
        RX_INLINE_VISIBILITY SharedRef &operator=(SharedRef &&InSharedRef) {
            Memory::swap(this, &InSharedRef, sizeof(SharedRef));
            return *this;
        }
        
        /**
          *Assignment operator replaces this shared reference with the specified shared reference.  The object
          *currently referenced by this shared reference will no longer be referenced and will be deleted if
          *there are no other referencers.  Must not be nullptr.
         *
          *@param  InRawPtrProxy  Proxy object used to assign the object (see MakeShareable helper function)
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedRef &operator=(SharedPointerInternals::RawPtrProxy<OtherType> const &InRawPtrProxy) {
            // If the following assert goes off, it means a SharedRef was initialized from a nullptr object pointer.
            // Shared references must never be nullptr, so either pass a valid object or consider using SharedPtr instead.
            RxCheck(InRawPtrProxy.Object != nullptr);
            
            *this = SharedRef<ObjectType, Mode>(InRawPtrProxy.Object);
            return *this;
        }
        
        RX_INLINE_VISIBILITY bool operator<(const SharedRef<ObjectType> &rhs) const {
            return Object < rhs.Object;
        }
        
        /**
          *Returns a C++ reference to the object this shared reference is referencing
         *
          *@return  The object owned by this shared reference
         */
        RX_INLINE_VISIBILITY ObjectType& get() const {
            // Should never be nullptr as SharedRef is never nullable
            RxCheck(isValid());
            return *Object;
        }
        
        /**
          *Dereference operator returns a reference to the object this shared pointer points to
         *
          *@return  Reference to the object
         */
        RX_INLINE_VISIBILITY ObjectType& operator*() const {
            // Should never be nullptr as SharedRef is never nullable
            RxCheck(isValid());
            return *Object;
        }
        
        /**
          *Arrow operator returns a pointer to this shared reference's object
         *
          *@return  Returns a pointer to the object referenced by this shared reference
         */
        RX_INLINE_VISIBILITY ObjectType *operator->() const {
            // Should never be nullptr as SharedRef is never nullable
            RxCheck(isValid());
            return Object;
        }
        
        /**
          *Returns the number of shared references to this object (including this reference.)
          *IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
         *
          *@return  Number of shared references to the object (including this reference.)
         */
        RX_INLINE_VISIBILITY const Int32 getSharedReferenceCount() const {
            return SharedReferenceCount.getSharedReferenceCount();
        }
        
        /**
          *Returns true if this is the only shared reference to this object.  Note that there may be
          *outstanding weak references left.
          *IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
         *
          *@return  True if there is only one shared reference to the object, and this is it!
         */
        RX_INLINE_VISIBILITY const bool isUnique() const {
            return SharedReferenceCount.isUnique();
        }
        
    private:
        template <typename OtherType>
        void Init(OtherType *InObject) {
            // If the following assert goes off, it means a SharedRef was initialized from a nullptr object pointer.
            // Shared references must never be nullptr, so either pass a valid object or consider using SharedPtr instead.
            RxCheck(InObject != nullptr);
            
            // If the object happens to be derived from SharedFromThis, the following method
            // will prime the object with a weak pointer to itself.
            SharedPointerInternals::EnableSharedFromThis(this, InObject, InObject);
        }
        
        /**
          *Converts a shared pointer to a shared reference.  The pointer *must *be valid or an assertion will trigger.
          *NOTE: This explicit conversion constructor is intentionally private.  Use 'toSharedRef()' instead.
         *
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY explicit SharedRef(SharedPtr<OtherType, Mode> const &InSharedPtr) :
        Object(InSharedPtr.Object),
        SharedReferenceCount(InSharedPtr.SharedReferenceCount) {
            // If this assert goes off, it means a shared reference was created from a shared pointer that was nullptr.
            // Shared references are never allowed to be null.  Consider using SharedPtr instead.
            RxCheck(isValid());
        }
        
        template <typename OtherType>
        RX_INLINE_VISIBILITY explicit SharedRef(SharedPtr<OtherType, Mode> &&InSharedPtr) :
        Object(InSharedPtr.Object),
        SharedReferenceCount(std::move(InSharedPtr.SharedReferenceCount)) {
            InSharedPtr.Object = nullptr;
            
            // If this assert goes off, it means a shared reference was created from a shared pointer that was nullptr.
            // Shared references are never allowed to be null.  Consider using SharedPtr instead.
            RxCheck(isValid());
        }
        
        /**
          *Checks to see if this shared reference is actually pointing to an object.
          *NOTE: This validity test is intentionally private because shared references must always be valid.
         *
          *@return  True if the shared reference is valid and can be dereferenced
         */
        RX_INLINE_VISIBILITY const bool isValid() const {
            return Object != nullptr;
        }
        
        /**
         *Computes a hash code for this object
         *
         *@param  InSharedRef  Shared pointer to compute hash code for
         *
         *@return  Hash code value
         */
        friend UInt32 getTypeHash(const SharedRef<ObjectType, Mode>& InSharedRef) {
            return Hash(InSharedRef.Object);
        }
        
        // We declare ourselves as a friend (templated using OtherType) so we can access members as needed
        template <typename OtherType, ESPMode OtherMode> friend class SharedRef;
        
        // Declare other smart pointer types as friends as needed
        template <typename OtherType, ESPMode OtherMode> friend class SharedPtr;
        template <typename OtherType, ESPMode OtherMode> friend class WeakPtr;
        
    private:
        
        /* *The object we're holding a reference to.  Can be nullptr. */
        ObjectType *Object;
        
        /* *Interface to the reference counter for this object.  Note that the actual reference
         controller object is shared by all shared and weak pointers that refer to the object */
        SharedPointerInternals::SharedReferencer<Mode> SharedReferenceCount;
    };
    
    
    /**
      *Wrapper for a type that yields a reference to that type.
     */
    template <typename T>
    struct FMakeReferenceTo {
        typedef T& Type;
    };
    
    
    /**
      *Specialization for FMakeReferenceTo<void>.
     */
    template<>
    struct FMakeReferenceTo<void> {
        typedef void Type;
    };
    
    
    /**
      *SharedPtr is a non-intrusive reference-counted authoritative object pointer.  This shared pointer
      *will be conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
     */
    template <typename ObjectType, ESPMode Mode>
    class SharedPtr {
        // TSharedPtrs with UObjects are illegal.
        //	static_assert(!TPointerIsConvertibleFromTo<ObjectType, const UObjectBase>::Value, "You cannot use SharedPtr or WeakPtr with UObjects. Consider a UPROPERTY() pointer or TWeakObjectPtr.");
        
        enum {
            ObjectTypeHasSameModeSharedFromThis     = TPointerIsConvertibleFromTo<ObjectType, SharedFromThis<ObjectType, Mode>>::Value,
            ObjectTypeHasOppositeModeSharedFromThis = TPointerIsConvertibleFromTo<ObjectType, SharedFromThis<ObjectType, (Mode == ESPMode::NotThreadSafe) ? ESPMode::ThreadSafe : ESPMode::NotThreadSafe>>::Value
        };
        
        // SharedPtr of one mode to a type which has a SharedFromThis only of another mode is illegal.
        // A type which does not inherit SharedFromThis at all is ok.
        static_assert(SharedPtr::ObjectTypeHasSameModeSharedFromThis || !SharedPtr::ObjectTypeHasOppositeModeSharedFromThis, "You cannot use a SharedPtr of one mode with a type which inherits SharedFromThis of another mode.");
        
    public:
        using value_type = ObjectType;
        /**
          *Constructs an empty shared pointer
         */
        // NOTE: NullTag parameter is an Unreal extension to standard shared_ptr behavior
        RX_INLINE_VISIBILITY SharedPtr(SharedPointerInternals::NullTag *_ = nullptr) :
        Object(nullptr),
        SharedReferenceCount() {
        }
        
        /**
          *Constructs a shared pointer that owns the specified object.  Note that passing nullptr here will
          *still create a tracked reference to a nullptr pointer. (Consistent with std::shared_ptr)
         *
          *@param  InObject  Object this shared pointer to retain a reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY explicit SharedPtr(OtherType *InObject) :
        Object(InObject),
        SharedReferenceCount(SharedPointerInternals::NewDefaultReferenceController(InObject)) {
            // If the object happens to be derived from SharedFromThis, the following method
            // will prime the object with a weak pointer to itself.
            SharedPointerInternals::EnableSharedFromThis(this, InObject, InObject);
        }
        
        /**
          *Constructs a shared pointer that owns the specified object.  Note that passing nullptr here will
          *still create a tracked reference to a nullptr pointer. (Consistent with std::shared_ptr)
         *
          *@param  InObject   Object this shared pointer to retain a reference to
          *@param  InDeleter  Deleter object used to destroy the object when it is no longer referenced.
         */
        template <typename OtherType, class DeleterType>
        RX_INLINE_VISIBILITY SharedPtr(OtherType *InObject, DeleterType &&InDeleter) :
        Object(InObject),
        SharedReferenceCount(SharedPointerInternals::NewCustomReferenceController(InObject, std::forward<DeleterType>(InDeleter))) {
            // If the object happens to be derived from SharedFromThis, the following method
            // will prime the object with a weak pointer to itself.
            SharedPointerInternals::EnableSharedFromThis(this, InObject, InObject);
        }
        
        /**
          *Constructs a shared pointer using a proxy reference to a raw pointer. (See MakeShareable())
         *
          *@param  InRawPtrProxy  Proxy raw pointer that contains the object that the new shared pointer will reference
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr(SharedPointerInternals::RawPtrProxy<OtherType> const &InRawPtrProxy) :
        Object(InRawPtrProxy.Object),
        SharedReferenceCount(InRawPtrProxy.ReferenceController) {
            // If the object happens to be derived from SharedFromThis, the following method
            // will prime the object with a weak pointer to itself.
            SharedPointerInternals::EnableSharedFromThis(this, InRawPtrProxy.Object, InRawPtrProxy.Object);
        }
        
        /**
          *Constructs a shared pointer as a shared reference to an existing shared pointer's object.
          *This constructor is needed so that we can implicitly upcast to base classes.
         *
          *@param  InSharedPtr  The shared pointer whose object we should create an additional reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr(SharedPtr<OtherType, Mode> const &InSharedPtr) :
        Object(InSharedPtr.Object),
        SharedReferenceCount(InSharedPtr.SharedReferenceCount) {
        }
        
        RX_INLINE_VISIBILITY SharedPtr(SharedPtr const &InSharedPtr) :
        Object(InSharedPtr.Object),
        SharedReferenceCount(InSharedPtr.SharedReferenceCount) {
        }
        
        RX_INLINE_VISIBILITY SharedPtr(SharedPtr &&InSharedPtr) :
        Object(InSharedPtr.Object),
        SharedReferenceCount(std::move(InSharedPtr.SharedReferenceCount)) {
            InSharedPtr.Object = nullptr;
        }
        
        /**
          *Implicitly converts a shared reference to a shared pointer, adding a reference to the object.
          *NOTE: We allow an implicit conversion from SharedRef to SharedPtr because it's always a safe conversion.
         *
          *@param  InSharedRef  The shared reference that will be converted to a shared pointer
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr(SharedRef<OtherType, Mode> const &InSharedRef) :
        Object(InSharedRef.Object),
        SharedReferenceCount(InSharedRef.SharedReferenceCount) {
            // There is no rvalue overload of this constructor, because 'stealing' the pointer from a
            // SharedRef would leave it as null, which would invalidate its invariant.
        }
        
        /**
          *Special constructor used internally to statically cast one shared pointer type to another.  You
          *should never call this constructor directly.  Instead, use the StaticCastSharedPtr() function.
          *This constructor creates a shared pointer as a shared reference to an existing shared pointer after
          *statically casting that pointer's object.  This constructor is needed for static casts.
         *
          *@param  InSharedPtr  The shared pointer whose object we should create an additional reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr(SharedPtr<OtherType, Mode> const &InSharedPtr, SharedPointerInternals::StaticCastTag) :
        Object(static_cast<ObjectType*>(InSharedPtr.Object)),
        SharedReferenceCount(InSharedPtr.SharedReferenceCount) {
        }
        
        /**
          *Special constructor used internally to cast a 'const' shared pointer a 'mutable' pointer.  You
          *should never call this constructor directly.  Instead, use the ConstCastSharedPtr() function.
          *This constructor creates a shared pointer as a shared reference to an existing shared pointer after
          *const casting that pointer's object.  This constructor is needed for const casts.
         *
          *@param  InSharedPtr  The shared pointer whose object we should create an additional reference to
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr(SharedPtr<OtherType, Mode> const &InSharedPtr, SharedPointerInternals::ConstCastTag) :
        Object(const_cast<ObjectType*>(InSharedPtr.Object)),
        SharedReferenceCount(InSharedPtr.SharedReferenceCount) {
        }
        
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr(SharedPtr<OtherType, Mode> const &InSharedPtr, SharedPointerInternals::DynamicCastTag) :
        Object(dynamic_cast<ObjectType*>(InSharedPtr.Object)),
        SharedReferenceCount(InSharedPtr.SharedReferenceCount) {
        }
        
        
        /**
          *Special constructor used internally to create a shared pointer from an existing shared pointer,
          *while using the specified object pointer instead of the incoming shared pointer's object
          *pointer.  This is used by with the SharedFromThis feature (by UpdateWeakReferenceInternal)
         *
          *@param  OtherSharedPtr  The shared pointer whose reference count
          *@param  InObject  The object pointer to use (instead of the incoming shared pointer's object)
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr(SharedPtr<OtherType, Mode> const &OtherSharedPtr, ObjectType *InObject) :
        Object(InObject),
        SharedReferenceCount(OtherSharedPtr.SharedReferenceCount) {
        }
        
        /**
          *Assignment to a nullptr pointer.  The object currently referenced by this shared pointer will no longer be
          *referenced and will be deleted if there are no other referencers.
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        RX_INLINE_VISIBILITY SharedPtr &operator=(SharedPointerInternals::NullTag*) {
            reset();
            return *this;
        }
        
        /**
          *Assignment operator replaces this shared pointer with the specified shared pointer.  The object
          *currently referenced by this shared pointer will no longer be referenced and will be deleted if
          *there are no other referencers.
         *
          *@param  InSharedPtr  Shared pointer to replace with
         */
        RX_INLINE_VISIBILITY SharedPtr &operator=(SharedPtr const &InSharedPtr) {
            SharedReferenceCount = InSharedPtr.SharedReferenceCount;
            Object = InSharedPtr.Object;
            return *this;
        }
        
        RX_INLINE_VISIBILITY SharedPtr &operator=(SharedPtr &&InSharedPtr) {
            if (this != &InSharedPtr) {
                Object = InSharedPtr.Object;
                InSharedPtr.Object = nullptr;
                SharedReferenceCount = std::move(InSharedPtr.SharedReferenceCount);
            }
            return *this;
        }
        
        /**
          *Assignment operator replaces this shared pointer with the specified shared pointer.  The object
          *currently referenced by this shared pointer will no longer be referenced and will be deleted if
          *there are no other referencers.
         *
          *@param  InRawPtrProxy  Proxy object used to assign the object (see MakeShareable helper function)
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename OtherType>
        RX_INLINE_VISIBILITY SharedPtr &operator=(SharedPointerInternals::RawPtrProxy<OtherType> const &InRawPtrProxy) {
            *this = SharedPtr<ObjectType, Mode>(InRawPtrProxy);
            return *this;
        }
        
        /**
          *Converts a shared pointer to a shared reference.  The pointer *must *be valid or an assertion will trigger.
         *
          *@return  Reference to the object
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        RX_INLINE_VISIBILITY SharedRef<ObjectType, Mode> toSharedRef() const {
            // If this assert goes off, it means a shared reference was created from a shared pointer that was nullptr.
            // Shared references are never allowed to be null.  Consider using SharedPtr instead.
            RxCheck(isValid());
            return SharedRef<ObjectType, Mode>(*this);
        }
        
        /**
          *Returns the object referenced by this pointer, or nullptr if no object is reference
         *
          *@return  The object owned by this shared pointer, or nullptr
         */
        RX_INLINE_VISIBILITY ObjectType *get() const {
            return Object;
        }
        
        /**
          *Checks to see if this shared pointer is actually pointing to an object
         *
          *@return  True if the shared pointer is valid and can be dereferenced
         */
        RX_INLINE_VISIBILITY const bool isValid() const {
            return Object != nullptr;
        }
        
        RX_INLINE_VISIBILITY operator bool() const {
            return isValid();
        }
        
        /**
          *Dereference operator returns a reference to the object this shared pointer points to
         *
          *@return  Reference to the object
         */
        RX_INLINE_VISIBILITY typename FMakeReferenceTo<ObjectType>::Type operator*() const {
            RxCheck(isValid());
            return *Object;
        }
        
        /**
          *Arrow operator returns a pointer to the object this shared pointer references
         *
          *@return  Returns a pointer to the object referenced by this shared pointer
         */
        RX_INLINE_VISIBILITY ObjectType *operator->() const {
            RxCheck(isValid());
            return Object;
        }
        
        RX_INLINE_VISIBILITY bool operator<(const SharedPtr<ObjectType> &rhs) const {
            return Object < rhs.Object;
        }
        /**
          *resets this shared pointer, removing a reference to the object.  If there are no other shared
          *references to the object then it will be destroyed.
         */
        RX_INLINE_VISIBILITY void reset() {
            *this = SharedPtr<ObjectType, Mode>();
        }
        
        /**
          *Returns the number of shared references to this object (including this reference.)
          *IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
         *
          *@return  Number of shared references to the object (including this reference.)
         */
        RX_INLINE_VISIBILITY const Int32 getSharedReferenceCount() const {
            return SharedReferenceCount.getSharedReferenceCount();
        }
        
        /**
          *Returns true if this is the only shared reference to this object.  Note that there may be
          *outstanding weak references left.
          *IMPORTANT: Not necessarily fast!  Should only be used for debugging purposes!
         *
          *@return  True if there is only one shared reference to the object, and this is it!
         */
        RX_INLINE_VISIBILITY const bool isUnique() const {
            return SharedReferenceCount.isUnique();
        }
        
    private:
        
        /**
          *Constructs a shared pointer from a weak pointer, allowing you to access the object (if it
          *hasn't expired yet.)  Remember, if there are no more shared references to the object, the
          *shared pointer will not be valid.  You should always RxCheck to make sure this shared
          *pointer is valid before trying to dereference the shared pointer!
         *
          *NOTE: This constructor is private to force users to be explicit when converting a weak
          *      pointer to a shared pointer.  Use the weak pointer's pin() method instead!
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY explicit SharedPtr(WeakPtr<OtherType, Mode> const &InWeakPtr) :
        Object(nullptr),
        SharedReferenceCount(InWeakPtr.WeakReferenceCount) {
            // Check that the shared reference was created from the weak reference successfully.  We'll only
            // cache a pointer to the object if we have a valid shared reference.
            if(SharedReferenceCount.isValid()) {
                Object = InWeakPtr.Object;
            }
        }
        
        /**
          *Computes a hash code for this object
         *
          *@param  InSharedPtr  Shared pointer to compute hash code for
         *
          *@return  Hash code value
         */
        friend UInt32 getTypeHash(const SharedPtr<ObjectType, Mode>& InSharedPtr) {
            return Hash(InSharedPtr.Object);
        }
        
        // We declare ourselves as a friend (templated using OtherType) so we can access members as needed
        template <typename OtherType, ESPMode OtherMode> friend class SharedPtr;
        
        // Declare other smart pointer types as friends as needed
        template <typename OtherType, ESPMode OtherMode> friend class SharedRef;
        template <typename OtherType, ESPMode OtherMode> friend class WeakPtr;
        template <typename OtherType, ESPMode OtherMode> friend class SharedFromThis;
        
    private:
        
        /* *The object we're holding a reference to.  Can be nullptr. */
        ObjectType *Object;
        
        /**Interface to the reference counter for this object.  Note that the actual reference
         controller object is shared by all shared and weak pointers that refer to the object */
        SharedPointerInternals::SharedReferencer<Mode> SharedReferenceCount;
        
    };
    
    template <typename T>
    struct TIsZeroConstructType {
        enum { Value = false };
    };
    
    template <typename ObjectType, ESPMode Mode> struct TIsZeroConstructType<SharedPtr<ObjectType, Mode>> { enum { Value = true }; };
    
    
    /**
      *WeakPtr is a non-intrusive reference-counted weak object pointer.  This weak pointer will be
      *conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
     */
    template <typename ObjectType, ESPMode Mode>
    class WeakPtr {
    public:
        
        /* *Constructs an empty WeakPtr */
        // NOTE: NullTag parameter is an Unreal extension to standard shared_ptr behavior
        RX_INLINE_VISIBILITY WeakPtr(SharedPointerInternals::NullTag *_ = nullptr) :
        Object(nullptr),
        WeakReferenceCount() {
        }
        
        /**
          *Constructs a weak pointer from a shared reference
         *
          *@param  InSharedRef  The shared reference to create a weak pointer from
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr(SharedRef<OtherType, Mode> const &InSharedRef) : Object(InSharedRef.Object), WeakReferenceCount(InSharedRef.SharedReferenceCount) {
        }
        
        /**
          *Constructs a weak pointer from a shared pointer
         *
          *@param  InSharedPtr  The shared pointer to create a weak pointer from
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr(SharedPtr<OtherType, Mode> const &InSharedPtr) : Object(InSharedPtr.Object), WeakReferenceCount(InSharedPtr.SharedReferenceCount) {
        }
        
        /**
          *Constructs a weak pointer from a weak pointer of another type.
          *This constructor is intended to allow derived-to-base conversions.
         *
          *@param  InWeakPtr  The weak pointer to create a weak pointer from
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr(WeakPtr<OtherType, Mode> const &InWeakPtr) : Object(InWeakPtr.Object), WeakReferenceCount(InWeakPtr.WeakReferenceCount) {
        }
        
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr(WeakPtr<OtherType, Mode> &&InWeakPtr) : Object(InWeakPtr.Object), WeakReferenceCount(std::move(InWeakPtr.WeakReferenceCount)) {
            InWeakPtr.Object = nullptr;
        }
        
        RX_INLINE_VISIBILITY WeakPtr(WeakPtr const &InWeakPtr) : Object(InWeakPtr.Object), WeakReferenceCount(InWeakPtr.WeakReferenceCount) {
        }
        
        RX_INLINE_VISIBILITY WeakPtr(WeakPtr &&InWeakPtr) : Object(InWeakPtr.Object), WeakReferenceCount(std::move(InWeakPtr.WeakReferenceCount)) {
            InWeakPtr.Object = nullptr;
        }
        
        /**
          *Assignment to a nullptr pointer.  Clears this weak pointer's reference.
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        RX_INLINE_VISIBILITY WeakPtr &operator=(SharedPointerInternals::NullTag*) {
            reset();
            return *this;
        }
        
        /**
          *Assignment operator adds a weak reference to the object referenced by the specified weak pointer
         *
          *@param  InWeakPtr  The weak pointer for the object to assign
         */
        RX_INLINE_VISIBILITY WeakPtr &operator=(WeakPtr const &InWeakPtr) {
            Object = InWeakPtr.pin().get();
            WeakReferenceCount = InWeakPtr.WeakReferenceCount;
            return *this;
        }
        
        RX_INLINE_VISIBILITY WeakPtr &operator=(WeakPtr &&InWeakPtr) {
            if (this != &InWeakPtr) {
                Object             = InWeakPtr.Object;
                InWeakPtr.Object   = nullptr;
                WeakReferenceCount = std::move(InWeakPtr.WeakReferenceCount);
            }
            return *this;
        }
        
        /**
          *Assignment operator adds a weak reference to the object referenced by the specified weak pointer.
          *This assignment operator is intended to allow derived-to-base conversions.
         *
          *@param  InWeakPtr  The weak pointer for the object to assign
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr &operator=(WeakPtr<OtherType, Mode> const &InWeakPtr) {
            Object = InWeakPtr.pin().get();
            WeakReferenceCount = InWeakPtr.WeakReferenceCount;
            return *this;
        }
        
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr &operator=(WeakPtr<OtherType, Mode> &&InWeakPtr) {
            Object             = InWeakPtr.Object;
            InWeakPtr.Object   = nullptr;
            WeakReferenceCount = std::move(InWeakPtr.WeakReferenceCount);
            return *this;
        }
        
        /**
          *Assignment operator sets this weak pointer from a shared reference
         *
          *@param  InSharedRef  The shared reference used to assign to this weak pointer
         */
        // NOTE: The following is an Unreal extension to standard shared_ptr behavior
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr &operator=(SharedRef<OtherType, Mode> const &InSharedRef) {
            Object = InSharedRef.Object;
            WeakReferenceCount = InSharedRef.SharedReferenceCount;
            return *this;
        }
        
        /**
          *Assignment operator sets this weak pointer from a shared pointer
         *
          *@param  InSharedPtr  The shared pointer used to assign to this weak pointer
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY WeakPtr &operator=(SharedPtr<OtherType, Mode> const &InSharedPtr) {
            Object = InSharedPtr.Object;
            WeakReferenceCount = InSharedPtr.SharedReferenceCount;
            return *this;
        }
        
        /**
          *Converts this weak pointer to a shared pointer that you can use to access the object (if it
          *hasn't expired yet.)  Remember, if there are no more shared references to the object, the
          *returned shared pointer will not be valid.  You should always RxCheck to make sure the returned
          *pointer is valid before trying to dereference the shared pointer!
         *
          *@return  Shared pointer for this object (will only be valid if still referenced!)
         */
        RX_INLINE_VISIBILITY SharedPtr<ObjectType, Mode> pin() const {
            return SharedPtr<ObjectType, Mode>(*this);
        }
        
        /**
          *Checks to see if this weak pointer actually has a valid reference to an object
         *
          *@return  True if the weak pointer is valid and a pin operator would have succeeded
         */
        RX_INLINE_VISIBILITY const bool isValid() const {
            return Object != nullptr  &&WeakReferenceCount.isValid();
        }
        
        RX_INLINE_VISIBILITY bool operator<(const WeakPtr<ObjectType> &rhs) const {
            return Object < rhs.Object;
        }
        
        /**
          *resets this weak pointer, removing a weak reference to the object.  If there are no other shared
          *or weak references to the object, then the tracking object will be destroyed.
         */
        RX_INLINE_VISIBILITY void reset() {
            *this = WeakPtr<ObjectType, Mode>();
        }
        
        /**
          *Returns true if the object this weak pointer points to is the same as the specified object pointer.
         */
        RX_INLINE_VISIBILITY bool HasSameObject(const void *InOtherPtr) const {
            return pin().get() == InOtherPtr;
        }
        
    private:
        
        /**
          *Computes a hash code for this object
         *
          *@param  InWeakPtr  Weak pointer to compute hash code for
         *
          *@return  Hash code value
         */
        friend UInt32 getTypeHash(const WeakPtr<ObjectType, Mode>& InWeakPtr) {
            return Hash(InWeakPtr.Object);
        }
        
        // We declare ourselves as a friend (templated using OtherType) so we can access members as needed
        template <typename OtherType, ESPMode OtherMode> friend class WeakPtr;
        
        // Declare ourselves as a friend of SharedPtr so we can access members as needed
        template <typename OtherType, ESPMode OtherMode> friend class SharedPtr;
        
    private:
        
        /* *The object we have a weak reference to.  Can be nullptr.  Also, it's important to note that because
         this is a weak reference, the object this pointer points to may have already been destroyed. */
        ObjectType *Object;
        
        /**Interface to the reference counter for this object.  Note that the actual reference
         controller object is shared by all shared and weak pointers that refer to the object */
        SharedPointerInternals::WeakReferencer<Mode> WeakReferenceCount;
    };
    
    template<typename T>
    struct TIsWeakPointerType {
        enum { Value = false };
    };
    
    
    template <typename T, ESPMode Mode> struct TIsWeakPointerType<WeakPtr<T, Mode>> { enum { Value = true }; };
    template <typename T, ESPMode Mode> struct TIsZeroConstructType<WeakPtr<T, Mode>> { enum { Value = true }; };
    
    
    /**
      *Derive your class from SharedFromThis to enable access to a SharedRef directly from an object
      *instance that's already been allocated.  Use the optional Mode template argument for thread-safety.
     */
    template <typename ObjectType, ESPMode Mode>
    class SharedFromThis {
    public:
        
        /**
          *Provides access to a shared reference to this object.  Note that is only valid to call
          *this after a shared reference (or shared pointer) to the object has already been created.
          *Also note that it is illegal to call this in the object's destructor.
         *
          *@return	Returns this object as a shared pointer
         */
        SharedRef<ObjectType, Mode> asShared() {
            SharedPtr<ObjectType, Mode> SharedThis(WeakThis.pin());
            
            //
            // If the following assert goes off, it means one of the following:
            //
            //     - You tried to request a shared pointer before the object was ever assigned to one. (e.g. constructor)
            //     - You tried to request a shared pointer while the object is being destroyed (destructor chain)
            //
            // To fix this, make sure you create at least one shared reference to your object instance before requested,
            // and also avoid calling this function from your object's destructor.
            //
            RxCheck(SharedThis.get() == this);
            
            // Now that we've verified the shared pointer is valid, we'll convert it to a shared reference
            // and return it!
            return SharedThis.toSharedRef();
        }
        
        /**
          *Provides access to a shared reference to this object (const.)  Note that is only valid to call
          *this after a shared reference (or shared pointer) to the object has already been created.
          *Also note that it is illegal to call this in the object's destructor.
         *
          *@return	Returns this object as a shared pointer (const)
         */
        SharedRef<ObjectType const, Mode> asShared() const {
            SharedPtr<ObjectType const, Mode> SharedThis(WeakThis);
            
            //
            // If the following assert goes off, it means one of the following:
            //
            //     - You tried to request a shared pointer before the object was ever assigned to one. (e.g. constructor)
            //     - You tried to request a shared pointer while the object is being destroyed (destructor chain)
            //
            // To fix this, make sure you create at least one shared reference to your object instance before requested,
            // and also avoid calling this function from your object's destructor.
            //
            RxCheck(SharedThis.get() == this);
            
            // Now that we've verified the shared pointer is valid, we'll convert it to a shared reference
            // and return it!
            return SharedThis.toSharedRef();
        }
        
    protected:
        
        /**
          *Provides access to a shared reference to an object, given the object's 'this' pointer.  Uses
          *the 'this' pointer to derive the object's actual type, then casts and returns an appropriately
          *typed shared reference.  Intentionally declared 'protected', as should only be called when the
          *'this' pointer can be passed.
         *
          *@return	Returns this object as a shared pointer
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY static SharedRef<OtherType, Mode> SharedThis(OtherType *ThisPtr) {
            return StaticCastSharedRef<OtherType>(ThisPtr->asShared());
        }
        
        /**
          *Provides access to a shared reference to an object, given the object's 'this' pointer. Uses
          *the 'this' pointer to derive the object's actual type, then casts and returns an appropriately
          *typed shared reference.  Intentionally declared 'protected', as should only be called when the
          *'this' pointer can be passed.
         *
          *@return	Returns this object as a shared pointer (const)
         */
        template <typename OtherType>
        RX_INLINE_VISIBILITY static SharedRef<OtherType const, Mode> SharedThis(const OtherType *ThisPtr) {
            return StaticCastSharedRef<OtherType const>(ThisPtr->asShared());
        }
        
    public:		// @todo: Ideally this would be private, but template sharing problems prevent it
        
        /**
          *INTERNAL USE ONLY -- Do not call this method.  Freshens the internal weak pointer object using
          *the supplied object pointer along with the authoritative shared reference to the object.
          *Note that until this function is called, calls to asShared() will result in an empty pointer.
         */
        template <typename SharedPtrType, class OtherType>
        RX_INLINE_VISIBILITY void UpdateWeakReferenceInternal(SharedPtr<SharedPtrType, Mode> const *InSharedPtr, OtherType *InObject) const {
            if(!WeakThis.isValid()) {
                WeakThis = SharedPtr<ObjectType, Mode>(*InSharedPtr, InObject);
            }
        }
        
        /**
          *INTERNAL USE ONLY -- Do not call this method.  Freshens the internal weak pointer object using
          *the supplied object pointer along with the authoritative shared reference to the object.
          *Note that until this function is called, calls to asShared() will result in an empty pointer.
         */
        template <typename SharedRefType, class OtherType>
        RX_INLINE_VISIBILITY void UpdateWeakReferenceInternal(SharedRef<SharedRefType, Mode> const *InSharedRef, OtherType *InObject) const {
            if(!WeakThis.isValid()) {
                WeakThis = SharedRef<ObjectType, Mode>(*InSharedRef, InObject);
            }
        }
        
        /**
          *Checks whether given instance has been already made sharable (use in checks to detect when it
          *happened, since it's a straight way to crashing
         */
        RX_INLINE_VISIBILITY bool HasBeenAlreadyMadeSharable() const {
            return WeakThis.isValid();
        }
        
    protected:
        
        /* *Hidden stub constructor */
        SharedFromThis() {
        }
        
        /* *Hidden stub copy constructor */
        SharedFromThis(SharedFromThis const&) {
        }
        
        /* *Hidden stub assignment operator */
        RX_INLINE_VISIBILITY SharedFromThis& operator=(SharedFromThis const&) {
            return *this;
        }
        
        /* *Hidden destructor */
        ~SharedFromThis() {
        }
        
    private:
        
        /* *Weak reference to ourselves.  If we're destroyed then this weak pointer reference will be destructed
         with ourselves.  Note this is declared mutable only so that UpdateWeakReferenceInternal() can update it. */
        mutable WeakPtr<ObjectType, Mode> WeakThis;
    };
    
    
    /**
      *Global equality operator for SharedRef
     *
      *@return  True if the two shared references are equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(SharedRef<ObjectTypeA, Mode> const &InSharedRefA, SharedRef<ObjectTypeB, Mode> const &InSharedRefB) {
        return &(InSharedRefA.get()) == &(InSharedRefB.get());
    }
    
    
    /**
      *Global inequality operator for SharedRef
     *
      *@return  True if the two shared references are not equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(SharedRef<ObjectTypeA, Mode> const &InSharedRefA, SharedRef<ObjectTypeB, Mode> const &InSharedRefB) {
        return &(InSharedRefA.get()) != &(InSharedRefB.get());
    }
    
    
    /**
      *Global equality operator for SharedPtr
     *
      *@return  True if the two shared pointers are equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(SharedPtr<ObjectTypeA, Mode> const &InSharedPtrA, SharedPtr<ObjectTypeB, Mode> const &InSharedPtrB) {
        return InSharedPtrA.get() == InSharedPtrB.get();
    }
    
    
    /**
      *Global inequality operator for SharedPtr
     *
      *@return  True if the two shared pointers are not equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(SharedPtr<ObjectTypeA, Mode> const &InSharedPtrA, SharedPtr<ObjectTypeB, Mode> const &InSharedPtrB) {
        return InSharedPtrA.get() != InSharedPtrB.get();
    }
    
    
    /**
      *Tests to see if a SharedRef is "equal" to a SharedPtr (both are valid and refer to the same object)
     *
      *@return  True if the shared reference and shared pointer are "equal"
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(SharedRef<ObjectTypeA, Mode> const &InSharedRef, SharedPtr<ObjectTypeB, Mode> const &InSharedPtr) {
        return InSharedPtr.isValid()  &&InSharedPtr.get() == &(InSharedRef.get());
    }
    
    
    /**
      *Tests to see if a SharedRef is not "equal" to a SharedPtr (shared pointer is invalid, or both refer to different objects)
     *
      *@return  True if the shared reference and shared pointer are not "equal"
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(SharedRef<ObjectTypeA, Mode> const &InSharedRef, SharedPtr<ObjectTypeB, Mode> const &InSharedPtr) {
        return !InSharedPtr.isValid() || (InSharedPtr.get() != &(InSharedRef.get()));
    }
    
    
    /**
      *Tests to see if a SharedRef is "equal" to a SharedPtr (both are valid and refer to the same object) (reverse)
     *
      *@return  True if the shared reference and shared pointer are "equal"
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(SharedPtr<ObjectTypeB, Mode> const &InSharedPtr, SharedRef<ObjectTypeA, Mode> const &InSharedRef) {
        return InSharedRef == InSharedPtr;
    }
    
    
    /**
      *Tests to see if a SharedRef is not "equal" to a SharedPtr (shared pointer is invalid, or both refer to different objects) (reverse)
     *
      *@return  True if the shared reference and shared pointer are not "equal"
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(SharedPtr<ObjectTypeB, Mode> const &InSharedPtr, SharedRef<ObjectTypeA, Mode> const &InSharedRef) {
        return InSharedRef != InSharedPtr;
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the two weak pointers are equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return InWeakPtrA.pin().get() == InWeakPtrB.pin().get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ref are equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, SharedRef<ObjectTypeB, Mode> const &InSharedRefB) {
        return InWeakPtrA.pin().get() == &InSharedRefB.get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ptr are equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, SharedPtr<ObjectTypeB, Mode> const &InSharedPtrB) {
        return InWeakPtrA.pin().get() == InSharedPtrB.get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ref are equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(SharedRef<ObjectTypeA, Mode> const &InSharedRefA, WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return &InSharedRefA.get() == InWeakPtrB.pin().get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ptr are equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(SharedPtr<ObjectTypeA, Mode> const &InSharedPtrA, WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return InSharedPtrA.get() == InWeakPtrB.pin().get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer is null
     */
    template <typename ObjectTypeA, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, decltype(nullptr)) {
        return !InWeakPtrA.isValid();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer is null
     */
    template <typename ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator==(decltype(nullptr), WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return !InWeakPtrB.isValid();
    }
    
    
    /**
      *Global inequality operator for WeakPtr
     *
      *@return  True if the two weak pointers are not equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return InWeakPtrA.pin().get() != InWeakPtrB.pin().get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ref are not equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, SharedRef<ObjectTypeB, Mode> const &InSharedRefB) {
        return InWeakPtrA.pin().get() != &InSharedRefB.get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ptr are not equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, SharedPtr<ObjectTypeB, Mode> const &InSharedPtrB) {
        return InWeakPtrA.pin().get() != InSharedPtrB.get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ref are not equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(SharedRef<ObjectTypeA, Mode> const &InSharedRefA, WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return &InSharedRefA.get() != InWeakPtrB.pin().get();
    }
    
    
    /**
      *Global equality operator for WeakPtr
     *
      *@return  True if the weak pointer and the shared ptr are not equal
     */
    template <typename ObjectTypeA, class ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(SharedPtr<ObjectTypeA, Mode> const &InSharedPtrA, WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return InSharedPtrA.get() != InWeakPtrB.pin().get();
    }
    
    
    /**
      *Global inequality operator for WeakPtr
     *
      *@return  True if the weak pointer is not null
     */
    template <typename ObjectTypeA, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(WeakPtr<ObjectTypeA, Mode> const &InWeakPtrA, decltype(nullptr)) {
        return InWeakPtrA.isValid();
    }
    
    
    /**
      *Global inequality operator for WeakPtr
     *
      *@return  True if the weak pointer is not null
     */
    template <typename ObjectTypeB, ESPMode Mode>
    RX_INLINE_VISIBILITY bool operator!=(decltype(nullptr), WeakPtr<ObjectTypeB, Mode> const &InWeakPtrB) {
        return InWeakPtrB.isValid();
    }
    
    
    /**
      *Casts a shared pointer of one type to another type. (static_cast)  Useful for down-casting.
     *
      *@param  InSharedPtr  The shared pointer to cast
     */
    template <typename CastToType, class CastFromType, ESPMode Mode>
    RX_INLINE_VISIBILITY SharedPtr<CastToType, Mode> StaticCastSharedPtr(SharedPtr<CastFromType, Mode> const &InSharedPtr) {
        return SharedPtr<CastToType, Mode>(InSharedPtr, SharedPointerInternals::StaticCastTag());
    }
    
    template <typename CastToType, class CastFromType, ESPMode Mode>
    RX_INLINE_VISIBILITY SharedPtr<CastToType, Mode> DynamicCastSharedPtr(SharedPtr<CastFromType, Mode> const &InSharedPtr) {
        return SharedPtr<CastToType, Mode>(InSharedPtr, SharedPointerInternals::DynamicCastTag());
    }
    
    
    /**
      *Casts a 'const' shared reference to 'mutable' shared reference. (const_cast)
     *
      *@param  InSharedRef  The shared reference to cast
     */
    template <typename CastToType, class CastFromType, ESPMode Mode>
    RX_INLINE_VISIBILITY SharedRef<CastToType, Mode> ConstCastSharedRef(SharedRef<CastFromType, Mode> const &InSharedRef) {
        return SharedRef<CastToType, Mode>(InSharedRef, SharedPointerInternals::ConstCastTag());
    }
    
    
    /**
      *Casts a 'const' shared pointer to 'mutable' shared pointer. (const_cast)
     *
      *@param  InSharedPtr  The shared pointer to cast
     */
    template <typename CastToType, class CastFromType, ESPMode Mode>
    RX_INLINE_VISIBILITY SharedPtr<CastToType, Mode> ConstCastSharedPtr(SharedPtr<CastFromType, Mode> const &InSharedPtr) {
        return SharedPtr<CastToType, Mode>(InSharedPtr, SharedPointerInternals::ConstCastTag());
    }
    
    
    /**
      *MakeShareable utility function.  Wrap object pointers with MakeShareable to allow them to be implicitly
      *converted to shared pointers!  This is useful in assignment operations, or when returning a shared
      *pointer from a function.
     */
    // NOTE: The following is an Unreal extension to standard shared_ptr behavior
    template <typename ObjectType>
    RX_INLINE_VISIBILITY SharedPointerInternals::RawPtrProxy<ObjectType> MakeShareable(ObjectType *InObject) {
        return SharedPointerInternals::RawPtrProxy<ObjectType>(InObject);
    }
    
    
    /**
      *MakeShareable utility function.  Wrap object pointers with MakeShareable to allow them to be implicitly
      *converted to shared pointers!  This is useful in assignment operations, or when returning a shared
      *pointer from a function.
     */
    // NOTE: The following is an Unreal extension to standard shared_ptr behavior
    template <typename ObjectType, class DeleterType>
    RX_INLINE_VISIBILITY SharedPointerInternals::RawPtrProxy<ObjectType> MakeShareable(ObjectType *InObject, DeleterType &&InDeleter) {
        return SharedPointerInternals::RawPtrProxy<ObjectType>(InObject, std::forward<DeleterType>(InDeleter));
    }
    
    template <typename ObjectType,
    typename ...Args>
    RX_INLINE_VISIBILITY SharedPointerInternals::RawPtrProxy<ObjectType> MakeShareable(Args... args) {
        return SharedPointerInternals::RawPtrProxy<std::remove_reference_t<ObjectType>>(new ObjectType(std::forward<Args>(args)...));
    }
    
    ///**
    //  *Given a TArray of TWeakPtr's, will remove any invalid pointers.
    //  *@param  PointerArray  The pointer array to prune invalid pointers out of
    //*/
    //template <class Type>
    //RX_INLINE_VISIBILITY void CleanupPointerArray(TArray<WeakPtr<Type>>& PointerArray)
    //{
    //	TArray<WeakPtr<Type>> NewArray;
    //	for (Int32 i = 0; i <PointerArray.Num(); ++i)
    //	{
    //		if (PointerArray[i].isValid())
    //		{
    //			NewArray.Add(PointerArray[i]);
    //		}
    //	}
    //	PointerArray = NewArray;
    //}
    
    
    /**
      *Given a TMap of TWeakPtr's, will remove any invalid pointers. Not the most efficient.
      *@param  PointerMap  The pointer map to prune invalid pointers out of
     */
    //template <class KeyType, class ValueType>
    //RX_INLINE_VISIBILITY void CleanupPointerMap(TMap<WeakPtr<KeyType>, ValueType>& PointerMap)
    //{
    //	TMap<WeakPtr<KeyType>, ValueType> NewMap;
    //	for (typename TMap<WeakPtr<KeyType>, ValueType>::TConstIterator Op(PointerMap); Op; ++Op)
    //	{
    //		const WeakPtr<KeyType> WeakPointer = Op.Key();
    //		if (WeakPointer.isValid())
    //		{
    //			NewMap.Add(WeakPointer, Op.Value());
    //		}
    //	}
    //	PointerMap = NewMap;
    //}
}

// Shared pointer testing
//#include "SharedPointerTesting.inl"
