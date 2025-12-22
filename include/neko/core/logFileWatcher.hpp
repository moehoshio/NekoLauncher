/**
 * @file logFileWatcher.hpp
 * @brief Log file watcher for monitoring Minecraft log files in real-time
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include <neko/schema/types.hpp>

#include <functional>
#include <memory>
#include <string>

namespace neko::core {

    /**
     * @class LogFileWatcher
     * @brief Monitors a log file for new lines and publishes events.
     * 
     * This class is designed to watch Minecraft's latest.log file in real-time,
     * allowing BGM triggers based on chat messages, game events, and server commands.
     * 
     * Usage:
     * @code
     * auto& watcher = getLogFileWatcher();
     * watcher.start("/path/to/.minecraft/logs/latest.log");
     * // Watcher will publish LogFileLineEvent for each new line
     * @endcode
     */
    class LogFileWatcher {
    public:
        LogFileWatcher();
        ~LogFileWatcher();

        LogFileWatcher(const LogFileWatcher&) = delete;
        LogFileWatcher& operator=(const LogFileWatcher&) = delete;
        LogFileWatcher(LogFileWatcher&&) noexcept;
        LogFileWatcher& operator=(LogFileWatcher&&) noexcept;

        /**
         * @brief Start watching a log file.
         * @param logFilePath Absolute path to the log file (e.g., .minecraft/logs/latest.log)
         * @param fromEnd If true, start reading from the end of file (ignore existing content)
         * @return true if watching started successfully
         */
        bool start(const std::string& logFilePath, bool fromEnd = true);

        /**
         * @brief Stop watching the log file.
         */
        void stop();

        /**
         * @brief Check if the watcher is currently active.
         * @return true if watching a file
         */
        bool isWatching() const;

        /**
         * @brief Get the path of the currently watched file.
         * @return Path to the log file, or empty string if not watching
         */
        std::string getWatchedPath() const;

        /**
         * @brief Set the polling interval for checking file changes.
         * @param intervalMs Interval in milliseconds (default: 100ms)
         */
        void setPollingInterval(neko::uint32 intervalMs);

        /**
         * @brief Set a callback for each new line.
         * @param callback Function to call for each new line read
         * @note This is in addition to publishing LogFileLineEvent
         */
        void setLineCallback(std::function<void(const std::string&)> callback);

    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };

    /**
     * @brief Get the singleton LogFileWatcher instance.
     * @return Reference to the global LogFileWatcher
     */
    LogFileWatcher& getLogFileWatcher();

    /**
     * @brief Subscribe the LogFileWatcher to process events.
     * 
     * This function sets up automatic log file watching when a Minecraft process starts.
     * It uses the working directory to find the logs/latest.log file.
     */
    void subscribeLogWatcherToProcessEvents();

} // namespace neko::core
