/**
 * @brief Thread pool
 * @file threadPool.hpp
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */
#pragma once

#include "neko/schema/exception.hpp"
#include "neko/schema/types.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>

#include <thread>

#include <future>
#include <queue>
#include <unordered_set>
#include <vector>

/**
 * @brief Thread pool
 * @namespace neko::core::thread
 */
namespace neko::core::thread {

    using TimePoint = std::chrono::steady_clock::time_point;
    using TaskId = neko::uint64;

    struct Task {
        std::function<void()> function;
        neko::Priority priority;
        TaskId id;
        TimePoint submitTime;

        Task(std::function<void()> func, neko::Priority prio, TaskId taskId)
            : function(std::move(func)), priority(prio), id(taskId),
              submitTime(std::chrono::steady_clock::now()) {}

        /**
         * @brief Compare two tasks for priority.
         * @note  priority > id
         */
        bool operator<(const Task &other) const {
            if (priority != other.priority) {
                return priority > other.priority;
            }
            return id > other.id;
        }

        bool operator>(const Task &other) const {
            return other < *this;
        }
    };

    // Task statistics
    struct TaskStats {
        neko::uint64 activeTasks = 0;
        neko::uint64 totalTasks = 0;
        neko::uint64 submittedTasks = 0;
        neko::uint64 discardedTasks = 0;
        neko::uint64 completedTasks = 0;
        neko::uint64 failedTasks = 0;

        std::chrono::milliseconds totalExecutionTime{0};
        std::chrono::milliseconds maxExecutionTime{0};
        std::chrono::milliseconds avgExecutionTime{0};
    };

    struct WorkerInfo {
        std::thread thread;
        neko::uint64 id;
        TimePoint startTime;
        std::queue<Task> personalTaskQueue;
        std::mutex personalTaskMutex;

        WorkerInfo(std::thread &&t, neko::uint64 i)
            : thread(std::move(t)), id(i), startTime(std::chrono::steady_clock::now()) {}

        WorkerInfo(const WorkerInfo &) = delete;
        WorkerInfo &operator=(const WorkerInfo &) = delete;

        WorkerInfo(WorkerInfo &&other) noexcept
            : thread(std::move(other.thread)),
              id(other.id),
              startTime(other.startTime),
              personalTaskQueue(std::move(other.personalTaskQueue)) {}

        WorkerInfo &operator=(WorkerInfo &&other) noexcept {
            if (this != &other) {
                thread = std::move(other.thread);
                id = other.id;
                startTime = other.startTime;
                personalTaskQueue = std::move(other.personalTaskQueue);
            }
            return *this;
        }

        bool isActive() const noexcept {
            return thread.joinable();
        }
        void cleanup(bool waitForCompletion = true) noexcept {
            if (!isActive()) {
                return;
            }
            try {
                if (waitForCompletion) {
                    thread.join();
                } else {
                    thread.detach();
                }
            } catch (const std::system_error &e) {
                return;
            }
        }
    };

    class ThreadPool {
    private:
        // Worker and task queue
        std::vector<WorkerInfo> workers;
        std::priority_queue<Task> tasks;
        mutable std::mutex workerMutex;
        mutable std::mutex taskQueueMutex;
        std::condition_variable taskQueueCondVar;
        std::atomic<neko::uint64> nextWorkerId = 0;
        std::atomic<neko::uint64> nextTaskId = 0;
        std::atomic<neko::uint64> maxTaskQueueSize = 100000;

        std::atomic<bool> isStop = false;
        std::unordered_set<neko::uint64> exitWorkerIds;
        mutable std::mutex exitIdsMutex;

        // Task statistics
        std::atomic<neko::uint64> activeTasks = 0;
        std::atomic<neko::uint64> completedTasks = 0;
        std::atomic<neko::uint64> failedTasks = 0;
        std::atomic<neko::uint64> discardedTasks = 0;
        // Does not include discarded tasks
        std::atomic<neko::uint64> submittedTasks = 0;
        // Total number =  submittedTasks + discardedTasks

