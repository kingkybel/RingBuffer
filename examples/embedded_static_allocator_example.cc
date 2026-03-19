#include "ring_buffer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <new>

namespace
{
// Static storage block that can live in flash/stack in an embedded system.
template <typename T, std::size_t Capacity> struct static_storage
{
    alignas(T) std::array<std::byte, sizeof(T) * Capacity> buffer;

    constexpr std::size_t capacity() const noexcept
    {
        return Capacity;
    }

    T* data() noexcept
    {
        return reinterpret_cast<T*>(buffer.data());
    }
};

// Minimal allocator that fulfills the interface required by ring_buffer but never
// performs dynamic allocation at runtime.
template <typename T, std::size_t Capacity> class static_allocator
{
  public:
    using value_type = T;
    using pointer    = T*;
    using size_type  = std::size_t;

    static_allocator() noexcept = default;

    explicit constexpr static_allocator(T* storage, size_type capacity = Capacity) noexcept
        : storage_(storage)
        , capacity_(capacity)
    {
    }

    template <typename U>
    explicit constexpr static_allocator(static_allocator<U, Capacity> const& other) noexcept
        : storage_(other.storage_)
        , capacity_(other.capacity_)
    {
    }

    pointer allocate(size_type n)
    {
        if (storage_ == nullptr || n > capacity_)
        {
            throw std::bad_alloc();
        }
        return storage_;
    }

    void deallocate(pointer, size_type) const noexcept
    {
        // Storage is managed externally, so no-op.
    }

    template <typename U> bool operator==(static_allocator<U, Capacity> const& other) const noexcept
    {
        return storage_ == other.storage_;
    }

    template <typename U> bool operator!=(static_allocator<U, Capacity> const& other) const noexcept
    {
        return !(*this == other);
    }

  private:
    T*        storage_  = nullptr;
    size_type capacity_ = 0;

    template <typename U, std::size_t C> friend class static_allocator;
};

struct SensorSample
{
    std::uint32_t timestamp_us;
    int16_t       adc_value;
};

constexpr std::size_t kBufferCapacity = 8;
using SampleAllocator                 = static_allocator<SensorSample, kBufferCapacity>;
using SampleStorage                   = static_storage<SensorSample, kBufferCapacity>;
} // namespace

int main()
{
    static SampleStorage storage;
    SampleAllocator      allocator(storage.data());

    dkyb::ring_buffer<SensorSample, SampleAllocator> sample_buffer(kBufferCapacity, allocator);

    // Simulate periodic data collection without using malloc/free.
    for (std::uint32_t tick = 0; tick < 12; ++tick)
    {
        sample_buffer.emplace_back(SensorSample{tick * 125, static_cast<int16_t>(1'023 - tick)});
        std::cout << "Tick " << tick << ": buffered " << sample_buffer.size() << " samples\n";
    }

    // Consumer loop running on the same thread (typical in small embedded RTOS tasks).
    while (!sample_buffer.empty())
    {
        auto sample = sample_buffer.front();
        std::cout << "Processing sample @" << sample.timestamp_us << "us with ADC=" << sample.adc_value << "\n";
        sample_buffer.pop_front();
    }

    return 0;
}
