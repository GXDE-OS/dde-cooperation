// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEKIT_THREADS_SPIN_BARRIER_H
#define BASEKIT_THREADS_SPIN_BARRIER_H

#include <atomic>
#include <cassert>

namespace BaseKit {

//! Spin barrier synchronization primitive
/*!
    A barrier for a group of threads in the source code means any thread must stop at this point and cannot
    proceed until all other threads reach this barrier.

    Spin version: threads will busy-wait and waste CPU cycles instead of yielding the CPU to another thread.
    Do not use spinlocks unless you are certain that you understand the consequences!

    Thread-safe.

    https://en.wikipedia.org/wiki/Barrier_(computer_science)
    https://en.wikipedia.org/wiki/Spinlock
*/
class SpinBarrier
{
public:
    //! Default class constructor
    /*!
        \param threads - Count of threads to wait at the barrier
    */
    explicit SpinBarrier(int threads) noexcept;
    SpinBarrier(const SpinBarrier&) = delete;
    SpinBarrier(SpinBarrier&&) = delete;
    ~SpinBarrier() = default;

    SpinBarrier& operator=(const SpinBarrier&) = delete;
    SpinBarrier& operator=(SpinBarrier&&) = delete;

    //! Get the count of threads to wait at the barrier
    int threads() const noexcept { return _threads; }

    //! Wait at the barrier until all other threads reach this barrier
    /*!
        Will block.

        \return 'true' for the last thread that reach barrier, 'false' for each of the remaining threads
    */
    bool Wait() noexcept;

private:
    std::atomic<int> _counter;
    std::atomic<int> _generation;
    int _threads;
};

/*! \example threads_spin_barrier.cpp Spin barrier synchronization primitive example */

} // namespace BaseKit

#include "spin_barrier.inl"

#endif // BASEKIT_THREADS_SPIN_BARRIER_H
