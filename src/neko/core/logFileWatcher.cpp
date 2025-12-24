/**
 * @file logFileWatcher.cpp
 * @brief Log file watcher implementation for real-time Minecraft log monitoring
 * @author moehoshio
 */

#include "neko/core/logFileWatcher.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include <neko/log/nlog.hpp>

#include <QtCore/QTimer>
#include <QtCore/QFileSystemWatcher>

#include <filesystem>
#include <fstream>
#include <mutex>
#include <atomic>

namespace neko::core {

    struct LogFileWatcher::Impl {
        std::string logFilePath;
        std::streampos lastReadPosition = 0;
        std::atomic<bool> watching{false};
        std::unique_ptr<QTimer> pollTimer;
        std::function<void(const std::string&)> lineCallback;
        neko::uint32 pollingIntervalMs = 100;
        std::ifstream fileStream;
        mutable std::mutex mutex;

        void checkForNewLines() {
            if (!watching.load()) {
                return;
            }

            std::lock_guard<std::mutex> lock(mutex);

            // Check if file exists
            if (!std::filesystem::exists(logFilePath)) {
                return;
            }

            // Check if file was rotated (size smaller than last position)
            auto currentSize = std::filesystem::file_size(logFilePath);
            if (currentSize < static_cast<std::uintmax_t>(lastReadPosition)) {
                // File was rotated, reset to beginning
                log::debug("Log file rotated, resetting read position");
                lastReadPosition = 0;
                if (fileStream.is_open()) {
                    fileStream.close();
                }
            }

            // Open or reopen file if needed
            if (!fileStream.is_open()) {
                fileStream.open(logFilePath, std::ios::in);
                if (!fileStream.is_open()) {
                    return;
                }
                fileStream.seekg(lastReadPosition);
            }

            // Read new lines
            std::string line;
            while (std::getline(fileStream, line)) {
                if (line.empty()) {
                    continue;
                }

                // Remove trailing carriage return if present (Windows line endings)
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                log::debug("LogFileWatcher read line: {}", {}, line);

                // Publish event for BGM system
                bus::event::publish(event::LogFileLineEvent{.line = line, .source = logFilePath});

                // Call callback if set
                if (lineCallback) {
                    lineCallback(line);
                }
            }

            // Clear EOF flag and save position
            fileStream.clear();
            lastReadPosition = fileStream.tellg();
        }
    };

    LogFileWatcher::LogFileWatcher() : pImpl(std::make_unique<Impl>()) {
        pImpl->pollTimer = std::make_unique<QTimer>();
        QObject::connect(pImpl->pollTimer.get(), &QTimer::timeout, [this]() {
            pImpl->checkForNewLines();
        });
    }

    LogFileWatcher::~LogFileWatcher() {
        stop();
    }

    LogFileWatcher::LogFileWatcher(LogFileWatcher&&) noexcept = default;
    LogFileWatcher& LogFileWatcher::operator=(LogFileWatcher&&) noexcept = default;

    bool LogFileWatcher::start(const std::string& logFilePath, bool fromEnd) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);

        if (pImpl->watching.load()) {
            log::warn("LogFileWatcher already watching: {}", {}, pImpl->logFilePath);
            return false;
        }

        if (!std::filesystem::exists(logFilePath)) {
            log::warn("Log file does not exist yet: {}", {}, logFilePath);
            // Still start watching - file may be created later
        }

        pImpl->logFilePath = logFilePath;

        if (fromEnd && std::filesystem::exists(logFilePath)) {
            // Start from end of file
            pImpl->lastReadPosition = std::filesystem::file_size(logFilePath);
        } else {
            pImpl->lastReadPosition = 0;
        }

        pImpl->watching.store(true);
        pImpl->pollTimer->start(pImpl->pollingIntervalMs);

        log::info("Started watching log file: {} (from {})", {}, 
                  logFilePath, fromEnd ? "end" : "beginning");
        return true;
    }

    void LogFileWatcher::stop() {
        std::lock_guard<std::mutex> lock(pImpl->mutex);

        if (!pImpl->watching.load()) {
            return;
        }

        pImpl->watching.store(false);
        pImpl->pollTimer->stop();

        if (pImpl->fileStream.is_open()) {
            pImpl->fileStream.close();
        }

        log::info("Stopped watching log file: {}", {}, pImpl->logFilePath);
        pImpl->logFilePath.clear();
        pImpl->lastReadPosition = 0;
    }

    bool LogFileWatcher::isWatching() const {
        return pImpl->watching.load();
    }

    std::string LogFileWatcher::getWatchedPath() const {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        return pImpl->logFilePath;
    }

    void LogFileWatcher::setPollingInterval(neko::uint32 intervalMs) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->pollingIntervalMs = intervalMs;
        if (pImpl->watching.load()) {
            pImpl->pollTimer->setInterval(intervalMs);
        }
    }

    void LogFileWatcher::setLineCallback(std::function<void(const std::string&)> callback) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->lineCallback = std::move(callback);
    }

    // Singleton instance
    LogFileWatcher& getLogFileWatcher() {
        static LogFileWatcher instance;
        return instance;
    }

    void subscribeLogWatcherToProcessEvents() {
        // When Minecraft process starts, start watching its log file
        bus::event::subscribe<event::ProcessStartedEvent>([](const event::ProcessStartedEvent& ev) {
            if (ev.detached) {
                return; // Don't watch detached processes
            }

            // Construct log file path from working directory
            std::filesystem::path workDir = ev.workingDir;
            std::filesystem::path logPath = workDir / "logs" / "latest.log";

            // Also check for .minecraft subdirectory
            if (!std::filesystem::exists(logPath)) {
                logPath = workDir / ".minecraft" / "logs" / "latest.log";
            }

            if (!logPath.empty()) {
                log::info("LogFileWatcher starting to watch: {}", {}, logPath.string());
                getLogFileWatcher().start(logPath.string(), true);
            }
        });

        // When process exits, stop watching
        bus::event::subscribe<event::ProcessExitedEvent>([](const event::ProcessExitedEvent&) {
            getLogFileWatcher().stop();
        });

        log::info("LogFileWatcher subscribed to process events");
    }

} // namespace neko::core
