/*
 * Repository:  https://github.com/kingkybel/RingBuffer
 * File Name:   examples/audio_buffer_example.cc
 * Description: Example demonstrating audio buffer usage with RingBuffer.
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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// Include the ring buffer implementation
#include "../include/ring_buffer.h"

using namespace ringbuffer;

/**
 * @brief Audio buffer example demonstrating real-time audio processing
 *
 * This example simulates a real-time audio processing system where audio samples
 * are continuously generated and processed. The ring buffer helps prevent audio
 * glitches by providing a buffer between the audio generation and processing.
 */
class AudioBufferExample
{
  private:
    static constexpr size_t BUFFER_SIZE = 1'024;  // Audio buffer capacity
    static constexpr size_t SAMPLE_RATE = 48'000; // Samples per second
    static constexpr size_t CHUNK_SIZE  = 512;    // Samples per chunk

    RingBuffer<float>                     audio_buffer_;
    std::mt19937                          rng_;
    std::uniform_real_distribution<float> dist_;

  public:
    AudioBufferExample()
        : audio_buffer_(BUFFER_SIZE)
        , rng_(std::random_device{}())
        , dist_(-1.0f, 1.0f)
    {
    }

    /**
     * @brief Simulate audio sample generation
     *
     * In a real audio system, this would be called by the audio hardware
     * interrupt at regular intervals (e.g., every 10ms).
     */
    void generate_audio_samples()
    {
        std::vector<float> samples(CHUNK_SIZE);

        // Generate some audio data (sine wave with noise)
        static float phase     = 0.0f;
        float const  frequency = 440.0f; // A4 note
        float const  two_pi    = 2.0f * 3.14159f;

        for (size_t i = 0; i < CHUNK_SIZE; ++i)
        {
            // Generate sine wave
            float sine = std::sin(phase);
            phase += two_pi * frequency / SAMPLE_RATE;

            // Add some noise
            float const noise = dist_(rng_) * 0.1f;

            samples[i] = sine + noise;
        }

        // Add samples to buffer
        for (float sample: samples)
        {
            audio_buffer_.push_back(sample);
        }

        std::cout << "Generated " << CHUNK_SIZE << " audio samples. Buffer: " << audio_buffer_.size() << "/"
                  << audio_buffer_.capacity() << std::endl;
    }

    /**
     * @brief Simulate audio processing
     *
     * In a real system, this would process audio samples for effects,
     * analysis, or playback.
     */
    void process_audio_samples()
    {
        if (audio_buffer_.empty())
        {
            std::cout << "No audio samples to process (buffer empty)" << std::endl;
            return;
        }

        // Process a chunk of samples
        size_t             samples_to_process = std::min(CHUNK_SIZE, audio_buffer_.size());
        std::vector<float> processed_samples;
        processed_samples.reserve(samples_to_process);

        for (size_t i = 0; i < samples_to_process; ++i)
        {
            float sample = audio_buffer_.pop_front();

            // Simple processing: apply gain and check for clipping
            float gain      = 0.8f;
            float processed = sample * gain;

            // Clamp to prevent clipping
            if (processed > 1.0f)
            {
                processed = 1.0f;
            }
            if (processed < -1.0f)
            {
                processed = -1.0f;
            }

            processed_samples.push_back(processed);
        }

        std::cout << "Processed " << samples_to_process << " audio samples. Buffer: " << audio_buffer_.size() << "/"
                  << audio_buffer_.capacity() << std::endl;

        // Calculate some statistics
        if (!processed_samples.empty())
        {
            float sum     = 0.0f;
            float max_abs = 0.0f;

            for (float sample: processed_samples)
            {
                sum += sample;
                float abs_sample = std::abs(sample);
                if (abs_sample > max_abs)
                {
                    max_abs = abs_sample;
                }
            }

            float const average = sum / static_cast<float>(processed_samples.size());

            std::cout << "  Average level: " << std::fixed << std::setprecision(3) << average
                      << ", Peak level: " << max_abs << std::endl;
        }
    }

