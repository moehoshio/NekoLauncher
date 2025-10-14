/**
 * @see neko/core/threadPool.hpp
 * @file threadBus.hpp
 * @brief Provides a thread bus for managing tasks and worker threads.
 */

#pragma once

#include <neko/schema/types.hpp>
#include <neko/core/threadPool.hpp>

#include "neko/core/resources.hpp"

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
    inline void waitForGlobalTasks() {
        neko::core::getThreadPool().waitForGlobalTasks();
    }
    template <typename Rep, typename Period>
    bool waitForGlobalTasks(std::chrono::duration<Rep, Period> duration) {
        return neko::core::getThreadPool().waitForGlobalTasks(duration);
    }
    inline void stop(bool waitForCompletion = true) {
        neko::core::getThreadPool().stop(waitForCompletion);
    }
    inline void setThreadCount(neko::uint64 newThreadCount) {
        neko::core::getThreadPool().setThreadCount(newThreadCount);
    }
    inline void setMaxQueueSize(neko::uint64 maxSize) {
        neko::core::getThreadPool().setMaxQueueSize(maxSize);
    }

} // namespace neko::bus::thread