        // Execution time statistics
        std::chrono::nanoseconds totalExecutionTime{0};
        std::chrono::nanoseconds maxExecutionTime{0};
        mutable std::shared_mutex executionMutex;

        // completion is used to notify waitFor...
        mutable std::shared_mutex completionMutex;
        std::condition_variable_any completionCondVar;

        std::atomic<bool> enableStats = true;
        std::function<void(const std::string &)> logger;

        /**
         * @brief Worker thread function that processes tasks.
         */
        void workerThread(neko::uint64 workerId) {
            WorkerInfo *self = nullptr;

            {
                std::lock_guard<std::mutex> lock(workerMutex);
                for (auto &w : workers) {
                    if (w.id == workerId) {
                        self = &w;
                        break;
                    }
                }
            }

            while (!isStop.load()) {
                { // Check if this worker is marked for exit
                    std::lock_guard<std::mutex> lock(exitIdsMutex);
                    if (exitWorkerIds.count(workerId)) {
                        std::lock_guard<std::mutex> lock(workerMutex);

                        for (auto it = workers.begin(); it != workers.end(); ++it) {
                            if (it->id == workerId) {
                                workers.erase(it);
                                break;
                            }
                        }
                    }
                }

                Task task{nullptr, neko::Priority::Normal, 0};
                bool hasTask = false;

                // Check personal queue first
                {
                    std::lock_guard<std::mutex> lock(self->personalTaskMutex);
                    if (!self->personalTaskQueue.empty()) {
                        task = std::move(self->personalTaskQueue.front());
                        self->personalTaskQueue.pop();
                        hasTask = true;
                    }
                }

                if (!hasTask) { // Check global queue
                    std::unique_lock<std::mutex> lock(taskQueueMutex);
                    if (!tasks.empty()) {
                        task = std::move(const_cast<Task &>(tasks.top()));
                        tasks.pop();
                        hasTask = true;
                        ++activeTasks;
                    }
                }

                if (!hasTask) {
                    {
                        std::unique_lock<std::mutex> lock(taskQueueMutex);
                        taskQueueCondVar.wait(lock, [this, self] {
                            std::lock_guard<std::mutex> lock(self->personalTaskMutex);
                            return isStop.load() || !tasks.empty() || !self->personalTaskQueue.empty();
                        });
                    }
                    continue;
                }

                auto startTime = std::chrono::steady_clock::now();

                try {
                    task.function();

                    if (enableStats.load()) {
                        auto endTime = std::chrono::steady_clock::now();
                        auto executionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);

                        {
                            std::unique_lock<std::shared_mutex> lock(executionMutex);
                            totalExecutionTime += executionTime;
                            if (executionTime > maxExecutionTime) {
                                maxExecutionTime = executionTime;
                            }
                        }
                    }

                    ++completedTasks;
                } catch (...) {
                    ++failedTasks;
                    if (logger) {
                        logger("Task execution failed with exception");
                    }
                }

                --activeTasks;

                {
                    std::unique_lock<std::shared_mutex> lock(completionMutex);
                    completionCondVar.notify_all();
                }
            }
            if (logger) {
                logger("Worker thread " + std::to_string(workerId) + " is stopping");
            }
        }

        WorkerInfo createWorker() noexcept {
            neko::uint64 workerId = nextWorkerId++;
            return WorkerInfo(std::thread(&ThreadPool::workerThread, this, workerId), workerId);
        }

    public:
        explicit ThreadPool(neko::uint64 threadCount = std::thread::hardware_concurrency()) noexcept {
            if (threadCount == 0) {
                threadCount = 1;
            }
            for (neko::uint64 i = 0; i < threadCount; ++i) {
                workers.push_back(createWorker());
            }
        }

        ~ThreadPool() noexcept {
            stop();
        }

        ThreadPool(const ThreadPool &) = delete;
        ThreadPool &operator=(const ThreadPool &) = delete;
        ThreadPool(ThreadPool &&) = delete;
        ThreadPool &operator=(ThreadPool &&) = delete;

        // === Submit task ===

