/*
 * Repository:  https://github.com/kingkybel/RingBuffer
 * File Name:   examples/logging_example.cc
 * Description: Example demonstrating circular logging with RingBuffer.
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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// Include the ring buffer implementation
#include "../include/ring_buffer.h"

using namespace ringbuffer;

/**
 * @brief Log entry structure for the circular logging system
 */
struct LogEntry
{
    enum class Level
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    std::chrono::steady_clock::time_point timestamp;
    Level                                 level;
    std::string                           message;
    std::string                           source;

    LogEntry(Level level, std::string const& message, std::string const& source = "")
        : timestamp(std::chrono::steady_clock::now())
        , level(level)
        , message(message)
        , source(source)
    {
    }

    std::string level_to_string() const
    {
        switch (level)
        {
            case Level::DEBUG:
                return "DEBUG";
            case Level::INFO:
                return "INFO";
            case Level::WARNING:
                return "WARNING";
            case Level::ERROR:
                return "ERROR";
            case Level::CRITICAL:
                return "CRITICAL";
            default:
                return "UNKNOWN";
        }
    }

    std::string to_string() const
    {
        auto time_point = std::chrono::time_point_cast<std::chrono::seconds>(timestamp);
        auto time_t     = std::chrono::system_clock::to_time_t(
            std::chrono::time_point<std::chrono::system_clock>(time_point.time_since_epoch())
        );

        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << " [" << level_to_string() << "]";
        if (!source.empty())
        {
            oss << " [" << source << "]";
        }
        oss << " " << message;

        return oss.str();
    }
};

/**
 * @brief Circular logging system using ring buffer
 *
 * This example demonstrates how a ring buffer can be used to implement
 * an efficient circular logging system that automatically overwrites
 * old log entries when the buffer is full.
 */
class CircularLogger
{
  private:
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1'000;

    RingBuffer<LogEntry> log_buffer_;
    std::atomic<bool>    enabled_{true};
    std::string          log_file_;
    bool                 write_to_file_;

    std::string level_color(LogEntry::Level level) const
    {
        switch (level)
        {
            case LogEntry::Level::DEBUG:
                return "\033[36m"; // Cyan
            case LogEntry::Level::INFO:
                return "\033[32m"; // Green
            case LogEntry::Level::WARNING:
                return "\033[33m"; // Yellow
            case LogEntry::Level::ERROR:
                return "\033[31m"; // Red
            case LogEntry::Level::CRITICAL:
                return "\033[35m"; // Magenta
            default:
                return "\033[0m"; // Reset
        }
    }

    std::string reset_color() const
    {
        return "\033[0m";
    }

  public:
    explicit CircularLogger(
        size_t buffer_size          = DEFAULT_BUFFER_SIZE,
        std::string const& log_file = "",
        bool write_to_file          = false
    )
        : log_buffer_(buffer_size)
        , log_file_(log_file)
        , write_to_file_(write_to_file)
    {
    }

    auto capacity() const
    {
        return log_buffer_.capacity();
    }

    /**
     * @brief Log a message with specified level
     */
    void log(LogEntry::Level level, std::string const& message, std::string const& source = "")
    {
        if (!enabled_)
        {
            return;
        }

        LogEntry entry(level, message, source);
        log_buffer_.push_back(entry);

        // Output to console with color coding
        std::string color = level_color(level);
        std::string reset = reset_color();

        std::cout << color << entry.to_string() << reset << std::endl;

        // Write to file if enabled
        if (write_to_file_ && !log_file_.empty())
        {
            std::ofstream file(log_file_, std::ios::app);
            if (file.is_open())
            {
                file << entry.to_string() << std::endl;
                file.close();
            }
        }
    }

    /**
     * @brief Convenience methods for different log levels
     */
    void debug(std::string const& message, std::string const& source = "")
    {
        log(LogEntry::Level::DEBUG, message, source);
    }

    void info(std::string const& message, std::string const& source = "")
    {
        log(LogEntry::Level::INFO, message, source);
    }

    void warning(std::string const& message, std::string const& source = "")
    {
        log(LogEntry::Level::WARNING, message, source);
    }

    void error(std::string const& message, std::string const& source = "")
    {
        log(LogEntry::Level::ERROR, message, source);
    }

    void critical(std::string const& message, std::string const& source = "")
    {
        log(LogEntry::Level::CRITICAL, message, source);
    }

    /**
     * @brief Get current buffer statistics
     */
    void print_status() const
    {
        std::cout << "\n=== Logger Status ===" << std::endl;
        std::cout << "Buffer Size: " << log_buffer_.size() << "/" << log_buffer_.capacity() << std::endl;
        std::cout << "Buffer Full: " << (log_buffer_.full() ? "Yes" : "No") << std::endl;
        std::cout << "Buffer Empty: " << (log_buffer_.empty() ? "Yes" : "No") << std::endl;
        std::cout << "Available Slots: " << log_buffer_.available() << std::endl;
        std::cout << "File Output: " << (write_to_file_ ? log_file_ : "Disabled") << std::endl;
        std::cout << "=====================" << std::endl;
    }

    /**
     * @brief Dump all current log entries
     */
    void dump_logs() const
    {
        std::cout << "\n=== Current Log Entries ===" << std::endl;

        if (log_buffer_.empty())
        {
            std::cout << "No log entries available." << std::endl;
            return;
        }

        size_t count = 0;
        for (auto const& entry: log_buffer_)
        {
            std::cout << "[" << count++ << "] " << entry.to_string() << std::endl;
        }

        std::cout << "==========================" << std::endl;
    }

