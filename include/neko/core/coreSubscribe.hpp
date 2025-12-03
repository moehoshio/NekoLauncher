#pragma once

#include <neko/log/nlog.hpp>

#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

namespace neko::core {

    inline void subscribeToCoreEvents() {

        (void)bus::event::subscribe<event::MaintenanceEvent>([](const event::MaintenanceEvent &evt) {
            log::warn("MaintenanceEvent received: Title: {}, Message: {}", {}, evt.title, evt.message);
        });
        (void)bus::event::subscribe<event::UpdateAvailableEvent>([](const event::UpdateAvailableEvent &evt) {
            
        });
        (void)bus::event::subscribe<event::UpdateCompleteEvent>([](const event::UpdateCompleteEvent &) {
            log::info("UpdateCompleteEvent received: Application has been updated successfully.");
        });
    }
} // namespace neko::core