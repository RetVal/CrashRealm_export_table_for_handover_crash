//
//  KeyValueObserverInterface.hpp
//  RxFoundation
//
//  Created by closure on 3/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#ifndef KeyValueObserverInterface_hpp
#define KeyValueObserverInterface_hpp

#include <RxFoundation/RxBase.hpp>
#include <RxFoundation/String.hpp>
#include <RxFoundation/SharedPointer.hpp>
#include <RxFoundation/Any.hpp>
#include <RxFoundation/OrderedSet.hpp>
#include <RxFoundation/Exception.hpp>
#include <map>
#include <set>

namespace Rx {
    class KeyValueObserverInterface;
    using KeyValueObserverInterfaceParamRef = KeyValueObserverInterface *;
    using KeyValueObserverInterfaceRef = SharedRef<KeyValueObserverInterface>;
    using KeyValueObserverInterfacePtr = SharedPtr<KeyValueObserverInterface>;
    
    class KeyValueCodingInterface {
    public:
        static const String &ExceptionName;
        
        KeyValueCodingInterface() {};
        virtual ~KeyValueCodingInterface() {};
    public:
        virtual Any getValueForKey(const String &keyPath) const RX_NOEXCEPT_(false);
        virtual void setValueForKey(const String &, Any value) RX_NOEXCEPT_(false);
        
        virtual Any getValueForUndefinedKey(const String &keyPath) const RX_NOEXCEPT_(false);
        virtual void setValueForUndefinedKey(const String &keyPath, Any value) RX_NOEXCEPT_(false);
    private:
    };
    
    class KeyValueObserverInterface : public virtual KeyValueCodingInterface, public virtual SharedFromThis<KeyValueObserverInterface> {
    private:
        KeyValueObserverInterface(const KeyValueObserverInterface &) = delete;
        KeyValueObserverInterface(KeyValueObserverInterface &&) = delete;
    public:
        class Change : public NotCopyableInterface {
        public:
            enum Kind : Int32 {
                Invalid,
                New = 0x1,
                Old = 0x2,
                Initialize = 0x4,
                All = ~0
            };
            
            Change() RX_NOEXCEPT : _kind(Kind::Invalid), _oldValue(nullptr), _newValue(nullptr),
            _reserved(nullptr) {}
            
            Change(Kind kind, Any oldValue, Any newValue, Any reserved = nullptr) RX_NOEXCEPT :
            _kind(kind),
            _oldValue(oldValue),
            _newValue(newValue),
            _reserved(reserved) {
            }
            
            bool isValid() const RX_NOEXCEPT { return getKind() != Kind::Invalid; }
            Kind getKind() const RX_NOEXCEPT { return _kind; }
            const Any &getNewValue() const RX_NOEXCEPT { return _newValue; }
            const Any &getOldValue() const RX_NOEXCEPT { return _oldValue; }
            const Any &getParam() const RX_NOEXCEPT { return _reserved; }
            
            void reset() RX_NOEXCEPT {
                _kind = Kind::Invalid;
                _oldValue = nullptr;
                _newValue = nullptr;
            }
        private:
            friend class KeyValueObserverInterface;
            Any _oldValue;
            Any _newValue;
            Any _reserved;
            Kind _kind;
        };
        
        KeyValueObserverInterface() RX_NOEXCEPT;
        virtual ~KeyValueObserverInterface() RX_NOEXCEPT;
        
        void addObserver(KeyValueObserverInterfaceParamRef observer, const String &key, Integer notifyKind, Any context) RX_NOEXCEPT;
        void removeObserver(KeyValueObserverInterfaceParamRef observer, const String &key) RX_NOEXCEPT;
        
        void willChangeValueForKey(const String &key) const RX_NOEXCEPT;
        void didChangeValueForKey(const String &key) const RX_NOEXCEPT;
        
        void *getObserversInfo() const RX_NOEXCEPT { return (void *)&_observersInfo.get(); }
        
        virtual std::set<String> keyPathsForValuesAffectingValueForKey​​(const String &key) const RX_NOEXCEPT;
        virtual void receiveObserverNotify(const String &keyPath,
                                           KeyValueObserverInterfacePtr object,
                                           Any context,
                                           const Change &change) RX_NOEXCEPT {
        }
    private:
        
