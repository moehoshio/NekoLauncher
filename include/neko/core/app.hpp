#pragma once

#include "neko/core/threadBus.hpp"
#include "neko/core/threadPool.hpp"

#include "neko/event/event.hpp"
#include "neko/event/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/schema/exception.hpp"

#include <QtWidgets/QApplication>

namespace neko::core::app {

    struct RunningInfo {
        std::function<int()> mainThreadRunLoopFunction;
        neko::uint64 eventLoopThreadId = 0;
        // If the future returns, it means the event loop has ended
        std::future<void> eventLoopFuture;
    };
    /**
     * @brief Starts the application.
     * @return RunningInfo containing information about the running application.
     * @throws ex::Runtime if no worker threads are available.
     */
    RunningInfo run() {
        RunningInfo info;
        auto ids = neko::bus::thread::getWorkerIds();
        if (ids.empty()) {
            throw ex::Runtime("No worker threads available");
        }

        info.eventLoopThreadId = ids[ids.size() - 1];
        info.mainThreadRunLoopFunction = [] {
            return QApplication::exec();
        };

        info.eventLoopFuture = neko::bus::thread::submitToWorker(info.eventLoopThreadId, [] {
            neko::bus::event::run();
        });
        neko::bus::event::publish<event::StartEvent>({});

        return info;
    }

    void quit() {
        neko::bus::event::stopLoop();
        QApplication::quit();
    }

} // namespace neko::core::app
