/**
 * @file event.hpp
 * @brief Event system
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include "neko/schema/types.hpp"

// STL includes
#include <any>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>

#include <chrono>

#include <functional>
#include <future>
#include <optional>

#include <memory>

#include <queue>
#include <string>
#include <vector>

#include <type_traits>
#include <typeindex>

#include <unordered_map>
#include <unordered_set>

#include <algorithm>

/**
 * @brief Event namespace
 * @namespace neko::event
 */
namespace neko::event {

    using TimePoint = std::chrono::steady_clock::time_point;
    using EventId = neko::uint64;
    using HandlerId = neko::uint64;

    // Event statistics
    struct EventStats {
        neko::uint64 totalEvents = 0;
        neko::uint64 processedEvents = 0;
        neko::uint64 droppedEvents = 0;
        neko::uint64 failedEvents = 0;
        std::chrono::milliseconds avgProcessingTime{0};
        std::chrono::milliseconds maxProcessingTime{0};
    };

    // Base event class
    class BaseEvent {
    public:
        EventId id;
        TimePoint timestamp;
        neko::Priority priority;
        neko::SyncMode mode;

        BaseEvent(neko::Priority prio = neko::Priority::Normal, neko::SyncMode procMode = neko::SyncMode::Async)
            : id(0), timestamp(std::chrono::steady_clock::now()), priority(prio), mode(procMode) {}
        virtual ~BaseEvent() = default;
        virtual std::type_index getType() const = 0;
    };

    // Templated event class
    template <typename T>
    class Event : public BaseEvent {
    public:
        T data;

        Event() : data(T{}) {}

        /**
         * @brief Construct an Event with event data.
         * @param eventData The event data.
         */
        Event(const T &eventData) : data(eventData) {}
        /**
         * @brief Construct an Event with event data (move).
         * @param eventData The event data (rvalue).
         */
        Event(T &&eventData) : data(std::move(eventData)) {}

        /**
         * @brief Get the type index of the event data.
         * @return The type index.
         */
        std::type_index getType() const override {
            return std::type_index(typeid(T));
        }
    };

    // Event filter interface
    template <typename T>
    class EventFilter {
    public:
        virtual ~EventFilter() = default;
        /**
         * @brief Determine whether the event data should be processed.
         * @param eventData The event data.
         * @return True if should process, false otherwise.
         */
        virtual bool shouldProcess(const T &eventData) = 0;
    };

    // Event handler interface
    class BaseEventHandler {
    public:
        HandlerId id;
        virtual ~BaseEventHandler() = default;
        /**
         * @brief Handle the event.
         * @param event The event to handle.
         */
        virtual void handle(const std::shared_ptr<BaseEvent> &event) = 0;
        /**
         * @brief Get the type index of the event this handler handles.
         * @return The type index.
         */
        virtual std::type_index getEventType() const = 0;
    };

    // Enhanced event handler with filters
    template <typename T>
    class EventHandler : public BaseEventHandler {
    private:
        std::function<void(const T &)> callback;
        std::vector<std::unique_ptr<EventFilter<T>>> filters;
        neko::Priority minPriority = neko::Priority::Low;

    public:
        /**
         * @brief Construct an EventHandler with a callback.
         * @param cb The callback function.
         */
        EventHandler(std::function<void(const T &)> cb) : callback(std::move(cb)) {}

        /**
         * @brief Add a filter to this handler.
         * @param filter The filter to add.
         */
        void addFilter(std::unique_ptr<EventFilter<T>> filter) {
            filters.push_back(std::move(filter));
        }

        /**
         * @brief Set the minimum priority for this handler.
         * @param priority The minimum priority.
         */
        void setMinPriority(neko::Priority priority) {
            minPriority = priority;
        }

        /**
         * @brief Handle the event.
         * @throws std::bad_cast if the event cannot be cast to Event<T>.
         * @throws maybe throw other exceptions in the callback.
         * @note The callback will only be invoked if the event's priority meets the minimum required priority
         */
        void handle(const std::shared_ptr<BaseEvent> &event) override {
            auto typedEvent = std::static_pointer_cast<Event<T>>(event);

            // Check priority
            if (static_cast<neko::uint8>(event->priority) < static_cast<neko::uint8>(minPriority)) {
                return;
            }

            // Apply filters
            for (const auto &filter : filters) {
                if (!filter->shouldProcess(typedEvent->data)) {
                    return;
                }
            }

            callback(typedEvent->data);
        }

