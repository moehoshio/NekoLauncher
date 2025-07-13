#pragma once
#include "neko/core/threadBus.hpp"
#include "neko/core/threadPool.hpp"

#include "neko/event/event.hpp"
#include "neko/event/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/schema/exception.hpp"
#include "neko/core/resources.hpp"

#include <QtWidgets/QApplication>

namespace neko::core::app {

    struct RunningInfo {
        neko::uint64 eventLoopThreadId = 0;
        std::function<int()> mainThreadRunFunction;
        // If the future returns, it means the event loop has ended
        std::future<void> eventLoopFuture;
    };
    RunningInfo run() {
        RunningInfo info;
        auto ids = neko::bus::thread::getWorkerIds();
        if (ids.empty()) {
            throw ex::Runtime("No worker threads available");
        }

        info.eventLoopThreadId = ids[0];
        info.mainThreadRunFunction = [] {
            return QApplication::exec();
        };

        info.eventLoopFuture = neko::bus::thread::submitToWorker(info.eventLoopThreadId, [] {
            neko::event::run();
        });
        neko::bus::event::publish<event::StartEvent>({});

        return info;
    }

    void quit() {
        neko::event::stopLoop();
        QApplication::quit();
    }

} // namespace neko::core::app
