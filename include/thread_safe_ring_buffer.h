/*
 * Repository:  https://github.com/kingkybel/RingBuffer
 * File Name:   include/thread_safe_ring_buffer.h
 * Description: Thread-safe wrapper around RingBuffer for multi-threaded applications.
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

#include "ring_buffer.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>

namespace dkyb
{

/**
 * @brief Thread-safe wrapper around ring_buffer
 *
 * This class provides thread-safe access to a RingBuffer instance,
 * making it suitable for producer-consumer scenarios and multi-threaded
 * applications.
 *
 * @tparam T The type of elements stored in the buffer
 * @tparam Allocator The allocator type
 */
template <typename T, typename Allocator = std::allocator<T>> class ring_buffer_thread_safe
{
  public:
    using value_type      = T;
    using size_type       = std::size_t;
    using reference       = T&;
    using const_reference = T const&;
    using pointer         = T*;
    using const_pointer   = T const*;
    using allocator_type  = Allocator;

    /**
     * @brief Construct a new Thread Safe Ring Buffer
     *
     * @param capacity The maximum number of elements the buffer can hold
     * @param allocator The allocator to use for memory management
     */
    explicit ring_buffer_thread_safe(size_type capacity, Allocator const& allocator = Allocator{})
        : buffer_(capacity, allocator)
    {
    }

    /**
     * @brief Add an element to the back of the buffer
     *
     * If the buffer is full, waits until space becomes available before inserting.
     *
     * @param item The element to add
     */
    void push_back(T const& item)
    {
        std::unique_lock lock(mutex_);
        not_full_.wait(lock, [this] { return !buffer_.full(); });
        buffer_.push_back(item);
        not_empty_.notify_one();
    }

    /**
     * @brief Add an element to the back of the buffer (move version)
     *
     * If the buffer is full, waits until space becomes available before inserting.
     *
     * @param item The element to add
     */
    void push_back(T&& item)
    {
        std::unique_lock lock(mutex_);
        not_full_.wait(lock, [this] { return !buffer_.full(); });
        buffer_.push_back(std::move(item));
        not_empty_.notify_one();
    }

    /**
     * @brief Add an element to the back of the buffer (emplace version)
     *
     * Constructs the element in-place at the back of the buffer.
     * If the buffer is full, waits until space becomes available before inserting.
     *
     * @param args Arguments to forward to the constructor of T
     */
    template <typename... Args> void emplace_back(Args&&... args)
    {
        std::unique_lock lock(mutex_);
        not_full_.wait(lock, [this] { return !buffer_.full(); });
        buffer_.emplace_back(std::forward<Args>(args)...);
        not_empty_.notify_one();
    }

    /**
     * @brief Try to add an element to the buffer without blocking
     *
     * @param item The element to add
     * @return true if the element was added, false if the buffer was full
     */
    bool try_push_back(T const& item)
    {
        std::lock_guard lock(mutex_);
        if (buffer_.full())
        {
            return false;
        }
        buffer_.push_back(item);
        not_empty_.notify_one();
        return true;
    }

    /**
     * @brief Try to add an element to the buffer without blocking (move version)
     *
     * @param item The element to add
     * @return true if the element was added, false if the buffer was full
     */
    bool try_push_back(T&& item)
    {
        std::lock_guard lock(mutex_);
        if (buffer_.full())
        {
            return false;
        }
        buffer_.push_back(std::move(item));
        not_empty_.notify_one();
        return true;
    }

    /**
     * @brief Try to add an element to the buffer with a timeout
     *
     * @param item The element to add
     * @param timeout Maximum time to wait for space in the buffer
     * @return true if the element was added, false if timeout occurred
     */
    template <typename Rep, typename Period>
    bool push_back_with_timeout(T const& item, std::chrono::duration<Rep, Period> const& timeout)
    {

        // Wait for space to become available
        if (std::unique_lock lock(mutex_);
            buffer_.full() && !not_full_.wait_for(lock, timeout, [this] { return !buffer_.full(); }))
        {
            return false; // Timeout occurred
        }

        buffer_.push_back(item);
        not_empty_.notify_one();
        return true;
    }

    /**
     * @brief Try to add an element to the buffer with a timeout (move version)
     *
     * @param item The element to add
     * @param timeout Maximum time to wait for space in the buffer
     * @return true if the element was added, false if timeout occurred
     */
    template <typename Rep, typename Period>
    bool push_back_with_timeout(T&& item, std::chrono::duration<Rep, Period> const& timeout)
    {

        // Wait for space to become available
        if (std::unique_lock lock(mutex_);
            buffer_.full() && !not_full_.wait_for(lock, timeout, [this] { return !buffer_.full(); }))
        {
            return false; // Timeout occurred
        }

        buffer_.push_back(std::move(item));
        not_empty_.notify_one();
        return true;
    }

    /**
     * @brief Remove and return the front element
     *
     * @return The front element
     * @throws std::runtime_error if the buffer is empty
     */
    T pop_front()
    {
        std::lock_guard lock(mutex_);
        T               item = buffer_.pop_front();
        not_full_.notify_one();
        return item;
    }

    /**
     * @brief Try to remove and return the front element without blocking
     *
     * @return The front element if available, std::nullopt otherwise
     */
    std::optional<T> try_pop_front()
    {
        std::lock_guard lock(mutex_);
        if (buffer_.empty())
        {
            return std::nullopt;
        }
        T item = buffer_.pop_front();
        not_full_.notify_one();
        return item;
    }

    /**
     * @brief Try to remove and return the front element with a timeout
     *
     * @param timeout Maximum time to wait for an element to become available
     * @return The front element if available, std::nullopt if timeout occurred
     */
    template <typename Rep, typename Period>
    std::optional<T> pop_front_with_timeout(std::chrono::duration<Rep, Period> const& timeout)
    {

        // Wait for an element to become available
    if (std::unique_lock lock(mutex_);
            buffer_.empty() && !not_empty_.wait_for(lock, timeout, [this] { return !buffer_.empty(); }))
        {
            return std::nullopt; // Timeout occurred
        }

        T item = buffer_.pop_front();
        not_full_.notify_one();
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
        std::lock_guard lock(mutex_);
        return buffer_.front();
    }

    /**
     * @brief Access the front element without removing it (const version)
     *
     * @return Const reference to the front element
     * @throws std::runtime_error if the buffer is empty
     */
    const_reference front() const
    {
        std::lock_guard lock(mutex_);
        return buffer_.front();
    }

    /**
     * @brief Access the back element without removing it
     *
     * @return Reference to the back element
     * @throws std::runtime_error if the buffer is empty
     */
    reference back()
    {
        std::lock_guard lock(mutex_);
        return buffer_.back();
    }

    /**
     * @brief Access the back element without removing it (const version)
     *
     * @return Const reference to the back element
     * @throws std::runtime_error if the buffer is empty
     */
    const_reference back() const
    {
        std::lock_guard lock(mutex_);
        return buffer_.back();
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
        std::lock_guard lock(mutex_);
        return buffer_[index];
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
        std::lock_guard lock(mutex_);
        return buffer_[index];
    }

    /**
     * @brief Get the number of elements currently in the buffer
     *
     * @return The number of elements
     */
    size_type size() const
    {
        std::lock_guard lock(mutex_);
        return buffer_.size();
    }

    /**
     * @brief Get the maximum capacity of the buffer
     *
     * @return The maximum number of elements the buffer can hold
     */
    size_type capacity() const
    {
        std::lock_guard lock(mutex_);
        return buffer_.capacity();
    }

    /**
     * @brief Check if the buffer is empty
     *
     * @return true if the buffer contains no elements
     */
    bool empty() const
    {
        std::lock_guard lock(mutex_);
        return buffer_.empty();
    }

    /**
     * @brief Check if the buffer is full
     *
     * @return true if the buffer contains the maximum number of elements
     */
    bool full() const
    {
        std::lock_guard lock(mutex_);
        return buffer_.full();
    }

    /**
     * @brief Get the number of free slots available
     *
     * @return The number of elements that can be added before overwriting
     */
    size_type available() const
    {
        std::lock_guard lock(mutex_);
        return buffer_.available();
    }

    /**
     * @brief Remove all elements from the buffer
     */
    void clear()
    {
        std::lock_guard lock(mutex_);
        buffer_.clear();
        not_full_.notify_all();
    }

    /**
     * @brief Wait until the buffer is not empty
     *
     * @param timeout Maximum time to wait
     * @return true if the buffer is not empty, false if timeout occurred
     */
    template <typename Rep, typename Period>
    bool wait_until_not_empty(std::chrono::duration<Rep, Period> const& timeout)
    {
        std::unique_lock lock(mutex_);
        return not_empty_.wait_for(lock, timeout, [this] { return !buffer_.empty(); });
    }

    /**
     * @brief Wait until the buffer is not full
     *
     * @param timeout Maximum time to wait
     * @return true if the buffer is not full, false if timeout occurred
     */
    template <typename Rep, typename Period> bool wait_until_not_full(std::chrono::duration<Rep, Period> const& timeout)
    {
        std::unique_lock lock(mutex_);
        return not_full_.wait_for(lock, timeout, [this] { return !buffer_.full(); });
    }

  private:
    ring_buffer<T, Allocator> buffer_;
    mutable std::mutex        mutex_;
    std::condition_variable   not_empty_;
    std::condition_variable   not_full_;
};

} // namespace dkyb