        /**
         * @brief Get the type index of the event this handler handles.
         * @return The type index.
         */
        std::type_index getEventType() const override {
            return std::type_index(typeid(T));
        }
    };

    // scheduled task
    struct ScheduledTask {
        TimePoint execTime;
        std::function<void()> callback;
        EventId id;
        neko::Priority priority;
        bool cancelled = false;
        bool repeating = false;
        std::chrono::milliseconds interval{0};

        /**
         * @brief Construct a scheduled task.
         * @param t The execution time.
         * @param cb The callback function.
         * @param eventId The event/task ID.
         * @param prio The priority.
         */
        ScheduledTask(TimePoint t, std::function<void()> cb, EventId eventId, neko::Priority prio = neko::Priority::Normal)
            : execTime(t), callback(std::move(cb)), id(eventId), priority(prio) {}

        /**
         * @brief Compare two scheduled tasks for priority.
         * @note  execTime > priority > id
         */
        bool operator<(const ScheduledTask &other) const {
            if (execTime != other.execTime) {
                return execTime > other.execTime;
            }
            if (static_cast<neko::uint8>(priority) != static_cast<neko::uint8>(other.priority)) {
                return static_cast<neko::uint8>(priority) < static_cast<neko::uint8>(other.priority);
            }
            return id > other.id;
        }
        bool operator>(const ScheduledTask &other) const {
            return other < *this;
        }
    };

    /**
     * @class EventLoop
     * @brief Event loop class that manages event handling and task scheduling.
     */
    class EventLoop {
    private:
        // === Member variables ===

        // Task scheduling
        std::priority_queue<ScheduledTask> taskQueue;
        mutable std::mutex taskMtx;
        std::condition_variable taskCv;
        std::atomic<bool> stop;
        std::atomic<EventId> nextTaskId{1};
        std::unordered_set<EventId> cancelledTasks;

        // Event system
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<BaseEventHandler>>> eventHandlers;
        std::queue<std::shared_ptr<BaseEvent>> eventQueue;
        mutable std::shared_mutex eventMtx;
        std::condition_variable_any eventCv;
        std::atomic<HandlerId> nextHandlerId{1};

        // Enhanced features e.g. statistics, logging
        std::atomic<bool> enableStats{true};
        EventStats stats;
        mutable std::mutex statsMtx;
        neko::uint64 maxQueueSize = 100000;
        std::function<void(const std::string &)> logger;

        // Event loop control
        mutable std::mutex loopMtx;
        std::condition_variable loopCv;

    private:
        // === Internal methods ===

        // === Event methods ===
        /**
         * @brief Publish an event to the event queue.
         * @param event The event to publish.
         */
        void publishEvent(const std::shared_ptr<BaseEvent> &event) {
            std::unique_lock<std::shared_mutex> lock(eventMtx);

            if (eventQueue.size() >= maxQueueSize) {
                updateStats(false, true); // dropped event
                if (logger) {
                    lock.unlock();
                    logger("Event queue overflow, dropping event");
                }
                return;
            }

            eventQueue.push(event);
            lock.unlock();

            // notify the event loop
            eventCv.notify_one();
            loopCv.notify_one();
        }

        /**
         * @brief Process all events in the event queue.
         * @return True if any events were processed, false otherwise.
         */
        bool processEvents() {
            bool processedAny = false;

            while (!stop.load()) {
                std::shared_ptr<BaseEvent> event;
                {
                    std::unique_lock<std::shared_mutex> lock(eventMtx);
                    if (eventQueue.empty())
                        break;
                    event = eventQueue.front();
                    eventQueue.pop();
                    processedAny = true;
                }
                processSingleEvent(event);
            }

            return processedAny;
        }

