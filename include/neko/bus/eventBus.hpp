/**
 * @see neko/event/event.hpp
 * @file eventBus.hpp
 * @brief Provides a bus for managing events and event listeners.
 */

#pragma once

#include <neko/schema/types.hpp>
#include <neko/event/event.hpp>

#include "neko/bus/resources.hpp"

namespace neko::bus::event {

    // === Event methods ===

    // === Subscription Event ===
    template <typename T>
    inline neko::event::HandlerId subscribe(std::function<void(const T &)> handler, neko::Priority minPriority = neko::Priority::Low) {
        return bus::getEventLoop().subscribe<T>(std::move(handler), minPriority);
    }

    template <typename T>
    inline bool unsubscribe(neko::event::HandlerId handlerId) {
        return bus::getEventLoop().unsubscribe<T>(handlerId);
    }

    // === Publish Event ===
    template <typename T>
    inline void publish(const T &eventData) {
        bus::getEventLoop().publish<T>(eventData);
    }

    template <typename T>
    inline void publish(T &&eventData) {
        bus::getEventLoop().publish<T>(std::forward<T>(eventData));
    }

    template <typename T>
    inline void publish(const T &eventData, neko::Priority priority, neko::SyncMode mode = neko::SyncMode::Async) {
        bus::getEventLoop().publish<T>(eventData, priority, mode);
    }

    template <typename T>
    inline neko::event::EventId publishAfter(neko::uint64 ms, const T &eventData) {
        return bus::getEventLoop().publishAfter<T>(ms, eventData);
    }

    template <typename T>
    inline neko::event::EventId publishAfter(neko::uint64 ms, T &&eventData) {
        return bus::getEventLoop().publishAfter<T>(ms, std::forward<T>(eventData));
    }

    template <typename T>
    inline bool addFilter(neko::event::HandlerId handlerId, std::unique_ptr<neko::event::EventFilter<T>> filter) {
        return bus::getEventLoop().addFilter<T>(handlerId, std::move(filter));
    }

    // === Task Scheduling ===
    inline neko::event::EventId scheduleTask(neko::event::TimePoint t, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return bus::getEventLoop().scheduleTask(t, std::move(cb), priority);
    }

    inline neko::event::EventId scheduleTask(neko::uint64 ms, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return bus::getEventLoop().scheduleTask(ms, std::move(cb), priority);
    }

    inline neko::event::EventId scheduleRepeating(neko::uint64 intervalMs, std::function<void()> cb, neko::Priority priority = neko::Priority::Normal) {
        return bus::getEventLoop().scheduleRepeating(intervalMs, std::move(cb), priority);
    }

    inline bool cancelTask(neko::event::EventId id) {
        return bus::getEventLoop().cancelTask(id);
    }

    inline void cleanupCancelledTasks() {
        bus::getEventLoop().cleanupCancelledTasks();
    }

    // === Event Loop Control ===

    inline void run() {
        bus::getEventLoop().run();
    }

    inline void stopLoop() {
        bus::getEventLoop().stopLoop();
    }

    inline void wakeUp() {
        bus::getEventLoop().wakeUp();
    }

    inline void setMaxQueueSize(neko::uint64 size) {
        bus::getEventLoop().setMaxQueueSize(size);
    }

    inline void enableStatistics(bool enable) {
        bus::getEventLoop().enableStatistics(enable);
    }

    inline void setLogger(std::function<void(const std::string &)> loggerFunc) {
        bus::getEventLoop().setLogger(std::move(loggerFunc));
    }

    // === Information methods ===

    inline void resetStatistics() {
        bus::getEventLoop().resetStatistics();
    }

    inline bool isRunning() {
        return bus::getEventLoop().isRunning();
    }

    inline neko::event::EventStats getStatistics() {
        return bus::getEventLoop().getStatistics();
    }

} // namespace neko::bus::event