#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#endif // _WIN32

#include "neko/app/app.hpp"
#include "neko/app/appinit.hpp"

#include <neko/log/nlog.hpp>
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include <iostream>
#include <thread>

using namespace neko;

int main() {
    try {
        app::init::initialize();
        bus::event::subscribe<event::NekoStartEvent>([](const event::NekoStartEvent &) {
            log::info("NekoLc has started.");
        });
        bus::event::subscribe<event::NekoQuitEvent>([](const event::NekoQuitEvent &) {
            log::info("NekoLc is quitting...");
            bus::event::stopLoop();
        });
        bus::event::publishAfter(13000, event::NekoQuitEvent{});
        auto runingInfo = app::run();
        // The main thread should be in the Qt event loop.
        // Before the UI is available, we wait for the event loop to exit.
        runingInfo.eventLoopFuture.get();
    } catch (const ex::Exception &e) {
        log::error("Unhandled Exception: " + std::string(e.what()));
    } catch (const std::exception &e) {
        log::error("Unexpected error: " + std::string(e.what()));
    }

    return 0;
}