        /**
         * @brief Process a single event.
         * @param event The event to process.
         */
        void processSingleEvent(const std::shared_ptr<BaseEvent> &event) {
            auto startTime = std::chrono::steady_clock::now();
            bool success = true;

            try {
                std::shared_lock<std::shared_mutex> lock(eventMtx);
                auto typeIndex = event->getType();
                auto it = eventHandlers.find(typeIndex);

                if (it != eventHandlers.end()) {
                    // Copy handler list to avoid modification during processing
                    auto handlers = it->second;
                    lock.unlock();

                    for (const auto &handler : handlers) {
                        try {
                            handler->handle(event);
                        } catch (const std::exception &e) {
                            success = false;
                            if (logger) {
                                logger("Event handler failed: " + std::string(e.what()));
                            }
                        } catch (...) {
                            success = false;
                            if (logger) {
                                logger("Event handler failed: unknown exception");
                            }
                        }
                    }
                }
            } catch (const std::exception &e) {
                success = false;
                if (logger) {
                    logger("Event processing failed: " + std::string(e.what()));
                }
            } catch (...) {
                success = false;
                if (logger) {
                    logger("Event handler failed: unknown exception");
                }
            }

            updateStats(false, false, !success, startTime);
        }

        // === Event methods End ===

        // === Task methods ===

        /**
         * @brief Process scheduled tasks.
         * @details If there are tasks ready to execute, execute them immediately and return the next task's execution time.
         * @return The next task execution time, if any.
         * If no tasks are scheduled, returns std::nullopt.
         */
        std::optional<TimePoint> processScheduledTasks() {
            std::unique_lock<std::mutex> lock(taskMtx);
            auto now = std::chrono::steady_clock::now();

            while (!taskQueue.empty() && !stop.load()) {
                auto next = taskQueue.top();

                // handle cancelled tasks
                if (cancelledTasks.find(next.id) != cancelledTasks.end()) {
                    taskQueue.pop();
                    if (!next.repeating) {
                        cancelledTasks.erase(next.id);
                    }
                    continue;
                }

                // If the task's execution time has not arrived, return the next task's time
                if (now < next.execTime) {
                    return next.execTime;
                }

                // Execute the task
                taskQueue.pop();
                lock.unlock();

                try {
                    next.callback();
                } catch (const std::exception &e) {
                    if (logger) {
                        logger("Scheduled task failed: " + std::string(e.what()));
                    }
                } catch (...) {
                    if (logger) {
                        logger("Scheduled task failed: unknown exception");
                    }
                }

                lock.lock();
                now = std::chrono::steady_clock::now();
            }

            return std::nullopt;
        }

        /**
         * @brief Internal implementation of scheduling tasks.
         * @param t The execution time.
         * @param cb The callback function.
         * @param priority The priority.
         * @return The scheduled task ID.
         */
        EventId scheduleTaskInternal(TimePoint t, std::function<void()> cb, neko::Priority priority) {
            EventId id = nextTaskId.fetch_add(1);
            {
                std::lock_guard<std::mutex> lock(taskMtx);
                ScheduledTask task{t, std::move(cb), id, priority};
                taskQueue.push(std::move(task));
            }

            // Notify the task processor and event loop
            taskCv.notify_one();
            loopCv.notify_one();

            return id;
        }

        // === Task methods End ===

        /**
         * @brief Wait for work in the event loop.
         * @details Waits for either a new event, a scheduled task, or a stop signal.
         * @param nextTaskTime The next scheduled task time.
         * @param maxWaitTime The maximum wait time.
         */
        void waitForWork(const std::optional<TimePoint> &nextTaskTime,
                         const std::chrono::milliseconds &maxWaitTime) {
            std::unique_lock<std::mutex> lock(loopMtx);

            if (stop.load())
                return;

            auto now = std::chrono::steady_clock::now();
            auto waitUntil = now + maxWaitTime;

            if (nextTaskTime.has_value()) {
                waitUntil = std::min(waitUntil, *nextTaskTime);
            }

            // Wait until:
            // 1. There is a new event or task (notify)
            // 2. The wait time is reached
            // 3. Stop signal
            loopCv.wait_until(lock, waitUntil, [this]() {
                return stop.load();
            });
        }

