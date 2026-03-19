/*
 * Repository:  https://github.com/kingkybel/ring_buffer
 * File Name:   benchmarks/benchmark_ring_buffer.cpp
 * Description: Performance benchmarks for ring_buffer implementation.
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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// Include the ring buffer implementations
#include "ring_buffer.h"
#include "thread_safe_ring_buffer.h"

using namespace dkyb;

template <typename T> class BenchmarkSuite
{
  private:
    static constexpr size_t WARMUP_ITERATIONS  = 1'000;
    static constexpr size_t MEASURE_ITERATIONS = 100'000;

  public:
    static void benchmark_push_pop_operations()
    {
        std::cout << "Benchmarking push/pop operations..." << std::endl;

        // Test different buffer sizes
        std::vector<size_t> capacities = {16, 64, 256, 1'024, 4'096, 16'384};

        for (size_t capacity: capacities)
        {
            T buffer(capacity);

            // Warmup
            for (size_t i = 0; i < WARMUP_ITERATIONS; ++i)
            {
                buffer.push_back(static_cast<int>(i));
                if (buffer.size() > capacity / 2)
                {
                    buffer.pop_front();
                }
            }

            // Measure
            auto start = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < MEASURE_ITERATIONS; ++i)
            {
                buffer.push_back(static_cast<int>(i));
                if (buffer.size() > capacity / 2)
                {
                    buffer.pop_front();
                }
            }

            auto end      = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

            double avg_ns         = static_cast<double>(duration.count()) / MEASURE_ITERATIONS;
            double ops_per_second = 1e9 / avg_ns;

            std::cout << "  Capacity " << capacity << ": " << avg_ns << " ns/op, " << ops_per_second << " ops/sec"
                      << std::endl;
        }
    }

    static void benchmark_sequential_access()
    {
        std::cout << "Benchmarking sequential access..." << std::endl;

        T buffer(10'000);

        // Fill buffer
        for (int i = 0; i < 10'000; ++i)
        {
            buffer.push_back(i);
        }

        // Warmup
        for (size_t i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            for (size_t j = 0; j < buffer.size(); ++j)
            {
                int volatile value = buffer[j]; // NOSONAR S3687: volatile forces a real memory read so the compiler
                                                // cannot remove this access.

                (void)value;
            }
        }

        // Measure
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < MEASURE_ITERATIONS; ++i)
        {
            for (size_t j = 0; j < buffer.size(); ++j)
            {
                int volatile value = buffer[j]; // NOSONAR S3687: volatile forces a real memory read so the compiler
                                                // cannot remove this access.

                (void)value;
            }
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        auto total_accesses      = static_cast<double>(MEASURE_ITERATIONS * buffer.size());
        auto avg_ns              = static_cast<double>(duration.count()) / total_accesses;
        auto accesses_per_second = 1e9 / avg_ns;

        std::cout << "  Sequential access: " << avg_ns << " ns/access, " << accesses_per_second << " accesses/sec"
                  << std::endl;
    }

    static void benchmark_iterator_traversal()
    {
        std::cout << "Benchmarking iterator traversal..." << std::endl;

        T buffer(10'000);

        // Fill buffer
        for (int i = 0; i < 10'000; ++i)
        {
            buffer.push_back(i);
        }

        // Warmup
        for (size_t i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            for (auto const& value: buffer)
            {
                int volatile v =
                    value; // NOSONAR S3687: volatile prevents the optimizer from eliding the iterator access.

                (void)v;
            }
        }

        // Measure
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < MEASURE_ITERATIONS; ++i)
        {
            for (auto const& value: buffer)
            {

                int volatile v =
                    value; // NOSONAR S3687: volatile prevents the optimizer from eliding the iterator access.
                (void)v;
            }
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        auto total_accesses      = static_cast<double>(MEASURE_ITERATIONS * buffer.size());
        auto avg_ns              = static_cast<double>(duration.count()) / total_accesses;
        auto accesses_per_second = 1e9 / avg_ns;

        std::cout << "  Iterator traversal: " << avg_ns << " ns/access, " << accesses_per_second << " accesses/sec"
                  << std::endl;
    }

    static void benchmark_emplace_operations()
    {
        std::cout << "Benchmarking emplace operations..." << std::endl;

        T buffer(1'000);

        // Warmup
        for (size_t i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            buffer.push_back(static_cast<int>(i));
            if (buffer.size() > 500)
            {
                buffer.pop_front();
            }
        }

        // Measure
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < MEASURE_ITERATIONS; ++i)
        {
            buffer.push_back(static_cast<int>(i % 100));
            if (buffer.size() > 500)
            {
                buffer.pop_front();
            }
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        double avg_ns         = static_cast<double>(duration.count()) / MEASURE_ITERATIONS;
        double ops_per_second = 1e9 / avg_ns;

        std::cout << "  Emplace operations: " << avg_ns << " ns/op, " << ops_per_second << " ops/sec" << std::endl;
    }

    static void benchmark_memory_efficiency()
    {
        std::cout << "Benchmarking memory efficiency..." << std::endl;

        size_t const capacity = 1'000'000;
        T            buffer(capacity);

        // Measure memory usage by filling buffer
        auto start_time = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < capacity; ++i)
        {
            buffer.push_back(static_cast<int>(i));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // Calculate memory efficiency
        size_t memory_used = capacity * sizeof(int);
        double fill_rate   = static_cast<double>(capacity) / static_cast<double>(duration.count()) * 1000000.0;

        std::cout << "  Memory efficiency: " << static_cast<double>(memory_used) / 1024.0 / 1024.0 << " MB for "
                  << capacity << " elements" << std::endl;
        std::cout << "  Fill rate: " << fill_rate << " elements/sec" << std::endl;
    }
};

template <typename T> class ThreadSafetyBenchmark
{
  private:
    static constexpr size_t WORKER_ITERATIONS = 50'000;
    static constexpr size_t BUFFER_CAPACITY   = 1'000;

  public:
    static void benchmark_producer_consumer()
    {
        std::cout << "Benchmarking producer-consumer scenario..." << std::endl;

        T           buffer(BUFFER_CAPACITY);
        std::atomic stop_flag{false};
        std::atomic produced{0UL};
        std::atomic consumed{0UL};

        // Producer function
        auto producer = [&buffer, &produced]() {
            std::random_device            rd;
            std::mt19937                  gen(rd());
            std::uniform_int_distribution dis(1, 100);

            for (size_t i = 0; i < WORKER_ITERATIONS; ++i)
            {
                buffer.push_back(dis(gen));
                produced++;
                std::this_thread::sleep_for(std::chrono::nanoseconds(100));
            }
        };

        // Consumer function
        auto consumer = [&buffer, &consumed, &stop_flag]() {
            while (!stop_flag)
            {
                if (auto item = buffer.try_pop_front(); item.has_value())
                {
                    consumed++;
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds(200));
            }
        };

        // Start threads
        std::thread producer_thread(producer);
        std::thread consumer_thread(consumer);

        // Wait for producer to finish
        producer_thread.join();

        // Wait a bit more for consumer to catch up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        stop_flag = true;
        consumer_thread.join();

        std::cout << "  Produced: " << produced.load() << " items" << std::endl;
        std::cout << "  Consumed: " << consumed.load() << " items" << std::endl;
        std::cout << "  Throughput: " << static_cast<double>(consumed.load()) / 0.1 << " items/sec" << std::endl;
    }

    static void benchmark_multiple_threads()
    {
        std::cout << "Benchmarking multiple producer-consumer threads..." << std::endl;

        int const num_threads = 4;
        T         buffer(BUFFER_CAPACITY * num_threads);

        std::atomic stop_flag{false};
        std::atomic total_produced{0UL};
        std::atomic total_consumed{0UL};

        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;

        // Create producer threads
        for (int t = 0; t < num_threads; ++t)
        {
            producers.emplace_back([&buffer, &total_produced, t]() {
                std::random_device            rd;
                std::mt19937                  gen(rd() + t);
                std::uniform_int_distribution dis(1, 100);

                for (size_t i = 0; i < WORKER_ITERATIONS / num_threads; ++i)
                {
                    buffer.push_back(dis(gen) + t * 1'000);
                    total_produced++;
                    std::this_thread::sleep_for(std::chrono::nanoseconds(50));
                }
            });
        }

        // Create consumer threads
        for (int t = 0; t < num_threads; ++t)
        {
            consumers.emplace_back([&buffer, &total_consumed, &stop_flag]() {
                while (!stop_flag)
                {
                    if (auto item = buffer.try_pop_front(); item.has_value())
                    {
                        total_consumed++;
                    }
                    std::this_thread::sleep_for(std::chrono::nanoseconds(100));
                }
            });
        }

        // Wait for producers to finish
        for (auto& t: producers)
        {
            t.join();
        }

        // Wait a bit more for consumers to catch up
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        stop_flag = true;
        for (auto& t: consumers)
        {
            t.join();
        }

        std::cout << "  Total produced: " << total_produced.load() << " items" << std::endl;
        std::cout << "  Total consumed: " << total_consumed.load() << " items" << std::endl;
        std::cout << "  Multi-threaded throughput: " << static_cast<double>(total_consumed.load()) / 0.2 << " items/sec"
                  << std::endl;
    }
};

int main()
{
    std::cout << "Ring Buffer Performance Benchmarks" << std::endl;
    std::cout << "====================================" << std::endl;

    std::cout << "\n--- Single-Threaded Ring Buffer Benchmarks ---" << std::endl;
    BenchmarkSuite<ring_buffer<int>>::benchmark_push_pop_operations();
    BenchmarkSuite<ring_buffer<int>>::benchmark_sequential_access();
    BenchmarkSuite<ring_buffer<int>>::benchmark_iterator_traversal();
    BenchmarkSuite<ring_buffer<int>>::benchmark_emplace_operations();
    BenchmarkSuite<ring_buffer<int>>::benchmark_memory_efficiency();

    std::cout << "\n--- Thread-Safe Ring Buffer Benchmarks ---" << std::endl;
    ThreadSafetyBenchmark<ring_buffer_thread_safe<int>>::benchmark_producer_consumer();
    ThreadSafetyBenchmark<ring_buffer_thread_safe<int>>::benchmark_multiple_threads();

    std::cout << "\n--- Comparison: Single vs Thread-Safe ---" << std::endl;

    // Compare performance overhead of thread safety
    {
        std::cout << "Single-threaded performance comparison:" << std::endl;

        ring_buffer<int>             single_buffer(1'000);
        ring_buffer_thread_safe<int> thread_buffer(1'000);

        size_t const iterations = 100'000;

        // Measure single-threaded ring buffer
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < iterations; ++i)
        {
            single_buffer.push_back(static_cast<int>(i));
            if (single_buffer.size() > 500)
            {
                single_buffer.pop_front();
            }
        }
        auto end             = std::chrono::high_resolution_clock::now();
        auto single_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        // Measure thread-safe ring buffer (single-threaded usage)
        start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < iterations; ++i)
        {
            thread_buffer.push_back(static_cast<int>(i));
            if (thread_buffer.size() > 500)
            {
                thread_buffer.pop_front();
            }
        }
        end                  = std::chrono::high_resolution_clock::now();
        auto thread_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        double overhead = static_cast<double>(thread_duration.count() - single_duration.count()) /
                          static_cast<double>(single_duration.count()) * 100.0;

        std::cout << "  Single-threaded buffer: " << single_duration.count() << " ns" << std::endl;
        std::cout << "  Thread-safe buffer: " << thread_duration.count() << " ns" << std::endl;
        std::cout << "  Overhead: " << overhead << "%" << std::endl;
    }

    std::cout << "\n====================================" << std::endl;
    std::cout << "Benchmarks completed!" << std::endl;

    return 0;
}
