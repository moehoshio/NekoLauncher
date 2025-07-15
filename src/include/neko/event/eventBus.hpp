#pragma once

#include "neko/schema/priority.hpp"
#include "neko/event/event.hpp"
#include "neko/core/resources.hpp"

namespace neko::bus::event {

    // === Event methods ===

    // === Subscription Event ===
    template <typename T>
    inline HandlerId subscribe(std::function<void(const T &)> handler, neko::Priority minPriority = neko::Priority::Low) {
        return core::getEventLoop().subscribe<T>(std::move(handler), minPriority);
    }

    template <typename T>
    inline bool unsubscribe(HandlerId handlerId) {
        return core::getEventLoop().unsubscribe<T>(handlerId);
    }

    // === Publish Event ===
    template <typename T>
    inline void publish(const T &eventData) {
        core::getEventLoop().publish<T>(eventData);
    }

    template <typename T>
    inline void publish(T &&eventData) {
        core::getEventLoop().publish<T>(std::forward<T>(eventData));
    }

    template <typename T>
    inline void publish(const T &eventData, neko::Priority priority, ProcessingMode mode = ProcessingMode::ASYNC) {
        core::getEventLoop().publish<T>(eventData, priority, mode);
    }

    template <typename T>
    inline EventId publishAfter(neko::uint64 ms, const T &eventData) {
        return core::getEventLoop().publishAfter<T>(ms, eventData);
    }

    template <typename T>
    inline EventId publishAfter(neko::uint64 ms, T &&eventData) {
        return core::getEventLoop().publishAfter<T>(ms, std::forward<T>(eventData));
    }

    template <typename T>
    inline bool addFilter(HandlerId handlerId, std::unique_ptr<EventFilter<T>> filter) {
        return core::getEventLoop().addFilter<T>(handlerId, std::move(filter));
    }

    // === Task Scheduling ===
    inline EventId scheduleTask(EventLoop::TimePoint t, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return core::getEventLoop().scheduleTask(t, std::move(cb), priority);
    }

    inline EventId scheduleTask(neko::uint64 ms, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return core::getEventLoop().scheduleTask(ms, std::move(cb), priority);
    }

    inline EventId scheduleRepeating(neko::uint64 intervalMs, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return core::getEventLoop().scheduleRepeating(intervalMs, std::move(cb), priority);
    }

    inline bool cancelTask(EventId id) {
        return core::getEventLoop().cancelTask(id);
    }

    inline void cleanupCancelledTasks() {
        core::getEventLoop().cleanupCancelledTasks();
    }

    // === Event Loop Control ===
    inline bool isRunning() {
        return core::getEventLoop().isRunning();
    }

    inline void run() {
        core::getEventLoop().run();
    }

    inline void stopLoop() {
        core::getEventLoop().stopLoop();
    }

    inline void wakeUp() {
        core::getEventLoop().wakeUp();
    }

    inline void setMaxQueueSize(neko::uint64 size) {
        core::getEventLoop().setMaxQueueSize(size);
    }

    inline void enableStatistics(bool enable) {
        core::getEventLoop().enableStatistics(enable);
    }

    inline void setLogger(std::function<void(const std::string &)> loggerFunc) {
        core::getEventLoop().setLogger(std::move(loggerFunc));
    }

    inline EventStats getStatistics() {
        return core::getEventLoop().getStatistics();
    }

    inline void resetStatistics() {
        core::getEventLoop().resetStatistics();
    }

    inline std::pair<size_t, size_t> getQueueSizes() {
        return core::getEventLoop().getQueueSizes();
    }

} // namespace neko::bus::event