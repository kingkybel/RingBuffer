/*
 * Repository:  https://github.com/kingkybel/RingBuffer
 * File Name:   examples/producer_consumer_example.cc
 * Description: Example demonstrating producer-consumer pattern with RingBuffer.
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

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// Include the thread-safe ring buffer implementation
#include "../include/thread_safe_ring_buffer.h"

using namespace ringbuffer;

/**
 * @brief Producer-Consumer example demonstrating multi-threaded data processing
 *
 * This example shows how a ring buffer can be used to implement a producer-consumer
 * pattern where multiple producer threads generate data and multiple consumer
 * threads process it efficiently.
 */
struct DataPacket
{
    int                                   id;
    double                                value;
    std::chrono::steady_clock::time_point timestamp;

    DataPacket(int id = 0, double value = 0.0)
        : id(id)
        , value(value)
        , timestamp(std::chrono::steady_clock::now())
    {
    }
};

class ProducerConsumerExample
{
  private:
    static constexpr size_t BUFFER_CAPACITY    = 1'000;
    static constexpr int    NUM_PRODUCERS      = 3;
    static constexpr int    NUM_CONSUMERS      = 2;
    static constexpr int    ITEMS_PER_PRODUCER = 1'000;

    ThreadSafeRingBuffer<DataPacket> buffer_;
    std::atomic<bool>                stop_flag_{false};
    std::atomic<int>                 total_produced_{0};
    std::atomic<int>                 total_consumed_{0};
    std::atomic<long long>           processing_time_ns_{0};

  public:
    ProducerConsumerExample()
        : buffer_(BUFFER_CAPACITY)
    {
    }

    /**
     * @brief Producer function that generates data packets
     */
    void producer(int producer_id)
    {
        std::random_device                     rd;
        std::mt19937                           gen(rd() + producer_id);
        std::uniform_real_distribution<double> value_dist(0.0, 100.0);
        std::uniform_int_distribution<int>     delay_dist(1, 10);

        for (int i = 0; i < ITEMS_PER_PRODUCER; ++i)
        {
            // Generate data packet
            int        packet_id = producer_id * ITEMS_PER_PRODUCER + i;
            double     value     = value_dist(gen);
            DataPacket packet(packet_id, value);

            // Try to add to buffer with timeout
            bool added = buffer_.push_back_with_timeout(packet, std::chrono::milliseconds(100));

            if (added)
            {
                total_produced_++;
                if (total_produced_ % 200 == 0)
                {
                    std::cout << "Producer " << producer_id << " produced " << total_produced_.load() << " items"
                              << std::endl;
                }
            }
            else
            {
                std::cout << "Producer " << producer_id << " timeout - buffer full!" << std::endl;
                i--; // Retry this item
            }

            // Simulate variable production time
            std::this_thread::sleep_for(std::chrono::microseconds(delay_dist(gen) * 100));
        }

        std::cout << "Producer " << producer_id << " finished" << std::endl;
    }

