// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

namespace BaseKit {

inline SPSCRingBuffer::SPSCRingBuffer(size_t capacity) : _capacity(capacity), _mask(capacity - 1), _buffer(new uint8_t[capacity]), _head(0), _tail(0)
{
    assert((capacity > 1) && "Ring buffer capacity must be greater than one!");
    assert(((capacity & (capacity - 1)) == 0) && "Ring buffer capacity must be a power of two!");

    memset(_pad0, 0, sizeof(cache_line_pad));
    memset(_pad1, 0, sizeof(cache_line_pad));
    memset(_pad2, 0, sizeof(cache_line_pad));
    memset(_pad3, 0, sizeof(cache_line_pad));
}

inline size_t SPSCRingBuffer::size() const noexcept
{
    const size_t head = _head.load(std::memory_order_acquire);
    const size_t tail = _tail.load(std::memory_order_acquire);

    return head - tail;
}

inline bool SPSCRingBuffer::Enqueue(const void* data, size_t size)
{
    assert((size <= _capacity) && "Data size should not be greater than ring buffer capacity!");
    if (size > _capacity)
        return false;

    if (size == 0)
        return true;

    assert((data != nullptr) && "Pointer to the data should not be null!");
    if (data == nullptr)
        return false;

    const size_t head = _head.load(std::memory_order_relaxed);
    const size_t tail = _tail.load(std::memory_order_acquire);

    // Check if there is required free space in the ring buffer
    if ((size + head - tail) > _capacity)
        return false;

    // Copy data into the ring buffer
    size_t head_index = head & _mask;
    size_t tail_index = tail & _mask;
    size_t remain = (tail_index > head_index) ? (tail_index - head_index) : (_capacity - head_index);
    size_t first = (size > remain) ? remain : size;
    size_t last = (size > remain) ? size - remain : 0;
    memcpy(&_buffer[head_index], (uint8_t*)data, first);
    memcpy(_buffer, (uint8_t*)data + first, last);

    // Increase the head cursor
    _head.store(head + size, std::memory_order_release);

    return true;
}

inline bool SPSCRingBuffer::Dequeue(void* data, size_t& size)
{
    if (size == 0)
        return true;

    assert((data != nullptr) && "Pointer to the data should not be null!");
    if (data == nullptr)
        return false;

    const size_t tail = _tail.load(std::memory_order_relaxed);
    const size_t head = _head.load(std::memory_order_acquire);

    // Get the ring buffer size
    size_t available = head - tail;
    if (size > available)
        size = available;

    // Check if the ring buffer is empty
    if (size == 0)
        return false;

    // Copy data from the ring buffer
    size_t head_index = head & _mask;
    size_t tail_index = tail & _mask;
    size_t remain = (head_index > tail_index) ? (head_index - tail_index) : (_capacity - tail_index);
    size_t first = (size > remain) ? remain : size;
    size_t last = (size > remain) ? size - remain : 0;
    memcpy((uint8_t*)data, &_buffer[tail_index], first);
    memcpy((uint8_t*)data + first, _buffer, last);

    // Increase the tail cursor
    _tail.store(tail + size, std::memory_order_release);

    return true;
}

} // namespace BaseKit