        void _willChangeValueForKey(const String &key, Change::Kind kind = Change::Kind::New) const RX_NOEXCEPT;
        void _didChangeValueForKey(const String &key, Change::Kind kind = Change::Kind::New, bool doCall = true) const RX_NOEXCEPT;
        
        enum class KVOCallState {
            Default,
            WillChange,
            Changing,
            DidChange,
        };
        
        class KVOPair : private std::pair<KeyValueObserverInterfaceParamRef, Any>, public SharedFromThis<KVOPair> {
        public:
            KVOPair(KeyValueObserverInterfaceParamRef observer,
                    Change::Kind notifyKind = Change::Kind::All,
                    Any context = nullptr) RX_NOEXCEPT :
            std::pair<KeyValueObserverInterfaceParamRef, Any>(observer, context),
            _notifyKind(notifyKind),
            _KVOCallState(KVOCallState::Default),
            _initialized(false),
            _enabled(false) {
            }
            ~KVOPair() RX_NOEXCEPT {};
            
            KVOPair(const KVOPair &value) RX_NOEXCEPT :
            std::pair<KeyValueObserverInterfaceParamRef, Any>(value.first, value.second),
            _notifyKind(value._notifyKind),
            _KVOCallState(value._KVOCallState),
            _initialized(value._initialized.load()),
            _enabled(value._enabled.load()) {
            }
            
            KVOPair(KVOPair &&value) RX_NOEXCEPT :
            std::pair<KeyValueObserverInterfaceParamRef, Any>(std::move(value.first),
                                                         std::move(value.second)),
            _notifyKind(value._notifyKind),
            _KVOCallState(value._KVOCallState),
            _initialized(value._initialized.load()),
            _enabled(value._enabled.load()) {
                value._notifyKind = Change::Kind::Invalid;
                value._initialized = false;
                value._enabled = false;
            }
        public:
            const KeyValueObserverInterfaceParamRef &getObserver() const RX_NOEXCEPT { return first; }
            const Any &getContext() const RX_NOEXCEPT { return second; }
            
            KeyValueObserverInterfaceParamRef &getObserver() RX_NOEXCEPT { return first; }
            Any &getContext() RX_NOEXCEPT { return second; }
            
            KVOCallState getKVOCallState() const RX_NOEXCEPT { return _KVOCallState; }
            KVOCallState getKVOCallState() RX_NOEXCEPT { return _KVOCallState; }
            void setKVOCallState(KVOCallState value) RX_NOEXCEPT {
                switch (value) {
                    case KVOCallState::Default:
                        _change.reset();
                        break;
                    default:
                        break;
                }
                _KVOCallState = value;
            }
            
            const Change &getChange() const RX_NOEXCEPT { return _change; }
            Change &getChange() RX_NOEXCEPT { return _change; }
            
            Change::Kind getNotifyKind() const RX_NOEXCEPT { return _notifyKind; }
            Change::Kind getNotifyKind() RX_NOEXCEPT { return _notifyKind; }
            
            bool isEnabled() const RX_NOEXCEPT { return _enabled; }
            void setEnabled(bool value) RX_NOEXCEPT { _enabled = value; }
            
            bool isInitialized() const RX_NOEXCEPT { return _initialized; }
            void setInitialized() RX_NOEXCEPT { _initialized = true; }
            
        public:
            bool operator==(const KVOPair &value) const RX_NOEXCEPT {
                return getObserver() == value.getObserver() &&
                (getNotifyKind() & value.getNotifyKind());
            }
            
        private:
            friend class KeyValueObserverInterface;
            Change::Kind _notifyKind;
            Change _change;
            KVOCallState _KVOCallState;
            std::atomic_bool _enabled;
            std::atomic_bool _initialized;
        };
        
        typedef SharedRef<KVOPair> KVOPairRef;
        SharedRef<std::map<const String, OrderedSetRef<KVOPairRef>>> _observersInfo;
        
        Array<KVOPairRef> _getObservers(const String &key, KeyValueObserverInterfaceParamRef object, Change::Kind notifyKind) const RX_NOEXCEPT;
        
#pragma mark - Private LockingInterface
        class NamedMutexLock : public MutexLock {
        public:
            NamedMutexLock(const String &name) : _name(name) {}
            RX_INLINE_VISIBILITY const String &getName() const { return _name; }
        private:
            const String _name;
        };
        
        mutable NamedMutexLock _observerLock;
    };
}

#endif /* KeyValueObserverInterface_hpp */
