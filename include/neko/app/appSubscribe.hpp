#pragma once

#include <QtCore/QCoreApplication>

#include <neko/log/nlog.hpp>

#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

namespace neko::app {
    inline void subscribeToAppEvent() {

        (void)bus::event::subscribe<event::NekoStartEvent>([](const event::NekoStartEvent &) {
            log::info("NekoStartEvent received, application has started.");
        });
        (void)bus::event::subscribe<event::NekoQuitEvent>([](const event::NekoQuitEvent &) {
            log::info("NekoQuitEvent received, stopping program.");
            QCoreApplication::quit();
            bus::event::stopLoop();
        });
        
    }
}
