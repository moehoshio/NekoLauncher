/**
 * @see neko::core::thread
 * @see neko/core/threadPool.hpp
 * @file threadBus.hpp
 * @brief Provides a thread bus for managing tasks and worker threads.
 */

#pragma once

#include "neko/core/threadPool.hpp"
#include "neko/core/resources.hpp"
#include "neko/schema/priority.hpp"

/**
 * @namespace neko::bus::thread
 */
namespace neko::bus::thread {

    // === Submit task ===
    auto submit(auto &&function, auto &&...args) {
        return neko::core::getThreadPool().submit(std::forward<decltype(function)>(function), std::forward<decltype(args)>(args)...);
    }
    auto submitWithPriority(neko::Priority priority, auto &&function, auto &&...args) {
        return neko::core::getThreadPool().submitWithPriority(priority, std::forward<decltype(function)>(function), std::forward<decltype(args)>(args)...);
    }
    auto submitToWorker(neko::uint64 workerId, auto &&function, auto &&...args) {
        return neko::core::getThreadPool().submitToWorker(workerId, std::forward<decltype(function)>(function), std::forward<decltype(args)>(args)...);
    }

    // === Thread info ===
    inline neko::uint64 getThreadCount() {
        return neko::core::getThreadPool().getThreadCount();
    }
    inline std::vector<neko::uint64> getWorkerIds() {
        return neko::core::getThreadPool().getWorkerIds();
    }
    inline neko::uint64 getPendingTaskCount() {
        return neko::core::getThreadPool().getPendingTaskCount();
    }
    inline bool isEmpty() {
        return neko::core::getThreadPool().isEmpty();
    }
    inline neko::core::thread::TaskStats getTaskStats() {
        return neko::core::getThreadPool().getTaskStats();
    }
    inline bool isStatisticsEnabled() {
        return neko::core::getThreadPool().isStatisticsEnabled();
    }
    inline neko::uint64 getMaxQueueSize() {
        return neko::core::getThreadPool().getMaxQueueSize();
    }
    inline bool isQueueFull() {
        return neko::core::getThreadPool().isQueueFull();
    }

    // === Instant Utilization ===
    inline double getQueueUtilization() {
        return neko::core::getThreadPool().getQueueUtilization();
    }
    inline double getThreadUtilization() {
        return neko::core::getThreadPool().getThreadUtilization();
    }

    // === Control ===
    inline void waitForAllTasksCompletion() {
        neko::core::getThreadPool().waitForAllTasksCompletion();
    }
    template <typename Rep, typename Period>
    bool waitForAllTasksCompletion(const std::chrono::duration<Rep, Period> &timeout) {
        return neko::core::getThreadPool().waitForAllTasksCompletion(timeout);
    }
    inline void stop(bool waitForCompletion = true) {
        neko::core::getThreadPool().stop(waitForCompletion);
    }
    inline void setThreadCount(neko::uint64 newThreadCount) {
        neko::core::getThreadPool().setThreadCount(newThreadCount);
    }
    inline void clearPendingTasks() {
        neko::core::getThreadPool().clearPendingTasks();
    }
    inline void resetStats() {
        neko::core::getThreadPool().resetStats();
    }
    inline void enableStatistics(bool enable) {
        neko::core::getThreadPool().enableStatistics(enable);
    }
    inline void setLogger(std::function<void(const std::string &)> loggerFunc) {
        neko::core::getThreadPool().setLogger(std::move(loggerFunc));
    }
    inline void setMaxQueueSize(neko::uint64 maxSize) {
        neko::core::getThreadPool().setMaxQueueSize(maxSize);
    }

} // namespace neko::bus::thread