        /**
         * @brief Update event statistics.
         * @param isNewEvent Whether this is a new event.
         * @param isDropped Whether the event was dropped.
         * @param isFailed Whether the event processing failed.
         * @param startTime The start time of processing.
         */
        void updateStats(bool isNewEvent = false, bool isDropped = false, bool isFailed = false, TimePoint startTime = TimePoint{}) {
            if (!enableStats.load())
                return;

            std::lock_guard<std::mutex> lock(statsMtx);
            if (isNewEvent) {
                ++stats.totalEvents;
            } else if (isDropped) {
                ++stats.droppedEvents;
            } else if (isFailed) {
                ++stats.failedEvents;
            } else {
                // Successfully processed event
                ++stats.processedEvents;
                if (startTime != TimePoint{}) {
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - startTime);

                    // Update average processing time
                    if (stats.processedEvents == 1) {
                        stats.avgProcessingTime = duration;
                    } else {
                        auto total = stats.avgProcessingTime.count() * (stats.processedEvents - 1);
                        stats.avgProcessingTime = std::chrono::milliseconds(
                            (total + duration.count()) / stats.processedEvents);
                    }

                    if (duration > stats.maxProcessingTime) {
                        stats.maxProcessingTime = duration;
                    }
                }
            }
        }

        // === Internal methods End ===

    public:
        // === Public methods ===

        // === Event methods ===

        /**
         * @brief Subscribe to an event type.
         * @tparam T The event data type.
         * @param handler The handler function.
         * @param minPriority The minimum priority to handle.
         * @return The handler ID.
         */
        template <typename T>
        HandlerId subscribe(std::function<void(const T &)> handler,
                            neko::Priority minPriority = neko::Priority::Low) {
            std::unique_lock<std::shared_mutex> lock(eventMtx);
            auto eventHandler = std::make_shared<EventHandler<T>>(std::move(handler));
            eventHandler->id = nextHandlerId.fetch_add(1);
            eventHandler->setMinPriority(minPriority);

            auto typeIndex = std::type_index(typeid(T));
            eventHandlers[typeIndex].push_back(eventHandler);

            return eventHandler->id;
        }

        /**
         * @brief Unsubscribe a handler from an event type.
         * @tparam T The event data type.
         * @param handlerId The handler ID.
         * @return True if unsubscribed, false otherwise.
         */
        template <typename T>
        bool unsubscribe(HandlerId handlerId) {
            std::unique_lock<std::shared_mutex> lock(eventMtx);
            auto typeIndex = std::type_index(typeid(T));
            auto it = eventHandlers.find(typeIndex);
            if (it == eventHandlers.end())
                return false;

            auto &handlers = it->second;
            auto removeIt = std::remove_if(handlers.begin(), handlers.end(),
                                           [handlerId](const std::shared_ptr<BaseEventHandler> &handler) {
                                               return handler->id == handlerId;
                                           });

            if (removeIt != handlers.end()) {
                handlers.erase(removeIt, handlers.end());
                return true;
            }
            return false;
        }

        /**
         * @brief Publish an event.
         * @tparam T The event data type.
         * @param eventData The event data.
         */
        template <typename T>
        void publish(const T &eventData) {
            auto event = std::make_shared<Event<T>>(eventData);
            publishEvent(event);
        }

        /**
         * @brief Publish an event (move).
         * @tparam T The event data type.
         * @param eventData The event data (rvalue).
         */
        template <typename T>
        void publish(T &&eventData) {
            auto event = std::make_shared<Event<T>>(std::forward<T>(eventData));
            publishEvent(event);
        }

        /**
         * @brief Publish an event with priority and processing mode.
         * @tparam T The event data type.
         * @param eventData The event data.
         * @param priority The event priority.
         * @param mode The processing mode.
         */
        template <typename T>
        void publish(const T &eventData, neko::Priority priority, neko::SyncMode mode = neko::SyncMode::Async) {
            updateStats(true);

            auto event = std::make_shared<Event<T>>(eventData);
            event->priority = priority;
            event->mode = mode;

            if (mode == neko::SyncMode::Sync) {
                processSingleEvent(event);
            } else {
                publishEvent(event);
            }
        }

