// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#ifndef BASEKIT_SYSTEM_SHARED_MEMORY_H
#define BASEKIT_SYSTEM_SHARED_MEMORY_H

#include "errors/exceptions.h"

#include <memory>
#include <string>

namespace BaseKit {

//! Shared memory manager
/*!
    Shared memory manager allows to create named memory buffers shared between multiple processes.
    This is one of the common ways to organize different kinds of IPC (inter-process communication).

    Not thread-safe.

    https://en.wikipedia.org/wiki/Shared_memory_(interprocess_communication)
*/
class SharedMemory
{
public:
    //! Create a new or open existing block of shared memory with a given name and size
    /*!
        \param name - Shared memory block name
        \param size - Shared memory block size
    */
    explicit SharedMemory(const std::string& name, size_t size);
    SharedMemory(const SharedMemory&) = delete;
    SharedMemory(SharedMemory&& shmem) = delete;
    ~SharedMemory();

    SharedMemory& operator=(const SharedMemory&) = delete;
    SharedMemory& operator=(SharedMemory&& shmem) = delete;

    //! Check if the shared memory block is valid
    explicit operator bool() const { return (ptr() != nullptr); }

    //! Get the shared memory block name
    const std::string& name() const noexcept { return _name; }
    //! Get the shared memory block size
    size_t size() const noexcept { return _size; }

    //! Get the shared memory block pointer
    void* ptr();
    //! Get the constant shared memory block pointer
    const void* ptr() const;

    //! Get the shared memory owner flag (true if the new one was created, false if the existing one was opened)
    bool owner() const;

private:
    class Impl;

    Impl& impl() noexcept { return reinterpret_cast<Impl&>(_storage); }
    const Impl& impl() const noexcept { return reinterpret_cast<Impl const&>(_storage); }

    static const size_t StorageSize = 64;
    static const size_t StorageAlign = 8;
    alignas(StorageAlign) std::byte _storage[StorageSize];

    std::string _name;
    size_t _size;
};


} // namespace BaseKit

#endif // BASEKIT_SYSTEM_SHARED_MEMORY_H