    /**
     * @brief Consumer function that processes data packets
     */
    void consumer(int consumer_id)
    {
        std::random_device                 rd;
        std::mt19937                       gen(rd() + consumer_id + 100);
        std::uniform_int_distribution<int> delay_dist(1, 5);

        while (!stop_flag_)
        {
            // Try to get item with timeout
            auto packet_opt = buffer_.pop_front_with_timeout(std::chrono::milliseconds(50));

            if (packet_opt.has_value())
            {
                DataPacket const& packet = *packet_opt;

                // Simulate processing time
                auto start_time = std::chrono::steady_clock::now();

                // Simple processing: calculate some derived values
                [[maybe_unused]]double processed_value = packet.value * 1.5 + 10.0;
                [[maybe_unused]]int    checksum        = packet.id ^ static_cast<int>(packet.value);

                // Simulate variable processing time
                std::this_thread::sleep_for(std::chrono::microseconds(delay_dist(gen) * 200));

                auto end_time        = std::chrono::steady_clock::now();
                auto processing_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

                processing_time_ns_ += processing_time.count();
                total_consumed_++;

                if (total_consumed_ % 100 == 0)
                {
                    std::cout << "Consumer " << consumer_id << " processed item " << total_consumed_.load()
                              << " (ID: " << packet.id << ", Value: " << std::fixed << std::setprecision(2)
                              << packet.value << ")" << std::endl;
                }
            }
            else
            {
                // No data available, could do other work here
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        std::cout << "Consumer " << consumer_id << " finished" << std::endl;
    }

    /**
     * @brief Monitor buffer status and system performance
     */
    void monitor_system()
    {
        auto const start_time = std::chrono::steady_clock::now();

        while (total_consumed_ < NUM_PRODUCERS * ITEMS_PER_PRODUCER)
        {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed      = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

            size_t buffer_size     = buffer_.size();
            size_t buffer_capacity = buffer_.capacity();
            float  utilization     = static_cast<float>(buffer_size) / buffer_capacity * 100.0f;

            int produced  = total_produced_.load();
            int consumed  = total_consumed_.load();
            int in_flight = produced - consumed;

            std::cout << "Time: " << elapsed << "s | " << "Buffer: " << buffer_size << "/" << buffer_capacity << " ("
                      << std::fixed << std::setprecision(1) << utilization << "%) | " << "Produced: " << produced
                      << " | " << "Consumed: " << consumed << " | " << "In-flight: " << in_flight << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    /**
     * @brief Run the producer-consumer simulation
     */
    void run_simulation()
    {
        std::cout << "=== Producer-Consumer Ring Buffer Simulation ===" << std::endl;
        std::cout << "Buffer Capacity: " << BUFFER_CAPACITY << std::endl;
        std::cout << "Producers: " << NUM_PRODUCERS << std::endl;
        std::cout << "Consumers: " << NUM_CONSUMERS << std::endl;
        std::cout << "Items per Producer: " << ITEMS_PER_PRODUCER << std::endl;
        std::cout << "Total Items: " << NUM_PRODUCERS * ITEMS_PER_PRODUCER << std::endl;
        std::cout << "=================================================" << std::endl;

        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;

        auto start_time = std::chrono::steady_clock::now();

        // Start producer threads
        std::cout << "\nStarting producers..." << std::endl;
        for (int i = 0; i < NUM_PRODUCERS; ++i)
        {
            producers.emplace_back(&ProducerConsumerExample::producer, this, i);
        }

        // Start consumer threads
        std::cout << "Starting consumers..." << std::endl;
        for (int i = 0; i < NUM_CONSUMERS; ++i)
        {
            consumers.emplace_back(&ProducerConsumerExample::consumer, this, i);
        }

        // Start monitoring thread
        std::thread monitor_thread(&ProducerConsumerExample::monitor_system, this);

        // Wait for all producers to finish
        std::cout << "\nWaiting for producers to complete..." << std::endl;
        for (auto& producer: producers)
        {
            producer.join();
        }

        std::cout << "All producers finished. Waiting for consumers to drain buffer..." << std::endl;

        // Wait a bit more for consumers to finish processing
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Signal consumers to stop
        stop_flag_ = true;

        // Wait for all consumers to finish
        std::cout << "Waiting for consumers to complete..." << std::endl;
        for (auto& consumer: consumers)
        {
            consumer.join();
        }

        monitor_thread.join();

        auto end_time   = std::chrono::steady_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

        // Calculate statistics
        long long avg_processing_time = processing_time_ns_.load() / total_consumed_.load();
        double    throughput          = static_cast<double>(total_consumed_.load()) / total_time.count();

        std::cout << "\n=== Simulation Results ===" << std::endl;
        std::cout << "Total time: " << total_time.count() << " seconds" << std::endl;
        std::cout << "Total produced: " << total_produced_.load() << std::endl;
        std::cout << "Total consumed: " << total_consumed_.load() << std::endl;
        std::cout << "Average processing time: " << avg_processing_time << " ns" << std::endl;
        std::cout << "Overall throughput: " << throughput << " items/second" << std::endl;

        if (total_produced_.load() == total_consumed_.load())
        {
            std::cout << "✓ All items were successfully processed!" << std::endl;
        }
        else
        {
            std::cout << "⚠️  Some items were lost or not processed!" << std::endl;
        }
    }
};

/**
 * @brief Demonstrate priority-based producer-consumer
 */
void demonstrate_priority_processing()
{
    std::cout << "\n=== Priority-Based Processing Example ===" << std::endl;

    ThreadSafeRingBuffer<DataPacket> priority_buffer(100);
    std::atomic<bool>                priority_stop{false};
    std::atomic<int>                 high_priority_count{0};
    std::atomic<int>                 low_priority_count{0};

    // High priority producer
    std::thread high_priority_producer([&]() {
        for (int i = 0; i < 50; ++i)
        {
            DataPacket packet(i, 999.0); // High priority value
            priority_buffer.push_back(packet);
            high_priority_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // Low priority producer
    std::thread low_priority_producer([&]() {
        for (int i = 0; i < 50; ++i)
        {
            DataPacket packet(i + 1'000, 1.0); // Low priority value
            priority_buffer.push_back(packet);
            low_priority_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    // Consumer that prioritizes high-value items
    std::thread priority_consumer([&]() {
        int processed_high = 0;
        int processed_low  = 0;

        while (processed_high + processed_low < 100)
        {
            auto packet_opt = priority_buffer.pop_front_with_timeout(std::chrono::milliseconds(100));

            if (packet_opt.has_value())
            {
                DataPacket const& packet = *packet_opt;
                if (packet.value > 500.0)
                {
                    processed_high++;
                    std::cout << "Processed high-priority item: ID=" << packet.id << ", Value=" << packet.value
                              << std::endl;
                }
                else
                {
                    processed_low++;
                    std::cout << "Processed low-priority item: ID=" << packet.id << ", Value=" << packet.value
                              << std::endl;
                }
            }
        }

        std::cout << "Priority consumer processed: " << processed_high << " high-priority, " << processed_low
                  << " low-priority" << std::endl;
    });

    high_priority_producer.join();
    low_priority_producer.join();
    priority_consumer.join();

    std::cout << "Priority processing completed" << std::endl;
}

int main()
{
    std::cout << "Ring Buffer Producer-Consumer Examples" << std::endl;
    std::cout << "======================================" << std::endl;

    try
    {
        // Run the main producer-consumer simulation
        ProducerConsumerExample example;
        example.run_simulation();

        // Demonstrate priority processing
        demonstrate_priority_processing();

        std::cout << "\n======================================" << std::endl;
        std::cout << "Producer-consumer examples completed successfully!" << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