        /**
         * @brief Publish an event after a delay.
         * @tparam T The event data type.
         * @param ms Delay in milliseconds.
         * @param eventData The event data.
         * @return The scheduled task ID.
         */
        template <typename T>
        EventId publishAfter(neko::uint64 ms, const T &eventData) {
            return scheduleTask(ms, [this, eventData]() {
                publish(eventData);
            });
        }

        /**
         * @brief Publish an event after a delay (move).
         * @tparam T The event data type.
         * @param ms Delay in milliseconds.
         * @param eventData The event data (rvalue).
         * @return The scheduled task ID.
         */
        template <typename T>
        EventId publishAfter(neko::uint64 ms, T &&eventData) {
            auto capturedData = std::forward<T>(eventData);
            return scheduleTask(ms, [this, capturedData = std::move(capturedData)]() mutable {
                publish(std::move(capturedData));
            });
        }

        /**
         * @brief Add a filter to an existing event handler.
         * @tparam T The event data type.
         * @param handlerId The handler ID.
         * @param filter The filter to add.
         * @return True if added, false otherwise.
         */
        template <typename T>
        bool addFilter(HandlerId handlerId, std::unique_ptr<EventFilter<T>> filter) {
            std::shared_lock<std::shared_mutex> readLock(eventMtx);
            auto typeIndex = std::type_index(typeid(T));
            auto it = eventHandlers.find(typeIndex);
            if (it == eventHandlers.end())
                return false;

            // Find the target handler
            std::shared_ptr<EventHandler<T>> targetHandler = nullptr;
            for (auto &handler : it->second) {
                if (handler->id == handlerId) {
                    targetHandler = std::static_pointer_cast<EventHandler<T>>(handler);
                    break;
                }
            }
            readLock.unlock();

            if (targetHandler) {
                // addFilter to the handler , no need for event lock
                targetHandler->addFilter(std::move(filter));
                return true;
            }
            return false;
        }

        // === Event methods End ===

        // === Task methods ===

        /**
         * @brief Schedule a task at a specific time.
         * @param t The execution time.
         * @param cb The callback function.
         * @param priority The priority.
         * @return The scheduled task ID.
         */
        EventId scheduleTask(TimePoint t, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
            return scheduleTaskInternal(t, std::move(cb), priority);
        }

        /**
         * @brief Schedule a task after a delay.
         * @param ms Delay in milliseconds.
         * @param cb The callback function.
         * @param priority The priority.
         * @return The scheduled task ID.
         */
        EventId scheduleTask(neko::uint64 ms, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
            return scheduleTaskInternal(std::chrono::steady_clock::now() + std::chrono::milliseconds(ms), std::move(cb), priority);
        }

        /**
         * @brief Schedule a repeating task.
         * @param intervalMs The interval in milliseconds.
         * @param cb The callback function.
         * @param priority The priority.
         * @return The scheduled task ID.
         */
        EventId scheduleRepeating(neko::uint64 intervalMs, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
            EventId id = nextTaskId.fetch_add(1);
            auto interval = std::chrono::milliseconds(intervalMs);

            // Use shared_ptr to safely capture data in callback
            auto sharedCb = std::make_shared<std::function<void()>>(std::move(cb));

            std::function<void()> repeatWrapper = [this, sharedCb, interval, id, priority]() {
                (*sharedCb)();
                // Reschedule (if not cancelled)
                {
                    std::lock_guard<std::mutex> lock(taskMtx);
                    if (cancelledTasks.find(id) == cancelledTasks.end()) {
                        ScheduledTask task{std::chrono::steady_clock::now() + interval,
                                           [this, sharedCb, interval, id, priority]() {
                                               (*sharedCb)();
                                               std::lock_guard<std::mutex> lock(taskMtx);
                                               if (cancelledTasks.find(id) == cancelledTasks.end()) {
                                                   ScheduledTask nextTask{std::chrono::steady_clock::now() + interval,
                                                                          [this, sharedCb, interval, id, priority]() { /* recursive */ }, id, priority};
                                                   nextTask.repeating = true;
                                                   nextTask.interval = interval;
                                                   taskQueue.push(std::move(nextTask));
                                               }
                                           },
                                           id, priority};
                        task.repeating = true;
                        task.interval = interval;
                        taskQueue.push(std::move(task));
                    }
                }
            };

            {
                std::lock_guard<std::mutex> lock(taskMtx);
                ScheduledTask task{std::chrono::steady_clock::now() + interval, std::move(repeatWrapper), id, priority};
                task.repeating = true;
                task.interval = interval;
                taskQueue.push(std::move(task));
            }
            taskCv.notify_one();
            return id;
        }