        /**
         * @brief Submit a task with normal priority.
         * @param function The function to execute.
         * @param args The arguments to pass to the function.
         * @return A future representing the result of the task.
         * @throws ex::ProgramExit if the thread pool is stopped.
         * @throws ex::TaskRejected if the task is rejected.
         */
        template <typename F, typename... Args>
        auto submit(F &&function, Args &&...args)
            -> std::future<std::invoke_result_t<F, Args...>> {
            return submitWithPriority(neko::Priority::Normal, std::forward<F>(function), std::forward<Args>(args)...);
        }

        /**
         * @brief Submit a task with a specific priority.
         * @param priority The priority of the task.
         * @param function The function to execute.
         * @param args The arguments to pass to the function.
         * @return A future representing the result of the task.
         * @throws ex::ProgramExit if the thread pool is stopped.
         * @throws ex::TaskRejected if the task is rejected.
         */
        template <typename F, typename... Args>
        auto submitWithPriority(neko::Priority priority, F &&function, Args &&...args)
            -> std::future<std::invoke_result_t<F, Args...>> {
            using ReturnType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

            if (isStop.load()) {
                throw ex::ProgramExit("Cannot submit tasks to stopped thread pool");
            }

            std::unique_lock<std::mutex> lock(taskQueueMutex);

            if (tasks.size() >= maxTaskQueueSize.load()) {
                throw ex::TaskRejected("Task queue is full");
            }

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                [function, args...]() mutable -> ReturnType {
                    return std::invoke(std::forward<F>(function), std::forward<Args>(args)...);
                });

            std::future<ReturnType> res = task->get_future();
            TaskId taskId = ++nextTaskId;

            tasks.emplace([task]() { (*task)(); }, priority, taskId);
            ++submittedTasks;

