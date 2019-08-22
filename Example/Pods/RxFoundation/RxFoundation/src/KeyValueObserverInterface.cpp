//
//  KeyValueObserverInterface.cpp
//  RxFoundation
//
//  Created by closure on 3/1/17.
//  Copyright © 2017 closure. All rights reserved.
//

#include <RxFoundation/KeyValueObserverInterface.hpp>
#include <RxFoundation/SharedPointer.hpp>
#include <cassert>

using namespace Rx;

const String &Rx::KeyValueCodingInterface::ExceptionName("Rx::KeyValueCodingInterface::Exception");

Any KeyValueCodingInterface::getValueForKey(const String &keyPath) const RX_NOEXCEPT_(false) {
    return getValueForUndefinedKey(keyPath);
}

void KeyValueCodingInterface::setValueForKey(const String &keyPath, Rx::Any value) RX_NOEXCEPT_(false) {
    setValueForUndefinedKey(keyPath, value);
}

Any KeyValueCodingInterface::getValueForUndefinedKey(const String &keyPath) const RX_NOEXCEPT_(false) {
    throw Exception(Rx::KeyValueCodingInterface::ExceptionName, String("get value for undefined key: %s\n", 0, keyPath.c_str()));
}

void KeyValueCodingInterface::setValueForUndefinedKey(const String &keyPath, Any value) RX_NOEXCEPT_(false) {
    throw Exception(Rx::KeyValueCodingInterface::ExceptionName, String("set value for undefined key: %s\n", 0, keyPath.c_str()));
}

KeyValueObserverInterface::KeyValueObserverInterface() RX_NOEXCEPT :
_observersInfo(new std::map<const String, OrderedSetRef<KVOPairRef>>()),
_observerLock("KeyValueObserverInterface") {
}

KeyValueObserverInterface::~KeyValueObserverInterface() RX_NOEXCEPT {
    LockGuard<decltype(this->_observerLock)> lock(this->_observerLock);
    if (_observersInfo->size()) {
        for (auto &observerInfo : _observersInfo.get()) {
            for (auto &observer : *observerInfo.second) {
                assert(observer->getKVOCallState() == KVOCallState::Default);
            }
        }
    }
}

std::set<String> KeyValueObserverInterface::keyPathsForValuesAffectingValueForKey​​(const String &key) const RX_NOEXCEPT {
    return std::set<String>();
}

void KeyValueObserverInterface::addObserver(KeyValueObserverInterfaceParamRef observer,
                                            const String &key,
                                            Integer notifyKind,
                                            Any context) RX_NOEXCEPT {
    LockGuard<decltype(this->_observerLock)> lock(this->_observerLock);
    auto it = _observersInfo->find(key);
    if (it == _observersInfo->end()) {
        _observersInfo->insert({key, MakeShareable<OrderedSet<KVOPairRef>>()});
        it = _observersInfo->find(key);
    }
    removeObserver(observer, key);
    KVOPairRef kvoPair = MakeShareable<KVOPair>(observer, (Change::Kind)notifyKind, context);
    kvoPair->setEnabled(true);
    it->second->addObject(kvoPair);
    _willChangeValueForKey(key, Change::Kind::Initialize);
    _didChangeValueForKey(key, Change::Kind::Initialize, (Change::Kind::Initialize == (notifyKind & Change::Kind::Initialize)));
}

void KeyValueObserverInterface::removeObserver(KeyValueObserverInterfaceParamRef observer, const String &key) RX_NOEXCEPT {
    decltype(this->_observersInfo)::value_type::iterator it;
    LockGuard<decltype(this->_observerLock)> lock(this->_observerLock);
    it = _observersInfo->find(key);
    if (it == _observersInfo->end()) {
        return;
    }
    KVOPairRef target = MakeShareable<KVOPair>(observer, Change::Kind::All, nullptr);
    auto targetPosition = it->second->find([&](const KVOPairRef value) {
        bool result = (*value == *target);
        return result;
    });
    if (targetPosition != it->second->end()) {
        (*targetPosition)->setEnabled(false);
        it->second->erase(targetPosition);
        if (it->second->isEmpty()) {
            _observersInfo->erase(it);
        }
    }
}

Array<KeyValueObserverInterface::KVOPairRef> KeyValueObserverInterface::_getObservers(const String &key, KeyValueObserverInterfaceParamRef object, Change::Kind notifyKind) const RX_NOEXCEPT {
    decltype(this->_observersInfo)::value_type::iterator it;
    {
        LockGuard<decltype(this->_observerLock)> lock(this->_observerLock);
        it = _observersInfo->find(key);
        if (it == _observersInfo->end()) {
            return {};
        }
    }
    
    Array<KVOPairRef> matched;
    
    std::copy_if(it->second->begin(), it->second->end(), std::back_inserter(matched), [&](const KVOPairRef value) -> bool {
        if (object != nullptr) {
            if (value->getObserver() != object) {
                return false;
            }
        }
        return value->isEnabled() && (value->getNotifyKind() | notifyKind) != Change::Kind::Invalid;
    });
    
    if (!matched.isEmpty()) {
        Array<Index> indexes;
        for (auto idx = it->second->begin(), end = it->second->end(); idx != end; ++idx) {
            
        }
    }
    
    return matched;
}

void KeyValueObserverInterface::willChangeValueForKey(const String &key) const RX_NOEXCEPT {
    _willChangeValueForKey(key);
}

