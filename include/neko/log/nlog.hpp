/**
 * @file nlog.hpp
 * @brief neko logging module
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include "neko/schema/exception.hpp"
#include "neko/schema/srcloc.hpp"
#include "neko/schema/types.hpp"

#include <chrono>
#include <format>
#include <memory>

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <filesystem>
#include <fstream>
#include <iostream>

#include <sstream>
#include <string>

#include <thread>

#include <queue>
#include <unordered_map>
#include <vector>

namespace neko::log {

    /**
     * @brief Log level enum
     */
    enum class Level : neko::uint8 {
        Debug = 1, ///< Debug
        Info = 2,  ///< General information
        Warn = 3,  ///< Potential issues
        Error = 4, ///< Error
        Off = 255  ///< Logging off
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
        std::unordered_map<std::thread::id, std::string> threadNames;
        std::mutex namesMutex;

    public:
        /**
         * @brief Set the current thread's name
         */
        void setCurrentThreadName(const std::string &name) {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames[std::this_thread::get_id()] = name;
        }

        /**
         * @brief Set the name of the specified thread
         */
        void setThreadName(std::thread::id threadId, const std::string &name) {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames[threadId] = name;
        }

        /**
         * @brief Get thread name, returns thread ID string if not set
         */
        std::string getThreadName(std::thread::id threadId) {
            std::lock_guard<std::mutex> lock(namesMutex);
            auto it = threadNames.find(threadId);
            if (it != threadNames.end()) {
                return it->second;
            }

            // Returns thread ID as string
            std::ostringstream oss;
            oss << "Thread " << threadId;
            return oss.str();
        }

        /**
         * @brief Remove thread name
         */
        void removeThreadName(std::thread::id threadId) {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames.erase(threadId);
        }

        /**
         * @brief Clear all thread names
         */
        void clearAllNames() {
            std::lock_guard<std::mutex> lock(namesMutex);
            threadNames.clear();
        }
    } inline threadNameManager;

    /**
     * @brief Log record structure
     */
    struct LogRecord {
        Level level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        neko::SrcLocInfo location;
        std::string threadName;

        LogRecord() = default;
        LogRecord(Level lvl, std::string msg, const neko::SrcLocInfo &loc = {})
            : level(lvl), message(std::move(msg)),
              timestamp(std::chrono::system_clock::now()), location(loc) {
            threadName = threadNameManager.getThreadName(std::this_thread::get_id());
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
     * @brief Default log formatter with configurable file path handling
     */
    class DefaultFormatter : public IFormatter {
    private:
        std::string rootPath;
        bool useFullPath;

    public:
        /**
         * @brief Constructor
         * @param rootPath Root path for truncating file paths (empty = use filename only)
         * @param useFullPath If true, use full file paths regardless of rootPath
         */
        explicit DefaultFormatter(const std::string &rootPath = "", bool useFullPath = false)
            : rootPath(rootPath), useFullPath(useFullPath) {}

        std::string format(const LogRecord &record) override {
            // Format timestamp
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

            auto truncatePath = [](const std::string &fullPath, const std::string &rootPath) -> std::string {
                std::filesystem::path full(fullPath);
                std::filesystem::path root(rootPath);

                std::error_code ec;
                auto rel = std::filesystem::relative(full, root, ec);
                if (!ec && !rel.empty()) {
                    return rel.string();
                }
                return fullPath;
            };

            // Handle file path based on configuration
            std::string file;
            if (useFullPath) {
                file = record.location.getFile();
            } else if (!rootPath.empty()) {
                file = truncatePath(record.location.getFile(), rootPath);
            } else {
                file = std::filesystem::path(record.location.getFile()).filename().string();
            }

            return std::format("[{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}] [{}] [{}] [{}:{}] {}",
                               tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                               tm.tm_hour, tm.tm_min, tm.tm_sec, ms.count(),
                               levelToString(record.level),
                               record.threadName,
                               file, record.location.getLine(),
                               record.message);
        }
    };

    /**
     * @brief Log appender interface
     */
    class IAppender {
    private:
        Level level = Level::Debug; // Default to accept all levels
        bool useLoggerLevel = true; // Default to use logger's level

    public:
        virtual ~IAppender() = default;
        virtual void append(const LogRecord &record) = 0;
        virtual void flush() {}

        /**
         * @brief Set appender's log level
         */
        void setLevel(Level lvl) {
            level = lvl;
            useLoggerLevel = false;
        }

        /**
         * @brief Get appender's log level
         */
        Level getLevel() const {
            return level;
        }

        /**
         * @brief Set to use logger's level instead of appender's level
         */
        void setLoggerLevel(bool use = true) {
            useLoggerLevel = use;
        }

        /**
         * @brief Check if this appender should use logger's level
         */
        bool shouldUseLoggerLevel() const {
            return useLoggerLevel;
        }

        /**
         * @brief Check if the given level should be logged by this appender
         */
        bool isEnabled(Level logLevel, Level loggerLevel) const {
            Level effectiveLevel = useLoggerLevel ? loggerLevel : level;
            return logLevel >= effectiveLevel && effectiveLevel != Level::Off;
        }
    };

    /**
     * @brief Console appender
     */
    class ConsoleAppender : public IAppender {
    private:
        std::unique_ptr<IFormatter> formatter;
        std::mutex mutex;

    public:
        explicit ConsoleAppender(std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>())
            : formatter(std::move(formatter)) {}

        explicit ConsoleAppender(Level level, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>())
            : formatter(std::move(formatter)) {
            setLevel(level);
        }

        void append(const LogRecord &record) override {
            constexpr neko::strview
                red = "\033[31m",
                green = "\033[32m",
                yellow = "\033[33m",
                blue = "\033[34m",
                magenta = "\033[35m",
                cyan = "\033[36m",
                reset = "\033[0m";

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
        explicit FileAppender(const std::string &filename, bool isTruncate = false, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>())
            : formatter(std::move(formatter)), file(filename, isTruncate ? std::ios::trunc : std::ios::app) {
            if (!file.is_open()) {
                throw neko::ex::FileError("Failed to open log file: " + filename);
            }
        }

        explicit FileAppender(const std::string &filename, Level level, bool isTruncate = false, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>())
            : formatter(std::move(formatter)), file(filename, isTruncate ? std::ios::trunc : std::ios::app) {
            if (!file.is_open()) {
                throw neko::ex::FileError("Failed to open log file: " + filename);
            }
            setLevel(level);
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
        Level level = Level::Info;
        neko::SyncMode mode = neko::SyncMode::Sync;
        std::vector<std::unique_ptr<IAppender>> appenders;
        mutable std::mutex appenderMutex;

        // Queue for async logging
        std::queue<LogRecord> logQueue;
        std::condition_variable logQueueCondVar;
        mutable std::mutex logQueueMutex;

    public:
        explicit Logger(Level level = Level::Info) : level(level) {
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

        void addFileAppender(const std::string &filename, bool isTruncate = false, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.push_back(std::make_unique<FileAppender>(filename, isTruncate, std::move(formatter)));
        }

        void addFileAppender(const std::string &filename, Level level, bool isTruncate = false, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.push_back(std::make_unique<FileAppender>(filename, level, isTruncate, std::move(formatter)));
        }

        void addConsoleAppender(std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.push_back(std::make_unique<ConsoleAppender>(std::move(formatter)));
        }

        void addConsoleAppender(Level level, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
            std::lock_guard<std::mutex> lock(appenderMutex);
            appenders.push_back(std::make_unique<ConsoleAppender>(level, std::move(formatter)));
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
                if (appender->isEnabled(record.level, this->level)) {
                    appender->append(record);
                }
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
                    std::unique_lock<std::mutex> lock(logQueueMutex);
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
            while (!logQueue.empty()) {
                append(logQueue.front());
                logQueue.pop();
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

        void log(Level level, const std::string &message, const neko::SrcLocInfo &location = {}) {
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

        void debug(const std::string &message, const neko::SrcLocInfo &location = {}) {
            log(Level::Debug, message, location);
        }

        void info(const std::string &message, const neko::SrcLocInfo &location = {}) {
            log(Level::Info, message, location);
        }

        void warn(const std::string &message, const neko::SrcLocInfo &location = {}) {
            log(Level::Warn, message, location);
        }

        void error(const std::string &message, const neko::SrcLocInfo &location = {}) {
            log(Level::Error, message, location);
        }

        // === formatted message logging ===

        template <typename... Args>
        void debug(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Debug, message, location);
        }

        template <typename... Args>
        void info(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Info, message, location);
        }

        template <typename... Args>
        void warn(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Warn, message, location);
        }

        template <typename... Args>
        void error(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
            auto message = std::format(fmt, std::forward<Args>(args)...);
            log(Level::Error, message, location);
        }
    } inline logger;

    // === Convenience functions ===

    // === Info ===
    inline Level getLevel() {
        return logger.getLevel();
    }
    inline neko::SyncMode getMode() {
        return logger.getMode();
    }

    inline bool isEnabled(Level level) {
        return logger.isEnabled(level);
    }

    // === Control ===

    inline void setLevel(Level level) {
        logger.setLevel(level);
    }

    inline void setMode(neko::SyncMode m) {
        logger.setMode(m);
    }

    inline void addFileAppender(const std::string &filename, bool isTruncate = false, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
        logger.addFileAppender(filename, isTruncate, std::move(formatter));
    }

    inline void addFileAppender(const std::string &filename, Level level, bool isTruncate = false, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
        logger.addFileAppender(filename, level, isTruncate, std::move(formatter));
    }

    inline void addConsoleAppender(std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
        logger.addConsoleAppender(std::move(formatter));
    }

    inline void addConsoleAppender(Level level, std::unique_ptr<IFormatter> formatter = std::make_unique<DefaultFormatter>()) {
        logger.addConsoleAppender(level, std::move(formatter));
    }

    inline void addAppender(std::unique_ptr<IAppender> appender) {
        logger.addAppender(std::move(appender));
    }

    inline void clearAppenders() {
        logger.clearAppenders();
    }

    inline void flushLog() {
        logger.flush();
    }

    inline void runLogLoop() {
        logger.runLoop();
    }
    inline void stopLogLoop() {
        logger.stopLoop();
    }

    // === Logging ===

    inline void debug(const std::string &message, const neko::SrcLocInfo &location = {}) {
        logger.debug(message, location);
    }

    inline void info(const std::string &message, const neko::SrcLocInfo &location = {}) {
        logger.info(message, location);
    }

    inline void warn(const std::string &message, const neko::SrcLocInfo &location = {}) {
        logger.warn(message, location);
    }

    inline void error(const std::string &message, const neko::SrcLocInfo &location = {}) {
        logger.error(message, location);
    }

    template <typename... Args>
    void debug(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        logger.debug(message, location);
    }
    template <typename... Args>
    void info(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        logger.info(message, location);
    }
    template <typename... Args>
    void warn(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        logger.warn(message, location);
    }
    template <typename... Args>
    void error(const neko::SrcLocInfo &location, std::format_string<Args...> fmt, Args &&...args) {
        auto message = std::format(fmt, std::forward<Args>(args)...);
        logger.error(message, location);
    }

    /**
     * @brief Convenience function to set current thread name
     */
    inline void setCurrentThreadName(const std::string &name) {
        threadNameManager.setCurrentThreadName(name);
    }

    /**
     * @brief Convenience function to set specified thread name
     */
    inline void setThreadName(std::thread::id threadId, const std::string &name) {
        threadNameManager.setThreadName(threadId, name);
    }

    struct autoLog {
        std::string startMsg;
        std::string endMsg;
        neko::SrcLocInfo location;
        std::unique_ptr<IFormatter> formatter;

        autoLog(const std::string &start = "Start", const std::string &end = "End", neko::SrcLocInfo loc = {}, std::unique_ptr<IFormatter> fmt = std::make_unique<DefaultFormatter>())
            : startMsg(start), endMsg(end), location(loc), formatter(std::move(fmt)) {
            startMsg = formatter->format(LogRecord(Level::Info, startMsg, location));
            endMsg = formatter->format(LogRecord(Level::Info, endMsg, location));
            logger.info(startMsg, location);
        }

        ~autoLog() {
            logger.info(endMsg, location);
        }
    };

} // namespace neko::log