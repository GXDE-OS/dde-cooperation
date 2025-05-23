// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEKIT_THREADS_SEQLOCK_H
#define BASEKIT_THREADS_SEQLOCK_H

#include <atomic>
#include <cstring>

namespace BaseKit {

//! Sequential lock synchronization primitive
/*!
    Sequential locks are an important synchronization mechanism and
    represent a significant improvement over conventional read/write
    locks in some contexts. They avoid the need to update a synchronization
    variable during a reader critical section, and hence improve performance
    by avoiding cache coherence misses on the lock object itself.

    Thread-safe.

    https://en.wikipedia.org/wiki/Seqlock
*/
template <typename T>
class SeqLock
{
public:
    SeqLock();
    explicit SeqLock(const T& data);
    SeqLock(const SeqLock&) = delete;
    SeqLock(SeqLock&&) = default;
    ~SeqLock() = default;

    SeqLock& operator=(const T& data) noexcept;
    SeqLock& operator=(const SeqLock&) = delete;
    SeqLock& operator=(SeqLock&&) = delete;

    //! Read data under the sequential lock
    /*!
        Will block in a spin loop.

        \return Read data
    */
    T Read() const noexcept;

    //! Write data under the sequential lock
    /*!
        Will not block.

        \param data - Data to write
    */
    void Write(const T& data) noexcept;

private:
    typedef char cache_line_pad[128];

    cache_line_pad _pad0;
    T _data;
    std::atomic<size_t> _seq;
    cache_line_pad _pad1;
};

/*! \example threads_seq_lock.cpp Sequential lock synchronization primitive example */

} // namespace BaseKit

#include "seq_lock.inl"

#endif // BASEKIT_THREADS_SEQLOCK_H
