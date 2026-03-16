/*
 * Repository:  https://github.com/kingkybel/RingBuffer
 * File Name:   include/ring_buffer.h
 * Description: A ring buffer (also known as circular buffer or circular queue) is a data structure that uses a single, fixed-size buffer as if it were connected end-to-end.
 *
 * Copyright (C) 2026 Dieter J Kybelksties
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * @date: 2026-03-13
 * @author: Dieter J Kybelksties
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>

namespace ringbuffer
{

/**
 * @brief A fixed-size circular buffer implementation
 *
 * This ring buffer provides O(1) push and pop operations with efficient
 * memory usage. It's designed for scenarios where you need a sliding window
 * of data or producer-consumer patterns.
 *
 * @tparam T The type of elements stored in the buffer
 * @tparam Allocator The allocator type (defaults to std::allocator<T>)
 */
template <typename T, typename Allocator = std::allocator<T>> class RingBuffer
{
  public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T&;
    using const_reference = T const&;
    using pointer         = T*;
    using const_pointer   = T const*;
    using allocator_type  = Allocator;

    /**
     * @brief Iterator class for RingBuffer
     */
    class iterator
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator()
            : buffer_(nullptr)
            , index_(0)
            , size_(0)
        {
        }

        iterator(RingBuffer* buffer, size_type index)
            : buffer_(buffer)
            , index_(index)
            , size_(buffer->size())
        {
        }

        reference operator*() const
        {
            return (*buffer_)[index_];
        }

        pointer operator->() const
        {
            return &(*buffer_)[index_];
        }

        iterator& operator++()
        {
            ++index_;
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(iterator const& other) const
        {
            return buffer_ == other.buffer_ && index_ == other.index_;
        }

        bool operator!=(iterator const& other) const
        {
            return !(*this == other);
        }

      private:
        RingBuffer* buffer_;
        size_type   index_;
        size_type   size_;
    };

    /**
     * @brief Const iterator class for RingBuffer
     */
    class const_iterator
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T const;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T const*;
        using reference         = T const&;

        const_iterator()
            : buffer_(nullptr)
            , index_(0)
            , size_(0)
        {
        }

        const_iterator(RingBuffer const* buffer, size_type index)
            : buffer_(buffer)
            , index_(index)
            , size_(buffer->size())
        {
        }

        explicit const_iterator(iterator const& it)
            : buffer_(it.buffer_)
            , index_(it.index_)
            , size_(it.size_)
        {
        }

        reference operator*() const
        {
            return (*buffer_)[index_];
        }

        pointer operator->() const
        {
            return &(*buffer_)[index_];
        }

        const_iterator& operator++()
        {
            ++index_;
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const_iterator const& other) const
        {
            return buffer_ == other.buffer_ && index_ == other.index_;
        }

        bool operator!=(const_iterator const& other) const
        {
            return !(*this == other);
        }

      private:
        RingBuffer const* buffer_;
        size_type         index_;
        size_type         size_;
    };

    /**
     * @brief Construct a new Ring Buffer
     *
     * @param capacity The maximum number of elements the buffer can hold
     * @param allocator The allocator to use for memory management
     */
    explicit RingBuffer(size_type capacity, Allocator const& allocator = Allocator{})
        : capacity_(capacity)
        , allocator_(allocator)
        , buffer_(allocator_.allocate(capacity_))
        , head_(0)
        , tail_(0)
        , count_(0)
        , full_(false)
        , just_overwrote_(false)
        , overwrite_position_(0)
    {

        if (capacity_ == 0)
        {
            throw std::invalid_argument("Ring buffer capacity must be greater than 0");
        }
    }

    /**
     * @brief Copy constructor
     */
    RingBuffer(RingBuffer const& other)
        : capacity_(other.capacity_)
        , allocator_(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.allocator_))
        , buffer_(allocator_.allocate(capacity_))
        , head_(other.head_)
        , tail_(other.tail_)
        , count_(other.count_)
        , full_(other.full_)
        , just_overwrote_(false)
    {

        // Copy elements
        for (size_type i = 0; i < count_; ++i)
        {
            std::allocator_traits<Allocator>::construct(allocator_, &buffer_[i], other.buffer_[i]);
        }
    }

    /**
     * @brief Move constructor
     */
    RingBuffer(RingBuffer&& other) noexcept
        : capacity_(other.capacity_)
        , allocator_(std::move(other.allocator_))
        , buffer_(other.buffer_)
        , head_(other.head_)
        , tail_(other.tail_)
        , count_(other.count_)
        , full_(other.full_)
        , just_overwrote_(false)
    {

        other.buffer_   = nullptr;
        other.capacity_ = 0;
        other.head_     = 0;
        other.tail_     = 0;
        other.count_    = 0;
        other.full_     = false;
    }

    /**
     * @brief Copy assignment operator
     */
    RingBuffer& operator=(RingBuffer const& other)
    {
        if (this != &other)
        {
            // Destroy current elements
            clear();

            // Deallocate current buffer
            if (buffer_)
            {
                allocator_.deallocate(buffer_, capacity_);
            }

            // Copy allocator if required
            if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value)
            {
                allocator_ = other.allocator_;
            }

            // Allocate new buffer
            capacity_ = other.capacity_;
            buffer_   = allocator_.allocate(capacity_);
            head_     = other.head_;
            tail_     = other.tail_;
            count_    = other.count_;
            full_     = other.full_;

            // Copy elements
            for (size_type i = 0; i < count_; ++i)
            {
                std::allocator_traits<Allocator>::construct(allocator_, &buffer_[i], other.buffer_[i]);
            }
            just_overwrote_ = false;
        }
        return *this;
    }

    /**
     * @brief Move assignment operator
     */
    RingBuffer& operator=(RingBuffer&& other) noexcept
    {
        if (this != &other)
        {
            // Destroy current elements
            clear();

            // Deallocate current buffer
            if (buffer_)
            {
                allocator_.deallocate(buffer_, capacity_);
            }

            // Move resources
            capacity_  = other.capacity_;
            allocator_ = std::move(other.allocator_);
            buffer_    = other.buffer_;
            head_      = other.head_;
            tail_      = other.tail_;
            count_     = other.count_;
            full_      = other.full_;

            // Reset other
            other.buffer_   = nullptr;
            other.capacity_ = 0;
            other.head_     = 0;
            other.tail_     = 0;
            other.count_    = 0;
            other.full_     = false;
        }
        return *this;
    }

    /**
     * @brief Destructor
     */
    ~RingBuffer()
    {
        clear();
        if (buffer_)
        {
            allocator_.deallocate(buffer_, capacity_);
        }
    }

    /**
     * @brief Add an element to the back of the buffer
     *
     * If the buffer is full, the oldest element will be overwritten.
     *
     * @param item The element to add
     */
    void push_back(T const& item)
    {
        if (full_)
        {
            // Overwrite the oldest element
            buffer_[head_] = item;
            overwrite_advance_head();
        }
        else
        {
            // Add new element
            std::allocator_traits<Allocator>::construct(allocator_, &buffer_[tail_], item);
            increment_tail();
        }
    }

    /**
     * @brief Add an element to the back of the buffer (move version)
     *
     * If the buffer is full, the oldest element will be overwritten.
     *
     * @param item The element to add
     */
    void push_back(T&& item)
    {
        if (full_)
        {
            // Overwrite the oldest element
            buffer_[head_] = std::move(item);
            overwrite_advance_head();
        }
        else
        {
            // Add new element
            std::allocator_traits<Allocator>::construct(allocator_, &buffer_[tail_], std::move(item));
            increment_tail();
        }
    }

    /**
     * @brief Add an element to the back of the buffer (emplace version)
     *
     * Constructs the element in-place at the back of the buffer.
     * If the buffer is full, the oldest element will be overwritten.
     *
     * @param args Arguments to forward to the constructor of T
     */
    template <typename... Args> void emplace_back(Args&&... args)
    {
        if (full_)
        {
            // Destroy the oldest element
            std::allocator_traits<Allocator>::destroy(allocator_, &buffer_[head_]);
            // Construct new element in its place
            std::allocator_traits<Allocator>::construct(allocator_, &buffer_[head_], std::forward<Args>(args)...);
            overwrite_advance_head();
        }
        else
        {
            // Construct new element at tail
            std::allocator_traits<Allocator>::construct(allocator_, &buffer_[tail_], std::forward<Args>(args)...);
            increment_tail();
        }
    }

    /**
     * @brief Remove and return the front element
     *
     * @return The front element
     * @throws std::runtime_error if the buffer is empty
     */
    T pop_front()
    {
        if (empty())
        {
            throw std::runtime_error("Cannot pop from empty ring buffer");
        }

        T item = std::move(buffer_[head_]);
        std::allocator_traits<Allocator>::destroy(allocator_, &buffer_[head_]);
        increment_head();

        return item;
    }

    /**
     * @brief Access the front element without removing it
     *
     * @return Reference to the front element
     * @throws std::runtime_error if the buffer is empty
     */
    reference front()
    {
        if (empty())
        {
            throw std::runtime_error("Cannot access front of empty ring buffer");
        }
        return buffer_[head_];
    }

    /**
     * @brief Access the front element without removing it (const version)
     *
     * @return Const reference to the front element
     * @throws std::runtime_error if the buffer is empty
     */
    const_reference front() const
    {
        if (empty())
        {
            throw std::runtime_error("Cannot access front of empty ring buffer");
        }
        return buffer_[head_];
    }

    /**
     * @brief Access the back element without removing it
     *
     * @return Reference to the back element
     * @throws std::runtime_error if the buffer is empty
     */
    reference back()
    {
        if (empty())
        {
            throw std::runtime_error("Cannot access back of empty ring buffer");
        }
        if (just_overwrote_)
        {
            // When we just overwrote, the back element is at the position we wrote to
            return buffer_[overwrite_position_];
        }
        else
        {
            // Normal case: back element is at tail - 1
            return buffer_[decrement_index(tail_)];
        }
    }

    /**
     * @brief Access the back element without removing it (const version)
     *
     * @return Const reference to the back element
     * @throws std::runtime_error if the buffer is empty
     */
    const_reference back() const
    {
        if (empty())
        {
            throw std::runtime_error("Cannot access back of empty ring buffer");
        }
        if (just_overwrote_)
        {
            // When we just overwrote, the back element is at the position we wrote to
            return buffer_[overwrite_position_];
        }
        else
        {
            // Normal case: back element is at tail - 1
            return buffer_[decrement_index(tail_)];
        }
    }

    /**
     * @brief Access element at specified index
     *
     * @param index The index of the element (0-based from front)
     * @return Reference to the element
     * @throws std::out_of_range if index is out of bounds
     */
    reference operator[](size_type index)
    {
        if (index >= count_)
        {
            throw std::out_of_range(std::format("Index {} out of range [size={}]", index, capacity_));
        }
        return buffer_[(head_ + index) % capacity_];
    }

    /**
     * @brief Access element at specified index (const version)
     *
     * @param index The index of the element (0-based from front)
     * @return Const reference to the element
     * @throws std::out_of_range if index is out of bounds
     */
    const_reference operator[](size_type index) const
    {
        if (index >= count_)
        {
            throw std::out_of_range(std::format("Index {} out of range [size={}]", index, capacity_));
        }
        return buffer_[(head_ + index) % capacity_];
    }

    /**
     * @brief Get the number of elements currently in the buffer
     *
     * @return The number of elements
     */
    [[nodiscard]] size_type size() const noexcept
    {
        return count_;
    }

    /**
     * @brief Get the maximum capacity of the buffer
     *
     * @return The maximum number of elements the buffer can hold
     */
    [[nodiscard]] size_type capacity() const noexcept
    {
        return capacity_;
    }

    /**
     * @brief Check if the buffer is empty
     *
     * @return true if the buffer contains no elements
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return count_ == 0;
    }

    /**
     * @brief Check if the buffer is full
     *
     * @return true if the buffer contains the maximum number of elements
     */
    [[nodiscard]] bool full() const noexcept
    {
        return full_;
    }

    /**
     * @brief Get the number of free slots available
     *
     * @return The number of elements that can be added before overwriting
     */
    [[nodiscard]] size_type available() const noexcept
    {
        return full_ ? 0 : capacity_ - count_;
    }

    /**
     * @brief Remove all elements from the buffer
     */
    void clear() noexcept
    {
        // Destroy all elements
        for (size_type i = 0; i < count_; ++i)
        {
            std::allocator_traits<Allocator>::destroy(allocator_, &buffer_[(head_ + i) % capacity_]);
        }
        head_  = 0;
        tail_  = 0;
        count_ = 0;
        full_  = false;
    }

    /**
     * @brief Get iterator to the beginning
     *
     * @return Iterator to the first element
     */
    iterator begin()
    {
        return iterator(this, 0);
    }

    /**
     * @brief Get iterator to the end
     *
     * @return Iterator to one past the last element
     */
    iterator end()
    {
        return iterator(this, count_);
    }

    /**
     * @brief Get const iterator to the beginning
     *
     * @return Const iterator to the first element
     */
    const_iterator begin() const
    {
        return const_iterator(this, 0);
    }

    /**
     * @brief Get const iterator to the end
     *
     * @return Const iterator to one past the last element
     */
    const_iterator end() const
    {
        return const_iterator(this, count_);
    }

    /**
     * @brief Get const iterator to the beginning
     *
     * @return Const iterator to the first element
     */
    const_iterator cbegin() const
    {
        return const_iterator(this, 0);
    }

    /**
     * @brief Get const iterator to the end
     *
     * @return Const iterator to one past the last element
     */
    const_iterator cend() const
    {
        return const_iterator(this, count_);
    }

    /**
     * @brief Debug method to print buffer contents (for debugging only)
     */
    void debug_print() const
    {
        std::cout << "Buffer contents: ";
        for (size_type i = 0; i < capacity_; ++i)
        {
            std::cout << buffer_[i] << " ";
        }
        std::cout << " | head=" << head_ << " tail=" << tail_ << " count=" << count_ << " full=" << full_ << " just_overwrote=" << just_overwrote_ << std::endl;
    }

  private:
    /**
     * @brief Increment the head index, wrapping around if necessary
     */
    void increment_head() noexcept
    {
        head_ = (head_ + 1) % capacity_;
        if (full_)
        {
            // When the buffer was full, removing one element makes it not full.
            full_ = false;
        }
        if (count_ > 0)
        {
            --count_;
        }
        just_overwrote_ = false;
    }

    /**
     * @brief Advance the head index when overwriting an element (buffer remains full)
     */
    void overwrite_advance_head() noexcept
    {
        // Store the position where we wrote the element
        overwrite_position_ = head_;
        head_ = (head_ + 1) % capacity_;
        just_overwrote_ = true;
        // Buffer remains full after overwriting
    }

    /**
     * @brief Increment the tail index, wrapping around if necessary
     */
    void increment_tail() noexcept
    {
        tail_ = (tail_ + 1) % capacity_;
        if (!full_)
        {
            count_++;
            full_ = (count_ == capacity_);
        }
        just_overwrote_ = false;
    }

    /**
     * @brief Decrement an index, wrapping around if necessary
     */
    [[nodiscard]] size_type decrement_index(size_type index) const noexcept
    {
        return index == 0 ? capacity_ - 1 : index - 1;
    }

    size_type capacity_;
    Allocator allocator_;
    pointer   buffer_;
    size_type head_;
    size_type tail_;
    size_type count_;
    bool      full_;
    bool      just_overwrote_;
    size_type overwrite_position_;
};

} // namespace ringbuffer
