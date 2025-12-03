#pragma once

#include <neko/schema/exception.hpp>

#include "neko/event/eventTypes.hpp"

// Bus Modules
#include "neko/bus/threadBus.hpp"
#include "neko/bus/eventBus.hpp"

namespace neko::app {

    struct RunningInfo {
        neko::uint64 eventLoopThreadId = 0;
        // If the future returns, it means the event loop has ended
        std::future<void> eventLoopFuture;
    };
    /**
     * @brief Starts the application.
     * @return RunningInfo containing information about the running application.
     * @throws ex::Runtime if no worker threads are available.
     */
    inline RunningInfo run() {
        RunningInfo info;
        auto ids = bus::thread::getWorkerIds();
        if (ids.empty()) {
            throw ex::Runtime("No worker threads available");
        }

        info.eventLoopThreadId = ids[ids.size() - 1];

        info.eventLoopFuture = bus::thread::submitToWorker(info.eventLoopThreadId,&bus::event::run);
        bus::event::publish<event::NekoStartEvent>({});

        return info;
    }

    inline void quit() {
        bus::event::publish<event::NekoQuitEvent>({});
    }

} // namespace neko::app