            lock.unlock();
            taskQueueCondVar.notify_one();
            return res;
        }

        /**
         * @brief Submit a task to a specific worker thread.
         * @param workerId The ID of the worker thread.
         * @param function The function to execute.
         * @param args The arguments to pass to the function.
         * @return A future representing the result of the task.
         * @throws ex::OutOfRange if the worker thread is not found.
         */
        template <typename F, typename... Args>
        auto submitToWorker(neko::uint64 workerId, F &&function, Args &&...args)
            -> std::future<std::invoke_result_t<F, Args...>> {
            using ReturnType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

            std::unique_lock<std::mutex> lock(workerMutex);

            WorkerInfo *self = nullptr;
            for (auto it = workers.begin(); it != workers.end(); ++it) {
                if (it->id == workerId) {
                    self = &(*it);
                    workers.erase(it);
                    break;
                }
            }
            if (!self)
                throw ex::OutOfRange("Worker not found with ID: " + std::to_string(workerId));

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                [function, args...]() mutable -> ReturnType {
                    return std::invoke(std::forward<F>(function), std::forward<Args>(args)...);
                });

            std::future<ReturnType> res = task->get_future();
            TaskId taskId = ++nextTaskId;

            std::unique_lock<std::mutex> plock(self->personalTaskMutex);
            self->personalTaskQueue.emplace([task]() { (*task)(); }, neko::Priority::Normal, taskId);

            lock.unlock();
            plock.unlock();

            ++submittedTasks;
            taskQueueCondVar.notify_all();
            return res;
        }

        // === Control ===

        /**
         * @brief Wait until the task queue is empty . threads maybe are executing their last task (no unassigned tasks).
         */
        void waitForTasksEmpty() {
            std::shared_lock<std::shared_mutex> lock(completionMutex);
            completionCondVar.wait(lock, [this] {
                std::lock_guard<std::mutex> queueLock(taskQueueMutex);
                return tasks.empty();
            });
        }

        /**
         * @brief Wait until the task queue is empty, with a timeout.
         * @param timeout The duration to wait before giving up.
         * @return True if the task queue became empty within the timeout, false otherwise.
         */
        template <typename Rep, typename Period>
        bool waitForTasksEmpty(const std::chrono::duration<Rep, Period> &timeout) {
            std::shared_lock<std::shared_mutex> lock(completionMutex);
            return completionCondVar.wait_for(lock, timeout, [this] {
                std::lock_guard<std::mutex> queueLock(taskQueueMutex);
                return tasks.empty();
            });
        }

        /**
         * @brief Wait for all tasks to complete.
         */
        void waitForAllTasksCompletion() {
            std::shared_lock<std::shared_mutex> lock(completionMutex);
            completionCondVar.wait(lock, [this] {
                std::lock_guard<std::mutex> queueLock(taskQueueMutex);
                return tasks.empty() && activeTasks.load() == 0;
            });
        }

        /**
         * @brief Wait for all tasks to complete with a timeout.
         * @param timeout The duration to wait before giving up.
         * @return True if all tasks completed within the timeout, false otherwise.
         */
        template <typename Rep, typename Period>
        bool waitForAllTasksCompletion(const std::chrono::duration<Rep, Period> &timeout) {
            std::shared_lock<std::shared_mutex> lock(completionMutex);
            return completionCondVar.wait_for(lock, timeout, [this] {
                std::lock_guard<std::mutex> queueLock(taskQueueMutex);
                return tasks.empty() && activeTasks.load() == 0;
            });
        }

        /**
         * @brief Stop the thread pool.
         * @param waitForCompletion Whether to wait for all tasks to complete before stopping.
         */
        void stop(bool waitForCompletion = true) noexcept {
            {
                std::unique_lock<std::mutex> lock(taskQueueMutex);
                isStop = true;
                taskQueueCondVar.notify_all();
            }

            std::unique_lock<std::mutex> workerLock(workerMutex);
            for (WorkerInfo &worker : workers) {
                worker.cleanup(waitForCompletion);
            }
            workers.clear();
        }

        /**
         * @brief Set the thread count.
         * @param newThreadCount The new thread count.
         * @note If downsizing threads, running tasks will not be interrupted; excess threads will be reclaimed after completing their tasks.
         * @throws ex::ProgramExit if the thread pool is stopped.
         */
        void setThreadCount(neko::uint64 newThreadCount) {
            if (isStop.load()) {
                throw ex::ProgramExit("Cannot resize stopped thread pool");
            }

            if (newThreadCount == 0) {
                newThreadCount = 1;
            }

            if (newThreadCount == getThreadCount()) {
                return;
            }

            std::unique_lock<std::mutex> lock(workerMutex);

            if (newThreadCount > workers.size()) {
                for (neko::uint64 i = workers.size(); i < newThreadCount; ++i) {
                    workers.push_back(createWorker());
                }
            } else if (newThreadCount < workers.size()) {
                // notify workers to exit
                std::lock_guard<std::mutex> exitLock(exitIdsMutex);
                neko::uint64 needToExit = workers.size() - newThreadCount;
                for (neko::uint64 i = 0; i < needToExit; ++i) {
                    exitWorkerIds.insert(workers.at(i).id);
                }
                taskQueueCondVar.notify_all();
            }
        }

        /**
         * @brief Clear all pending tasks in the queue.
         */
        void clearPendingTasks() {
            std::unique_lock<std::mutex> lock(taskQueueMutex);
            while (!tasks.empty()) {
                tasks.pop();
            }
        }

        /**
         * @brief Reset the task statistics.
         */
        void resetStats() noexcept {
            if (!enableStats.load())
                return;
            {
                std::unique_lock<std::shared_mutex> lock(executionMutex);
                totalExecutionTime = std::chrono::nanoseconds{0};
                maxExecutionTime = std::chrono::nanoseconds{0};
            }
            activeTasks.store(0);
            submittedTasks.store(0);
            discardedTasks.store(0);
            completedTasks.store(0);
            failedTasks.store(0);
        }

        /**
         * @brief Enable or disable statistics collection.
         * @param enable True to enable, false to disable.
         */
        void enableStatistics(bool enable) noexcept {
            enableStats.store(enable);
        }

        /**
         * @brief Set the logger function.
         * @param loggerFunc The logger function.
         */
        void setLogger(std::function<void(const std::string &)> loggerFunc) {
            logger = std::move(loggerFunc);
        }

        /**
         * @brief Set the maximum queue size.
         * @param maxSize The maximum queue size.
         * @note This does not affect tasks already in the queue, but if the queue is full, new tasks will be rejected.
         */
        void setMaxQueueSize(neko::uint64 maxSize) noexcept {
            maxTaskQueueSize.store(maxSize);
        }

        // === Thread info ===

        /**
         * @brief Get all available worker thread IDs.
         * @return A vector of worker IDs.
         */
        std::vector<neko::uint64> getWorkerIds() const {
            std::lock_guard<std::mutex> lock(workerMutex);
            std::vector<neko::uint64> ids;
            for (const auto &w : workers) {
                ids.push_back(w.id);
            }
            return ids;
        }

        /**
         * @brief Get the total number of worker threads.
         * @return The total number of worker threads.
         */
        neko::uint64 getThreadCount() const noexcept {
            std::lock_guard<std::mutex> lock(workerMutex);
            return workers.size();
        }

        /**
         * @brief Get the total number of pending tasks.
         * @return The total number of pending tasks.
         */
        neko::uint64 getPendingTaskCount() const noexcept {
            std::lock_guard<std::mutex> lock(taskQueueMutex);
            return tasks.size();
        }

        /**
         * @brief Check if the task queue is empty.
         */
        bool isEmpty() const noexcept {
            std::lock_guard<std::mutex> queueLock(taskQueueMutex);
            return tasks.empty() && activeTasks.load() == 0;
        }

        /**
         * @brief Get the task statistics.
         * @return The task statistics.
         */
        TaskStats getTaskStats() const noexcept {
            TaskStats stats;
            stats.activeTasks = activeTasks.load();
            stats.submittedTasks = submittedTasks.load();
            stats.discardedTasks = discardedTasks.load();
            stats.completedTasks = completedTasks.load();
            stats.failedTasks = failedTasks.load();
            stats.totalTasks = stats.submittedTasks + stats.discardedTasks;

            {
                std::shared_lock<std::shared_mutex> lock(executionMutex);
                stats.totalExecutionTime = std::chrono::duration_cast<std::chrono::milliseconds>(totalExecutionTime);
                stats.maxExecutionTime = std::chrono::duration_cast<std::chrono::milliseconds>(maxExecutionTime);
            }

            if (stats.completedTasks > 0) {
                stats.avgExecutionTime = std::chrono::milliseconds(static_cast<neko::uint64>(stats.totalExecutionTime.count() / stats.completedTasks));
            }
            return stats;
        }

        /**
         * @brief Check if statistics are enabled.
         * @return True if statistics are enabled, false otherwise.
         */
        bool isStatisticsEnabled() const noexcept {
            return enableStats.load();
        }

        /**
         * @brief Get the maximum queue size.
         * @return The maximum queue size.
         */
        neko::uint64 getMaxQueueSize() const noexcept {
            return maxTaskQueueSize.load();
        }

        /**
         * @brief Check if the task queue is full.
         * @return True if the queue is full, false otherwise.
         */
        bool isQueueFull() const noexcept {
            std::lock_guard<std::mutex> lock(taskQueueMutex);
            return tasks.size() >= maxTaskQueueSize.load();
        }

        // === Instant Utilization ===

        /**
         * @brief Get the current queue utilization as a percentage.
         * @return The queue utilization percentage.
         */
        double getQueueUtilization() const noexcept {
            std::lock_guard<std::mutex> lock(taskQueueMutex);
            auto maxSize = maxTaskQueueSize.load();
            if (maxSize == 0)
                return 0.0;
            return static_cast<double>(tasks.size()) / maxSize;
        }

        /**
         * @brief Get the thread utilization as a percentage.
         * @return The thread utilization percentage.
         */
        double getThreadUtilization() const noexcept {
            auto totalThreads = getThreadCount();
            auto activeThreads = activeTasks.load();
            if (totalThreads == 0)
                return 0.0;
            return static_cast<double>(activeThreads) / totalThreads;
        }
    };

} // namespace neko::core::thread