void KeyValueObserverInterface::_willChangeValueForKey(const String &key, Change::Kind kind) const RX_NOEXCEPT {
    {
        LockGuard<decltype(this->_observerLock)> lock(this->_observerLock);
        if (_observersInfo->size() == 0) {
            return;
        }
    }
    Array<KeyValueObserverInterface::KVOPairRef> observers = _getObservers(key, nullptr, kind);
    if (!observers.isEmpty()) {
        for (auto observer : observers) {
            auto object __unused = observer->getObserver();
            auto context = observer->getContext();
            auto KVOState __unused = observer->getKVOCallState();
            observer->setKVOCallState(KeyValueObserverInterface::KVOCallState::WillChange);
            if (observer->getNotifyKind() & Change::Kind::Old) {
                observer->_change._oldValue = getValueForKey(key);
            }
        }
    }
}

void KeyValueObserverInterface::didChangeValueForKey(const String &key) const RX_NOEXCEPT {
    _didChangeValueForKey(key);
}

namespace Rx {
    namespace detail {
        String _KVOChangeKind(KeyValueObserverInterface::Change::Kind kind) {
            switch (kind) {
                case KeyValueObserverInterface::Change::Kind::Invalid: return "invalid";
                case KeyValueObserverInterface::Change::Kind::New: return "new";
                case KeyValueObserverInterface::Change::Kind::Old: return "old";
                case KeyValueObserverInterface::Change::Kind::Initialize: return "initialize";
                default: return "all";
            }
        }
        
        void _LogKVO(const String &keyPath, const KeyValueObserverInterface::Change *change, const KeyValueObserverInterface *object, bool doCall, bool enabled) {
            RxCheck(change->getKind() != KeyValueObserverInterface::Change::Kind::Invalid);
            return;
//            auto &oldValue = change->getOldValue();
//            auto &newValue = change->getNewValue();
//            auto oldValueDesc = oldValue.empty() ? "false" : (oldValue ? "true" : "false");
//            auto newValueDesc = newValue.empty() ? "false" : (newValue ? "true" : "false");
//            Log("%s<%p> key: %s, old: %s, new: %s, kind: %s, do: %s, enabled: %s\n", 0, getClassName(object).c_str(), object, keyPath.c_str(), oldValueDesc, newValueDesc, _KVOChangeKind(change->getKind()).c_str(), doCall ? "true" : "false", enabled ? "true" : "false");
//            RxCheck(change->getKind() != KeyValueObserverInterface::Change::Kind::Invalid);
        }
    }
}

void KeyValueObserverInterface::_didChangeValueForKey(const String &key, Change::Kind kind, bool doCall) const RX_NOEXCEPT {
    {
        LockGuard<decltype(this->_observerLock)> lock(this->_observerLock);
        if (_observersInfo->size() == 0) {
            return;
        }
    }
    Array<KeyValueObserverInterface::KVOPairRef> observers = _getObservers(key, nullptr, kind);
    const std::set<String> &keyPaths = this->keyPathsForValuesAffectingValueForKey​​(key);
    if (!observers.isEmpty()) {
//        Log("observers: %s", 0, observers.copyDescription().c_str());
        auto thisRef = ConstCastSharedRef<KeyValueObserverInterface>(this->asShared());
        for (auto &observer : observers) {
            KeyValueObserverInterfaceParamRef &object = observer->getObserver();
            Any &context = observer->getContext();
            observer->setKVOCallState(KeyValueObserverInterface::KVOCallState::DidChange);
            if (observer->getNotifyKind() & Change::Kind::New ||
                !observer->isInitialized()) {
                observer->_change._newValue = getValueForKey(key);
            }
            
            if (observer->getNotifyKind() & Change::Kind::Initialize) {
                if (!observer->isInitialized()) {
                    observer->_change._kind = Change::Kind::Initialize;
                    observer->setInitialized();
                    detail::_LogKVO(key, &observer->_change, this, doCall, observer->isEnabled());
                    if (doCall) {
                        if (observer->isEnabled()) {
                            RxCheck(observer->_change.getKind() != KeyValueObserverInterface::Change::Kind::Invalid);
                            object->receiveObserverNotify(key, thisRef, context, observer->getChange());
                        }
                    }
                } else {
                    observer->_change._kind = (Change::Kind)kind;
                    detail::_LogKVO(key, &observer->_change, this, doCall, observer->isEnabled());
                    if (observer->_change._newValue != observer->_change._oldValue) {
                        if (doCall) {
                            if (observer->isEnabled()) {
                                RxCheck(observer->_change.getKind() != KeyValueObserverInterface::Change::Kind::Invalid);
                                object->receiveObserverNotify(key, thisRef, context, observer->getChange());
                            }
                        }
                    }
                }
            } else {
                observer->_change._kind = (Change::Kind)kind;
                detail::_LogKVO(key, &observer->_change, this, doCall, observer->isEnabled());
                if (observer->_change._newValue != observer->_change._oldValue) {
                    if (doCall) {
                        if (observer->isEnabled()) {
                            RxCheck(observer->_change.getKind() != KeyValueObserverInterface::Change::Kind::Invalid);
                            object->receiveObserverNotify(key, thisRef, context, observer->getChange());
                        }
                    }
                }
            }
            
            observer->setKVOCallState(KeyValueObserverInterface::KVOCallState::Default);
            observer->_change._oldValue = observer->_change._newValue; // update last new value to old value
        }
    }
    
    for (auto &keyPath : keyPaths) {
        if (kind == KeyValueObserverInterface::Change::Kind::Initialize) {
            _willChangeValueForKey(keyPath, kind);
        }
        _didChangeValueForKey(keyPath, kind);
    }
}
