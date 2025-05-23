// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEKIT_THREADS_BARRIER_H
#define BASEKIT_THREADS_BARRIER_H

#include "errors/exceptions.h"

#include <memory>

namespace BaseKit {

//! Barrier synchronization primitive
/*!
    A barrier for a group of threads in the source code means any thread must stop at this point
    and cannot proceed until all other threads reach this barrier.

    Thread-safe.

    https://en.wikipedia.org/wiki/Barrier_(computer_science)
*/
class Barrier
{
public:
    //! Default class constructor
    /*!
        \param threads - Count of threads to wait at the barrier
    */
    explicit Barrier(int threads);
    Barrier(const Barrier&) = delete;
    Barrier(Barrier&& barrier) = delete;
    ~Barrier();

    Barrier& operator=(const Barrier&) = delete;
    Barrier& operator=(Barrier&& barrier) = delete;

    //! Get the count of threads to wait at the barrier
    int threads() const noexcept;

    //! Wait at the barrier until all other threads reach this barrier
    /*!
        Will block.

        \return 'true' for the last thread that reach barrier, 'false' for each of the remaining threads
    */
    bool Wait();

private:
    class Impl;

    Impl& impl() noexcept { return reinterpret_cast<Impl&>(_storage); }
    const Impl& impl() const noexcept { return reinterpret_cast<Impl const&>(_storage); }

    static const size_t StorageSize = 128;
    static const size_t StorageAlign = 8;
    alignas(StorageAlign) std::byte _storage[StorageSize];
};

/*! \example threads_barrier.cpp Barrier synchronization primitive example */

} // namespace BaseKit

#endif // BASEKIT_THREADS_BARRIER_H
