/**
 * @see neko/event/event.hpp
 * @file eventBus.hpp
 * @brief Provides a bus for managing events and event listeners.
 */

#pragma once

#include <neko/schema/types.hpp>
#include <neko/event/event.hpp>

#include "neko/core/resources.hpp"

namespace neko::bus::event {

    // === Event methods ===

    // === Subscription Event ===
    template <typename T>
    inline neko::event::HandlerId subscribe(std::function<void(const T &)> handler, neko::Priority minPriority = neko::Priority::Low) {
        return core::getEventLoop().subscribe<T>(std::move(handler), minPriority);
    }

    template <typename T>
    inline bool unsubscribe(neko::event::HandlerId handlerId) {
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
    inline void publish(const T &eventData, neko::Priority priority, neko::SyncMode mode = neko::SyncMode::Async) {
        core::getEventLoop().publish<T>(eventData, priority, mode);
    }

    template <typename T>
    inline neko::event::EventId publishAfter(neko::uint64 ms, const T &eventData) {
        return core::getEventLoop().publishAfter<T>(ms, eventData);
    }

    template <typename T>
    inline neko::event::EventId publishAfter(neko::uint64 ms, T &&eventData) {
        return core::getEventLoop().publishAfter<T>(ms, std::forward<T>(eventData));
    }

    template <typename T>
    inline bool addFilter(neko::event::HandlerId handlerId, std::unique_ptr<neko::event::EventFilter<T>> filter) {
        return core::getEventLoop().addFilter<T>(handlerId, std::move(filter));
    }

    // === Task Scheduling ===
    inline neko::event::EventId scheduleTask(neko::event::TimePoint t, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return core::getEventLoop().scheduleTask(t, std::move(cb), priority);
    }

    inline neko::event::EventId scheduleTask(neko::uint64 ms, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return core::getEventLoop().scheduleTask(ms, std::move(cb), priority);
    }

    inline neko::event::EventId scheduleRepeating(neko::uint64 intervalMs, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return core::getEventLoop().scheduleRepeating(intervalMs, std::move(cb), priority);
    }

    inline bool cancelTask(neko::event::EventId id) {
        return core::getEventLoop().cancelTask(id);
    }

    inline void cleanupCancelledTasks() {
        core::getEventLoop().cleanupCancelledTasks();
    }

    // === Event Loop Control ===

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

    // === Information methods ===

    inline void resetStatistics() {
        core::getEventLoop().resetStatistics();
    }

    inline bool isRunning() {
        return core::getEventLoop().isRunning();
    }

    inline neko::event::EventStats getStatistics() {
        return core::getEventLoop().getStatistics();
    }

    inline neko::event::QueueSizes getQueueSizes() {
        return core::getEventLoop().getQueueSizes();
    }

} // namespace neko::bus::event