        /**
         * @brief Cancel a scheduled task.
         * @param id The task ID.
         * @return True if cancelled, false otherwise.
         */
        bool cancelTask(EventId id) {
            std::lock_guard<std::mutex> lock(taskMtx);
            auto inserted = cancelledTasks.insert(id);
            return inserted.second;
        }

        /**
         * @brief Clean up cancelled tasks from the queue.
         */
        void cleanupCancelledTasks() {
            std::lock_guard<std::mutex> lock(taskMtx);
            // Only keep cancelled task IDs that are still in the queue
            std::unordered_set<EventId> activeCancelled;
            std::priority_queue<ScheduledTask> tempQueue = taskQueue;

            while (!tempQueue.empty()) {
                auto task = tempQueue.top();
                tempQueue.pop();
                if (cancelledTasks.find(task.id) != cancelledTasks.end()) {
                    activeCancelled.insert(task.id);
                }
            }

            cancelledTasks = std::move(activeCancelled);
        }

        // === Task methods End ===

        // === Event Loop Control ===

        /**
         * @brief Run the main event loop.
         */
        void run() {
            constexpr auto cleanupInterval = std::chrono::seconds(2);
            constexpr auto maxWaitTime = std::chrono::milliseconds(500);

            auto lastCleanup = std::chrono::steady_clock::now();

            while (!stop.load()) {

                bool hasEvents = processEvents();

                auto nextTaskTime = processScheduledTasks();

                auto now = std::chrono::steady_clock::now();
                bool shouldCleanup = (now - lastCleanup >= cleanupInterval);

                if (shouldCleanup) {
                    cleanupCancelledTasks();
                    lastCleanup = now;
                }

                // if no immediate work is pending, wait
                if (!hasEvents && !shouldCleanup) {
                    waitForWork(nextTaskTime, maxWaitTime);
                }
            }
        }

        /**
         * @brief Stop the event loop.
         */
        void stopLoop() {
            stop.store(true);

            taskCv.notify_all();
            eventCv.notify_all();
            loopCv.notify_all();
        }

        /**
         * @brief Wake up the event loop.
         */
        void wakeUp() {
            loopCv.notify_one();
        }

        // === Event Loop Control End ===

        // === Configuration and management methods ===

        /**
         * @brief Set the maximum event queue size.
         * @param size The maximum size.
         */
        void setMaxQueueSize(neko::uint64 size) {
            maxQueueSize = size;
        }

        /**
         * @brief Enable or disable statistics collection.
         * @param enable True to enable, false to disable.
         */
        void enableStatistics(bool enable) {
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
         * @brief Reset event processing statistics.
         */
        void resetStatistics() {
            std::lock_guard<std::mutex> lock(statsMtx);
            stats = EventStats{};
        }

        // === Configuration and management methods End ===

        // ==== Information methods ====

        /**
         * @brief Check if the event loop is running.
         * @return True if running, false otherwise.
         */
        bool isRunning() const {
            return !stop.load();
        }

        /**
         * @brief Get event processing statistics.
         * @return The event statistics.
         */
        EventStats getStatistics() const {
            std::lock_guard<std::mutex> lock(statsMtx);
            return stats;
        }

        struct QueueSizes {
            neko::uint64 eventQueueSize;
            neko::uint64 taskQueueSize;
        };

        /**
         * @brief Get the current sizes of the event and task queues.
         * @return The current sizes of the event and task queues.
         */
        QueueSizes getQueueSizes() const {
            std::shared_lock<std::shared_mutex> eventLock(eventMtx);
            std::lock_guard<std::mutex> taskLock(taskMtx);
            return {eventQueue.size(), taskQueue.size()};
        }

        // === Information methods End ===

    }; // EventLoop

} // namespace neko::event
