#pragma once

#include <neko/log/nlog.hpp>

#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"


namespace neko::minecraft {
    inline void subscribeToMinecraftEvents() {

        (void)bus::event::subscribe<event::MaintenanceEvent>([](const event::MaintenanceEvent &evt) {
            log::warn("MaintenanceEvent received: Title: {}, Message: {}", {}, evt.title, evt.message);
        });
    }
} // namespace neko::minecraft