//
//  Atomic.cpp
//  RxFoundation
//
//  Created by closure on 10/30/16.
//  Copyright © 2016 closure. All rights reserved.
//

#include <RxFoundation/Atomic.hpp>

#if DEPLOYMENT_TARGET_MACOSX
#include <pthread/pthread.h>
#include <libkern/OSAtomic.h>
#include <dispatch/dispatch.h>
#elif DEPLOYMENT_TARGET_ANDROID
#include <pthread.h>
#endif

#include <thread>

using namespace Rx;

void SpinLock::lock() {
    while (locked.test_and_set(std::memory_order_acquire)) {
        ;
    }
}

bool SpinLock::tryLock(void) {
    return locked.test_and_set(std::memory_order_acquire);
}

void SpinLock::unlock(void) {
    locked.clear(std::memory_order_release);
}

MutexLock::MutexLock() RX_NOEXCEPT {
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_impl, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
}

MutexLock::~MutexLock() {
    pthread_mutex_destroy(&_impl);
}

bool MutexLock::tryLock() {
    return 0 == pthread_mutex_trylock(&_impl);
}

void MutexLock::lock() {
    pthread_mutex_lock(&_impl);
}

void MutexLock::unlock() {
    pthread_mutex_unlock(&_impl);
}

ConditionVariable::~ConditionVariable() {
}

void ConditionVariable::wait(LockGuard<MutexLock> &lock) RX_NOEXCEPT {
    pthread_cond_wait(&_condition, lock.getLock()->getNativeHandle());
}

void ConditionVariable::notifyAll() RX_NOEXCEPT {
    pthread_cond_broadcast(&_condition);
}

void ConditionVariable::notifyOne() RX_NOEXCEPT {
    pthread_cond_signal(&_condition);
}

ReadWriteLock::ReadWriteLock() RX_NOEXCEPT : _reader(0), _writer(0), _pending(0) {
}

ReadWriteLock::~ReadWriteLock() {
}

void ReadWriteLock::lockRead() RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    while (_writer > 0 || _pending > 0) {
        _condition.wait(lock);
    }
    ++_reader;
}

void ReadWriteLock::unlockRead() RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    assert(_reader > 0 && "unlockRead when there is no reader!");
    --_reader;
    if (_reader == 0) {
        _condition.notifyAll();
    }
}

bool ReadWriteLock::tryLockRead() RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    while (_writer > 0 || _pending > 0) {
        return false;
    }
    ++_reader;
    return true;
}

void ReadWriteLock::lockWrite() RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    ++_pending;
    while (_writer > 0 || _reader > 0) {
        _condition.wait(lock);
    }
    --_pending;
    ++_writer;
}

void ReadWriteLock::unlockWrite() RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    assert(_writer > 0 && "unlock write when there is no writer!");
    --_writer;
    _condition.notifyAll();
}

bool ReadWriteLock::tryLockWrite() RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    while (_writer > 0 || _reader > 0) {
        return false;
    }
    ++_writer;
    return true;
}

bool ReadWriteLock::isReading() const RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    return _reader > 0;
}

bool ReadWriteLock::isWriting() const RX_NOEXCEPT {
    LockGuard<decltype(_mutex)> lock(_mutex);
    return _writer > 0;
}

Semaphore::Semaphore(Integer value) {
#if DEPLOYMENT_TARGET_MACOSX
    _impl = dispatch_semaphore_create(value);
#elif defined(__ANDROID_NDK__)
    sem_init(&_impl, value, 0);
#endif
}

//Semaphore::Semaphore(Semaphore &&value) : _impl(value._impl) {
//    value._impl = nullptr;
//}
//
//Semaphore::Semaphore(const Semaphore &value) {
//    _impl = value._impl;
//    if (_impl) {
//        dispatch_retain(_impl);
//    }
//    dispatch_debug(_impl, __FUNCTION__);
//}
//
//Semaphore &Semaphore::operator=(const Semaphore &value) {
//    if (_impl) {
//        dispatch_release(_impl);
//    }
//    _impl = value._impl;
//    if (_impl) {
//        dispatch_retain(_impl);
//    }
//    dispatch_debug(_impl, __FUNCTION__);
//    return *this;
//}

Semaphore::~Semaphore() {
#if DEPLOYMENT_TARGET_MACOSX
    if (_impl) {
        dispatch_release(_impl);
        _impl = nullptr;
    }
#elif defined(__ANDROID_NDK__)
    sem_destroy(&_impl);
#endif
}

void Semaphore::signal() {
#if DEPLOYMENT_TARGET_MACOSX
    dispatch_semaphore_signal((dispatch_semaphore_t)_impl);
#elif defined(__ANDROID_NDK__)
    sem_post(&_impl);
#endif
}

bool Semaphore::wait(UInt64 timeout) {
#if DEPLOYMENT_TARGET_MACOSX
    return (0 == dispatch_semaphore_wait((dispatch_semaphore_t)_impl, timeout));
#elif defined(__ANDROID_NDK__)
    struct timespec ts;
    ts.tv_nsec = timeout;
    return 0 == sem_timedwait(&_impl, &ts);
#endif
}
