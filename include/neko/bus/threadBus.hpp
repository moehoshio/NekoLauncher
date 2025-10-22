/**
 * @see neko/core/threadPool.hpp
 * @file threadBus.hpp
 * @brief Provides a thread bus for managing tasks and worker threads.
 */

#pragma once

#include <neko/schema/types.hpp>
#include <neko/core/threadPool.hpp>

#include "neko/bus/resources.hpp"

/**
 * @namespace neko::bus::thread
 */
namespace neko::bus::thread {

    // === Submit task ===
    auto submit(auto &&function, auto &&...args) {
        return bus::getThreadPool().submit(std::forward<decltype(function)>(function), std::forward<decltype(args)>(args)...);
    }
    auto submitWithPriority(neko::Priority priority, auto &&function, auto &&...args) {
        return bus::getThreadPool().submitWithPriority(priority, std::forward<decltype(function)>(function), std::forward<decltype(args)>(args)...);
    }
    auto submitToWorker(neko::uint64 workerId, auto &&function, auto &&...args) {
        return bus::getThreadPool().submitToWorker(workerId, std::forward<decltype(function)>(function), std::forward<decltype(args)>(args)...);
    }

    // === Thread info ===
    inline neko::uint64 getThreadCount() {
        return bus::getThreadPool().getThreadCount();
    }
    inline std::vector<neko::uint64> getWorkerIds() {
        return bus::getThreadPool().getWorkerIds();
    }
    inline neko::uint64 getPendingTaskCount() {
        return bus::getThreadPool().getPendingTaskCount();
    }
    inline neko::uint64 getMaxQueueSize() {
        return bus::getThreadPool().getMaxQueueSize();
    }
    inline bool isQueueFull() {
        return bus::getThreadPool().isQueueFull();
    }

    // === Instant Utilization ===
    inline double getQueueUtilization() {
        return bus::getThreadPool().getQueueUtilization();
    }
    inline double getThreadUtilization() {
        return bus::getThreadPool().getThreadUtilization();
    }

    // === Control ===
    inline void waitForGlobalTasks() {
        bus::getThreadPool().waitForGlobalTasks();
    }
    template <typename Rep, typename Period>
    bool waitForGlobalTasks(std::chrono::duration<Rep, Period> duration) {
        return bus::getThreadPool().waitForGlobalTasks(duration);
    }
    inline void stop(bool waitForCompletion = true) {
        bus::getThreadPool().stop(waitForCompletion);
    }
    inline void setThreadCount(neko::uint64 newThreadCount) {
        bus::getThreadPool().setThreadCount(newThreadCount);
    }
    inline void setMaxQueueSize(neko::uint64 maxSize) {
        bus::getThreadPool().setMaxQueueSize(maxSize);
    }

} // namespace neko::bus::thread
