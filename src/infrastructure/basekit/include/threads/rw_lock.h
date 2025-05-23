// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEKIT_THREADS_RW_LOCK_H
#define BASEKIT_THREADS_RW_LOCK_H

#include "threads/locker.h"
#include "time/timestamp.h"

#include <memory>

namespace BaseKit {

//! Read/Write lock synchronization primitive
/*!
    An read/write lock allows concurrent access for read-only operations, while write operations
    require exclusive access. This means that multiple threads can read the data in parallel but
    an exclusive lock is needed for writing or modifying data. When a writer is writing the data,
    all other writers or readers will be blocked until the writer is finished writing.

    Thread-safe.

    https://en.wikipedia.org/wiki/Readers%E2%80%93writer_lock
*/
class RWLock
{
public:
    RWLock();
    RWLock(const RWLock&) = delete;
    RWLock(RWLock&& lock) = delete;
    ~RWLock();

    RWLock& operator=(const RWLock&) = delete;
    RWLock& operator=(RWLock&& lock) = delete;

    //! Try to acquire read lock without block
    /*!
        Will not block.

        \return 'true' if the read lock was successfully acquired, 'false' if the read lock is busy
    */
    bool TryLockRead();
    //! Try to acquire write lock without block
    /*!
        Will not block.

        \return 'true' if the write lock was successfully acquired, 'false' if the write lock is busy
    */
    bool TryLockWrite();

    //! Try to acquire read lock for the given timespan
    /*!
        Will block for the given timespan in the worst case.

        \param timespan - Timespan to wait for the read lock
        \return 'true' if the read lock was successfully acquired, 'false' if the read lock is busy
    */
    bool TryLockReadFor(const Timespan& timespan);
    //! Try to acquire write lock for the given timespan
    /*!
        Will block for the given timespan in the worst case.

        \param timespan - Timespan to wait for the write lock
        \return 'true' if the write lock was successfully acquired, 'false' if the write lock is busy
    */
    bool TryLockWriteFor(const Timespan& timespan);
    //! Try to acquire read lock until the given timestamp
    /*!
        Will block until the given timestamp in the worst case.

        \param timestamp - Timestamp to stop wait for the read lock
        \return 'true' if the read lock was successfully acquired, 'false' if the read lock is busy
    */
    bool TryLockReadUntil(const UtcTimestamp& timestamp)
    { return TryLockReadFor(timestamp - UtcTimestamp()); }
    //! Try to acquire write lock until the given timestamp
    /*!
        Will block until the given timestamp in the worst case.

        \param timestamp - Timestamp to stop wait for the write lock
        \return 'true' if the write lock was successfully acquired, 'false' if the write lock is busy
    */
    bool TryLockWriteUntil(const UtcTimestamp& timestamp)
    { return TryLockWriteFor(timestamp - UtcTimestamp()); }

    //! Acquire read lock with block
    /*!
        Will block.
    */
    void LockRead();
    //! Acquire write lock with block
    /*!
        Will block.
    */
    void LockWrite();

    //! Release read lock
    /*!
        Will not block.
    */
    void UnlockRead();
    //! Release write lock
    /*!
        Will not block.
    */
    void UnlockWrite();

private:
    class Impl;

    Impl& impl() noexcept { return reinterpret_cast<Impl&>(_storage); }
    const Impl& impl() const noexcept { return reinterpret_cast<Impl const&>(_storage); }

#if defined(__APPLE__)
    static const size_t StorageSize = 256;
#else
    static const size_t StorageSize = 56;
#endif
    static const size_t StorageAlign = 8;
    alignas(StorageAlign) std::byte _storage[StorageSize];
};

/*! \example threads_rw_lock.cpp Read/Write lock synchronization primitive example */

} // namespace BaseKit

#endif // BASEKIT_THREADS_RW_LOCK_H
