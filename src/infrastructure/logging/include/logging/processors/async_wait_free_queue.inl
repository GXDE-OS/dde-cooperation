// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4702) // C4702: unreachable code
#endif

namespace Logging {

template<typename T>
inline AsyncWaitFreeQueue<T>::AsyncWaitFreeQueue(size_t capacity) : _capacity(capacity), _mask(capacity - 1), _buffer(new Node[capacity]), _head(0), _tail(0)
{
    assert((capacity > 1) && "Ring queue capacity must be greater than one!");
    assert(((capacity & (capacity - 1)) == 0) && "Ring queue capacity must be a power of two!");

    memset(_pad0, 0, sizeof(cache_line_pad));
    memset(_pad1, 0, sizeof(cache_line_pad));
    memset(_pad2, 0, sizeof(cache_line_pad));
    memset(_pad3, 0, sizeof(cache_line_pad));

    // Populate the sequence initial values
    for (size_t i = 0; i < capacity; ++i)
        _buffer[i].sequence.store(i, std::memory_order_relaxed);
}

template<typename T>
inline size_t AsyncWaitFreeQueue<T>::size() const noexcept
{
    const size_t head = _head.load(std::memory_order_acquire);
    const size_t tail = _tail.load(std::memory_order_acquire);

    return head - tail;
}

template<typename T>
inline bool AsyncWaitFreeQueue<T>::Enqueue(Record& record)
{
    size_t head_sequence = _head.load(std::memory_order_relaxed);

    for (;;)
    {
        Node* node = &_buffer[head_sequence & _mask];
        size_t node_sequence = node->sequence.load(std::memory_order_acquire);

        // If node sequence and head sequence are the same then it means this slot is empty
        int64_t diff = (int64_t)node_sequence - (int64_t)head_sequence;
        if (diff == 0)
        {
            // Claim our spot by moving head. If head isn't the same
            // as we last checked then that means someone beat us to
            // the punch weak compare is faster, but can return spurious
            // results which in this instance is OK, because it's in the loop
            if (_head.compare_exchange_weak(head_sequence, head_sequence + 1, std::memory_order_relaxed))
            {
                // Store and swap the item value
                swap(node->value, record);

                // Increment the sequence so that the tail knows it's accessible
                node->sequence.store(head_sequence + 1, std::memory_order_release);
                return true;
            }
        }
        else if (diff < 0)
        {
            // If node sequence is less than head sequence then it means this slot is full
            // and therefore buffer is full
            return false;
        }
        else
        {
            // Under normal circumstances this branch should never be taken
            head_sequence = _head.load(std::memory_order_relaxed);
        }
    }

    // Never happens...
    return false;
}

template<typename T>
inline bool AsyncWaitFreeQueue<T>::Dequeue(Record& record)
{
    size_t tail_sequence = _tail.load(std::memory_order_relaxed);

    for (;;)
    {
        Node* node = &_buffer[tail_sequence & _mask];
        size_t node_sequence = node->sequence.load(std::memory_order_acquire);

        // If node sequence and head sequence are the same then it means this slot is empty
        int64_t diff = (int64_t)node_sequence - (int64_t)(tail_sequence + 1);
        if (diff == 0)
        {
            // Claim our spot by moving head. If head isn't the same
            // as we last checked then that means someone beat us to
            // the punch weak compare is faster, but can return spurious
            // results which in this instance is OK, because it's in the loop
            if (_tail.compare_exchange_weak(tail_sequence, tail_sequence + 1, std::memory_order_relaxed))
            {
                // Swap and get the item value
                swap(record, node->value);

                // Set the sequence to what the head sequence should be next time around
                node->sequence.store(tail_sequence + _mask + 1, std::memory_order_release);
                return true;
            }
        }
        else if (diff < 0)
        {
            // If seq is less than head seq then it means this slot is full and therefore the buffer is full
            return false;
        }
        else
        {
            // Under normal circumstances this branch should never be taken
            tail_sequence = _tail.load(std::memory_order_relaxed);
        }
    }

    // Never happens...
    return false;
}

} // namespace Logging

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
