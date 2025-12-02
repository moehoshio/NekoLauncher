#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#endif // _WIN32

#include <iostream>
#include "neko/app/appinit.hpp"
#include "neko/app/app.hpp"
#include "neko/bus/eventBus.hpp"

using namespace neko;

int main(){
    app::init::initialize();

    bus::event::subscribe<event::NekoStartEvent>([](const event::NekoStartEvent&){
        log::info("Received NekoStartEvent, application started.");
    });
    bus::event::subscribe<event::NekoQuitEvent>([](const event::NekoQuitEvent&){
        log::info("Received NekoQuitEvent, exiting application...");
        log::flushLog();
        bus::event::stopLoop();
    });

    bus::event::publishAfter(2000, [](){
        app::quit();
    });
    app::run();
    return 0;
}