    /**
     * @brief Filter and display logs by level
     */
    void dump_logs_by_level(LogEntry::Level level) const
    {
        std::cout << "\n=== Logs at Level: " << LogEntry(level, "").level_to_string() << " ===" << std::endl;

        size_t count = 0;
        for (auto const& entry: log_buffer_)
        {
            if (entry.level == level)
            {
                std::cout << "[" << count++ << "] " << entry.to_string() << std::endl;
            }
        }

        if (count == 0)
        {
            std::cout << "No entries found at this level." << std::endl;
        }

        std::cout << "============================" << std::endl;
    }

    /**
     * @brief Clear all log entries
     */
    void clear()
    {
        log_buffer_.clear();
        std::cout << "All log entries cleared." << std::endl;
    }

    /**
     * @brief Disable/enable logging
     */
    void set_enabled(bool enabled)
    {
        enabled_ = enabled;
        std::cout << "Logging " << (enabled ? "enabled" : "disabled") << std::endl;
    }

    /**
     * @brief Get the oldest log entry
     */
    std::string get_oldest_entry() const
    {
        if (log_buffer_.empty())
        {
            return "No entries available";
        }
        return log_buffer_.front().to_string();
    }

    /**
     * @brief Get the newest log entry
     */
    std::string get_newest_entry() const
    {
        if (log_buffer_.empty())
        {
            return "No entries available";
        }
        return log_buffer_.back().to_string();
    }
};

/**
 * @brief Simulate a system component that generates logs
 */
class SystemComponent
{
  private:
    CircularLogger& logger_;
    std::string     name_;
    std::mt19937    rng_;

  public:
    SystemComponent(CircularLogger& logger, std::string const& name)
        : logger_(logger)
        , name_(name)
        , rng_(std::random_device{}())
    {
    }

    void simulate_operation()
    {
        std::uniform_int_distribution<int> level_dist(0, 4);
        std::uniform_int_distribution<int> delay_dist(100, 1'000);

        // Simulate different types of log messages
        std::vector<std::string> messages = {
            "System initialization complete",
            "Configuration loaded successfully",
            "Network connection established",
            "Processing batch job",
            "Memory usage at 75%",
            "Warning: High CPU usage detected",
            "Error: Failed to connect to database",
            "Critical: System overload detected",
            "Debug: Variable state updated",
            "Info: User action logged"
        };

        LogEntry::Level levels[] = {
            LogEntry::Level::INFO,
            LogEntry::Level::INFO,
            LogEntry::Level::INFO,
            LogEntry::Level::DEBUG,
            LogEntry::Level::INFO,
            LogEntry::Level::WARNING,
            LogEntry::Level::ERROR,
            LogEntry::Level::CRITICAL,
            LogEntry::Level::DEBUG,
            LogEntry::Level::INFO
        };

        int             message_index = rng_() % messages.size();
        LogEntry::Level level         = levels[message_index];
        std::string     message       = messages[message_index];

        logger_.log(level, message, name_);

        // Simulate processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_dist(rng_)));
    }
};

/**
 * @brief Demonstrate advanced logging features
 */
void demonstrate_advanced_logging()
{
    std::cout << "\n=== Advanced Logging Features ===" << std::endl;

    CircularLogger advanced_logger(50, "advanced_log.txt", true);

    // Simulate different system components
    SystemComponent component1(advanced_logger, "Network");
    SystemComponent component2(advanced_logger, "Database");
    SystemComponent component3(advanced_logger, "UI");

    std::cout << "Simulating system operations..." << std::endl;

    for (int i = 0; i < 20; ++i)
    {
        component1.simulate_operation();
        component2.simulate_operation();
        component3.simulate_operation();

        if (i % 5 == 0)
        {
            advanced_logger.print_status();
        }
    }

    // Demonstrate filtering
    advanced_logger.dump_logs_by_level(LogEntry::Level::ERROR);
    advanced_logger.dump_logs_by_level(LogEntry::Level::WARNING);

    // Show oldest and newest entries
    std::cout << "\nOldest entry: " << advanced_logger.get_oldest_entry() << std::endl;
    std::cout << "Newest entry: " << advanced_logger.get_newest_entry() << std::endl;

    std::cout << "Advanced logging demonstration completed." << std::endl;
}

int main()
{
    std::cout << "Ring Buffer Circular Logging Examples" << std::endl;
    std::cout << "=====================================" << std::endl;

    try
    {
        // Create a circular logger with a small buffer to demonstrate overwriting
        CircularLogger logger(10, "example_log.txt", true);

        std::cout << "Starting circular logging demonstration..." << std::endl;
        std::cout << "Buffer size: " << logger.capacity() << std::endl;

        // Demonstrate basic logging
        logger.info("Application started", "Main");
        logger.debug("Initializing components", "Main");
        logger.warning("Low disk space detected", "System");
        logger.error("Failed to load configuration", "Config");
        logger.critical("System shutdown required", "System");

        logger.print_status();
        logger.dump_logs();

        // Fill the buffer to demonstrate circular behavior
        std::cout << "\nFilling buffer to demonstrate circular behavior..." << std::endl;

        for (int i = 0; i < 15; ++i)
        {
            logger.info("Log entry " + std::to_string(i), "Test");
            if (i == 5)
            {
                std::cout << "Buffer status after 5 entries:" << std::endl;
                logger.print_status();
            }
        }

        logger.print_status();
        logger.dump_logs();

        // Demonstrate filtering
        logger.dump_logs_by_level(LogEntry::Level::INFO);

        // Demonstrate advanced features
        demonstrate_advanced_logging();

        std::cout << "\n=====================================" << std::endl;
        std::cout << "Circular logging examples completed successfully!" << std::endl;
        std::cout << "Log files created: example_log.txt, advanced_log.txt" << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