    /**
     * @brief Simulate audio monitoring
     *
     * Shows buffer status and warns about potential issues.
     */
    void monitor_audio_buffer() const
    {
        size_t const current_size = audio_buffer_.size();
        size_t const capacity     = audio_buffer_.capacity();
        float const  utilization  = static_cast<float>(current_size) / static_cast<float>(capacity) * 100.0f;

        std::cout << "Audio Buffer Status: " << current_size << "/" << capacity << " (" << std::fixed
                  << std::setprecision(1) << utilization << "%)";

        if (utilization > 90.0f)
        {
            std::cout << " ⚠️  WARNING: Buffer nearly full!";
        }
        else if (utilization < 10.0f)
        {
            std::cout << " ⚠️  WARNING: Buffer nearly empty!";
        }
        else
        {
            std::cout << " ✓";
        }
        std::cout << std::endl;
    }

    /**
     * @brief Run the audio buffer simulation
     */
    void run_simulation()
    {
        std::cout << "=== Audio Buffer Simulation ===" << std::endl;
        std::cout << "Buffer Size: " << BUFFER_SIZE << " samples" << std::endl;
        std::cout << "Sample Rate: " << SAMPLE_RATE << " Hz" << std::endl;
        std::cout << "Chunk Size: " << CHUNK_SIZE << " samples" << std::endl;
        std::cout << "===============================" << std::endl;

        int const simulation_steps = 20;

        for (int step = 0; step < simulation_steps; ++step)
        {
            std::cout << "\nStep " << (step + 1) << "/" << simulation_steps << ":" << std::endl;

            // Simulate variable load by sometimes generating more/less audio
            int generation_factor = 1;
            if (step % 5 == 0)
            {
                generation_factor = 2; // Burst of audio
            }
            if (step % 7 == 0)
            {
                generation_factor = 0; // Silence period
            }

            for (int i = 0; i < generation_factor; ++i)
            {
                generate_audio_samples();
            }

            // Process audio (simulating variable processing speed)
            int processing_factor = 1;
            if (step % 3 == 0)
            {
                processing_factor = 2; // Fast processing
            }
            if (step % 4 == 0)
            {
                processing_factor = 0; // Slow processing
            }

            for (int i = 0; i < processing_factor; ++i)
            {
                process_audio_samples();
            }

            monitor_audio_buffer();

            // Simulate real-time timing
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "\n=== Simulation Complete ===" << std::endl;
        std::cout << "Final buffer state: " << audio_buffer_.size() << "/" << audio_buffer_.capacity() << std::endl;
    }
};

/**
 * @brief Demonstrate audio analysis using ring buffer
 */
void demonstrate_audio_analysis()
{
    std::cout << "\n=== Audio Analysis Example ===" << std::endl;

    RingBuffer<float> analysis_buffer(256);

    // Simulate audio analysis over time
    std::mt19937                    rng(std::random_device{}());
    std::normal_distribution<float> dist(0.0f, 0.5f);

    for (int i = 0; i < 10; ++i)
    {
        // Add new audio data
        for (int j = 0; j < 32; ++j)
        {
            analysis_buffer.push_back(dist(rng));
        }

        // Analyze current buffer contents
        float  sum    = 0.0f;
        float  sum_sq = 0.0f;
        size_t count  = 0;

        for (auto const& sample: analysis_buffer)
        {
            sum += sample;
            sum_sq += sample * sample;
            count++;
        }

        if (count > 0)
        {
            float const mean     = sum / static_cast<float>(count);
            float       variance = (sum_sq / static_cast<float>(count)) - (mean * mean);
            float const rms      = std::sqrt(std::max(0.0f, variance));

            std::cout << "Analysis Window " << (i + 1) << ": " << "Mean=" << std::fixed << std::setprecision(3) << mean
                      << ", RMS=" << rms << ", Buffer=" << analysis_buffer.size() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main()
{
    std::cout << "Ring Buffer Audio Processing Examples" << std::endl;
    std::cout << "=====================================" << std::endl;

    try
    {
        // Run the main audio buffer simulation
        AudioBufferExample audio_example;
        audio_example.run_simulation();

        // Demonstrate audio analysis
        demonstrate_audio_analysis();

        std::cout << "\n=====================================" << std::endl;
        std::cout << "Audio examples completed successfully!" << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
