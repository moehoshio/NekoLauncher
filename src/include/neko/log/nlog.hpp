/**
 * @file nlog.hpp
 * @brief neko logging system
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include "neko/schema/exception.hpp"
#include "neko/schema/srcloc.hpp"
#include "neko/schema/types.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace neko::log {

    /**
     * @brief Log level enum
     */
    enum class Level : neko::uint8 {
        Debug = 1, ///< Debug level, debug information
        Info = 2,  ///< Info level, general information
        Warn = 3,  ///< Warning level, potential issues
        Error = 4, ///< Error level, error information
        Off = 5    ///< Logging off
    };

    /**
     * @brief Convert log level to string
     */
    constexpr neko::cstr levelToString(Level lv) noexcept {
        switch (lv) {
            case Level::Debug:
                return "Debug";
            case Level::Info:
                return "Info";
            case Level::Warn:
                return "Warn";
            case Level::Error:
                return "Error";
            case Level::Off:
                return "Off";
            default:
                return "Unknown";
        }
    }

    /**
     * @brief Thread name manager
     */
    class ThreadNameManager {
    private:
        static std::unordered_map<std::thread::id, std::string> threadNames;
        static std::mutex namesMutex;

    public:
        /**
         * @brief Set the current thread's name
         */
        static void setCurrentThreadName(const std::string &name) {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames[std::this_thread::get_id()] = name;
        }

        /**
         * @brief Set the name of the specified thread
         */
        static void setThreadName(std::thread::id threadId, const std::string &name) {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames[threadId] = name;
        }

        /**
         * @brief Get thread name, returns thread ID string if not set
         */
        static std::string getThreadName(std::thread::id threadId) {
            std::lock_guard<std::mutex> lock(namesMutex);
            auto it = threadNames.find(threadId);
            if (it != threadNames.end()) {
                return it->second;
            }

            // Returns thread ID as string
            std::ostringstream oss;
            oss << threadId;
            return oss.str();
        }

        /**
         * @brief Remove thread name
         */
        static void removeThreadName(std::thread::id threadId) {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames.erase(threadId);
        }

        /**
         * @brief Clear all thread names
         */
        static void clearAllNames() {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames.clear();
        }
    };

    // Static member definitions
    inline std::unordered_map<std::thread::id, std::string> ThreadNameManager::threadNames;
    inline std::mutex ThreadNameManager::namesMutex;

    /**
     * @brief Log record structure
     */
    struct LogRecord {
        Level level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        neko::srcLocInfo location;
        std::string threadName;

        LogRecord(Level lvl, std::string msg, const neko::srcLocInfo &loc = {})
            : level(lvl), message(std::move(msg)),
              timestamp(std::chrono::system_clock::now()), location(loc) {
            threadName = ThreadNameManager::getThreadName(std::this_thread::get_id());
        }
    };

    /**
     * @brief Log formatter interface
     */
    class IFormatter {
    public:
        virtual ~IFormatter() = default;
        virtual std::string format(const LogRecord &record) = 0;
    };

    /**
     * @brief Default log formatter
     */
    class DefaultFormatter : public IFormatter {
    public:
        std::string format(const LogRecord &record) override {
            auto time_t = std::chrono::system_clock::to_time_t(record.timestamp);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          record.timestamp.time_since_epoch()) %
                      1000;

            std::tm tm;
#ifdef _WIN32
            localtime_s(&tm, &time_t);
#else
            localtime_r(&time_t, &tm);
#endif

            return std::format("[{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}] [{}] [{}] [{}:{}] {}",
                               tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                               tm.tm_hour, tm.tm_min, tm.tm_sec, ms.count(),
                               levelToString(record.level),
                               record.threadName,
                               record.location.getFile(), record.location.getLine(),
                               record.message);
        }
    };

    /**
     * @brief Log appender interface
     */
    class IAppender {
    public:
        virtual ~IAppender() = default;
        virtual void append(const LogRecord &record) = 0;
        virtual void flush() {}
    };

    /**
     * @brief Console appender
     */
    class ConsoleAppender : public IAppender {
    private:
        std::unique_ptr<IFormatter> formatter;
        std::mutex mutex;
        constexpr neko::strview
            red = "\033[31m",
            green = "\033[32m",
            yellow = "\033[33m",
            blue = "\033[34m",
            magenta = "\033[35m",
            cyan = "\033[36m",
            reset = "\033[0m";

    public:
        explicit ConsoleAppender(std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>())
            : formatter(std::move(formatter)) {}

        void append(const LogRecord &record) override {
            std::lock_guard<std::mutex> lock(mutex);
            auto formatted = formatter->format(record);

            switch (record.level) {
                case Level::Debug:
                    std::cout << blue << formatted << reset << std::endl;
                    break;
                case Level::Info:
                    std::cout << reset << formatted << reset << std::endl;
                    break;
                case Level::Warn:
                    std::cout << yellow << formatted << reset << std::endl;
                    break;
                case Level::Error:
                    std::cerr << red << formatted << reset << std::endl;
                    break;
                case Level::Off:
                    break;
                default:
                    std::cout << formatted << std::endl;
                    break;
            }
        }

        void flush() override {
            std::lock_guard<std::mutex> lock(mutex);
            std::cout.flush();
            std::cerr.flush();
        }
    };

    /**
     * @brief File appender
     */
    class FileAppender : public IAppender {
    private:
        std::unique_ptr<IFormatter> formatter;
        std::ofstream file;
        std::mutex mutex;

    public:
        explicit FileAppender(const std::string &filename,
                              std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>())
            : formatter(std::move(formatter)), file(filename, std::ios::app) {
            if (!file.is_open()) {
                throw neko::ex::FileError("Failed to open log file: " + filename);
            }
        }

        void append(const LogRecord &record) override {
            std::lock_guard<std::mutex> lock(mutex);
            if (file.is_open()) {
                file << formatter->format(record) << std::endl;
            }
        }

        void flush() override {
            std::lock_guard<std::mutex> lock(mutex);
            if (file.is_open()) {
                file.flush();
            }
        }

        ~FileAppender() {
            if (file.is_open()) {
                file.close();
            }
        }
    };

    /**
     * @brief Main Logger class
     */
    class Logger {
    private:
        Level level;
        neko::SyncMode mode = neko::SyncMode::Async;
        std::vector<std::unique_ptr<IAppender>> appenders;
        mutable std::mutex appenderMutex;

        // Queue for async logging
        std::queue<LogRecord> logQueue;
        std::condition_variable logQueueCondVar;
        mutable std::mutex logQueueMutex;

    public:
        explicit Logger() : level(Level::Info) {}

        explicit Logger(Level level) : level(level) {
            addAppender(std::make_unique<ConsoleAppender>());
        }

        explicit Logger(Level level, const std::string &filename) : level(level) {
            addAppender(std::make_unique<FileAppender>(filename));
        }

        // === Info ===

        Level getLevel() const {
            std::lock_guard<std::mutex> lock(appenderMutex);
            return level;
        }
        neko::SyncMode getMode() const {
            return mode;
        }

        bool isEnabled(Level level) const {
            std::lock_guard<std::mutex> lock(appenderMutex);
            return level >= this->level && this->level != Level::Off;
        }

        // === Control ===

        void setLevel(Level level) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            this->level = level;
        }

        void setMode(neko::SyncMode m) {
            this->mode = m;
        }

        void addFileAppender(const std::string &filename,
                             std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.push_back(std::make_unique<FileAppender>(filename, std::move(formatter)));
        }

        void addConsoleAppender(std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.push_back(std::make_unique<ConsoleAppender>(std::move(formatter)));
        }

        void addAppender(std::unique_ptr<IAppender> appender) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.push_back(std::move(appender));
        }

        void clearAppenders() {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.clear();
        }

        void append(const LogRecord &record) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            for (const auto &appender : appenders) {
                appender->append(record);
            }
        }

        void flush() {
            std::lock_guard<std::mutex> lock(appenderMutex);
            for (auto &appender : appenders) {
                appender->flush();
            }
        }

        /**
         * @brief Run the logging loop for async mode
         * @note This will block until the mode is set to Sync or the application exits.
         */
        void runLoop() {
            while (mode == neko::SyncMode::Async) {
                LogRecord record;
                {
                    std::lock_guard<std::mutex> lock(logQueueMutex);
                    logQueueCondVar.wait_for(lock, std::chrono::milliseconds(500), [this] {
                        return !logQueue.empty();
                    });
                    if (logQueue.empty()) {
                        continue;
                    }
                    record = logQueue.front();
                    logQueue.pop();
                }
                append(record);
            }

            // Flush remaining logs when stopping the loop
            std::lock_guard<std::mutex> lock(logQueueMutex);
            for (const auto &record : logQueue) {
                append(record);
            }
            flush();
        }

        /**
         * @brief Stop the logging loop
         * @note This will stop the async logging loop and flush any remaining logs.
         */
        void stopLoop() {
            if (mode != neko::SyncMode::Async) {
                return;
            }
            std::lock_guard<std::mutex> lock(logQueueMutex);
            mode = neko::SyncMode::Sync;
            logQueueCondVar.notify_all();
        }

        // === Logging ===

        void log(Level level, const std::string &message, const neko::srcLocInfo &location = {}) {
            if (!isEnabled(level)) {
                return;
            }

            LogRecord record(level, message, location);

            if (mode == neko::SyncMode::Sync) {
                std::lock_guard<std::mutex> lock(appenderMutex);
                for (const auto &appender : appenders) {
                    appender->append(record);
                }
                return;
            }

            std::lock_guard<std::mutex> lock(logQueueMutex);
            logQueue.push(record);
            logQueueCondVar.notify_one();
        }

        // === single message logging ===

        void debug(const std::string &message, const neko::srcLocInfo &location = {}) {
            log(Level::Debug, message, location);
        }

        void info(const std::string &message, const neko::srcLocInfo &location = {}) {
            log(Level::Info, message, location);
        }

        void warn(const std::string &message, const neko::srcLocInfo &location = {}) {
            log(Level::Warn, message, location);
        }

        void error(const std::string &message, const neko::srcLocInfo &location = {}) {
            log(Level::Error, message, location);
        }

        // === formatted message logging ===

        template <typename... Args>
        void debug(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Debug, message, location);
        }

        template <typename... Args>
        void info(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Info, message, location);
        }

        template <typename... Args>
        void warn(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Warn, message, location);
        }

        template <typename... Args>
        void error(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Error, message, location);
        }
    };

    /**
     * @brief Get global Logger instance
     */
    inline Logger &getGlobalLogger() {
        static Logger instance;
        return instance;
    }

    // === Convenience functions ===

    // === Info ===
    inline Level getLevel() const {
        return getGlobalLogger().getLevel();
    }
    inline neko::SyncMode getMode() const {
        return getGlobalLogger().getMode();
    }

    inline bool isEnabled(Level level) const {
        return getGlobalLogger().isEnabled(level);
    }

    // === Control ===

    inline void setLevel(Level level) {
        getGlobalLogger().setLevel(level);
    }

    inline void setMode(neko::SyncMode m) {
        getGlobalLogger().setMode(m);
    }

    inline void addFileAppender(const std::string &filename,
                                std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
        getGlobalLogger().addFileAppender(filename, std::move(formatter));
    }

    inline void addConsoleAppender(std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
        getGlobalLogger().addConsoleAppender(std::move(formatter));
    }

    inline void addAppender(std::unique_ptr<IAppender> appender) {
        getGlobalLogger().addAppender(std::move(appender));
    }

    inline void clearAppenders() {
        getGlobalLogger().clearAppenders();
    }

    inline void flushLog() {
        getGlobalLogger().flush();
    }

    inline void runLogLoop() {
        getGlobalLogger().runLoop();
    }
    inline void stopLogLoop() {
        getGlobalLogger().stopLoop();
    }

    // === Logging ===

    inline void debug(const std::string &message, const neko::srcLocInfo &location = {}) {
        getGlobalLogger().debug(message, location);
    }

    inline void info(const std::string &message, const neko::srcLocInfo &location = {}) {
        getGlobalLogger().info(message, location);
    }

    inline void warn(const std::string &message, const neko::srcLocInfo &location = {}) {
        getGlobalLogger().warn(message, location);
    }

    inline void error(const std::string &message, const neko::srcLocInfo &location = {}) {
        getGlobalLogger().error(message, location);
    }

    template <typename... Args>
    void debug(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        getGlobalLogger().debug(message, location);
    }
    template <typename... Args>
    void info(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        getGlobalLogger().info(message, location);
    }
    template <typename... Args>
    void warn(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        getGlobalLogger().warn(message, location);
    }
    template <typename... Args>
    void error(const neko::srcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        getGlobalLogger().error(message, location);
    }

    /**
     * @brief Convenience function to set current thread name
     */
    inline void setCurrentThreadName(const std::string &name) {
        ThreadNameManager::setCurrentThreadName(name);
    }

    /**
     * @brief Convenience function to set specified thread name
     */
    inline void setThreadName(std::thread::id threadId, const std::string &name) {
        ThreadNameManager::setThreadName(threadId, name);
    }

    struct autoLog {
        std::string startMsg;
        std::string endMsg;
        neko::srcLocInfo location;
        std::unique_ptr<IFormatter> formatter;

        autoLog(const std::string &start = "Start", const std::string &end = "End", neko::srcLocInfo loc = {}, std::unique_ptr<IFormatter> fmt = std::make_unique<DefaultFormatter>())
            : startMsg(start), endMsg(end), location(loc), formatter(std::move(fmt)) {
            if (!location.hasInfo()) {
                location = neko::srcLoc::current();
            }
            startMsg = formatter->format(LogRecord(Level::Info, startMsg, location));
            endMsg = formatter->format(LogRecord(Level::Info, endMsg, location));
            getGlobalLogger().info(startMsg, location);
        }

        ~autoLog() {
            getGlobalLogger().info(endMsg, location);
        }
    };

} // namespace neko::log