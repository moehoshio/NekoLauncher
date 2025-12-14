#pragma once

#include <neko/log/nlog.hpp>

#include "neko/bus/eventBus.hpp"
#include "neko/bus/threadBus.hpp"
#include "neko/event/eventTypes.hpp"
#include "neko/core/launcher.hpp"
#include "neko/core/launcherProcess.hpp"

namespace neko::core {

    inline void subscribeToCoreEvents() {

        (void)bus::event::subscribe<event::RestartRequestEvent>([](const event::RestartRequestEvent &evt) {
            log::info("RestartRequestEvent received: Reason: {}, Command: {}", {}, evt.reason, evt.command);
            launcherNewProcess(evt.command);
            app::quit();
        });

        (void)bus::event::subscribe<event::MaintenanceEvent>([](const event::MaintenanceEvent &evt) {
            log::warn("MaintenanceEvent received: Title: {}, Message: {}", {}, evt.title, evt.message);
            // Forward to UI so a notice dialog is shown alongside logging
            bus::event::publish(event::ShowNoticeEvent{evt});
        });
        (void)bus::event::subscribe<event::ConfigLoadedEvent>([](const event::ConfigLoadedEvent &evt) {
            log::info("ConfigLoadedEvent: path={}, success={}", {}, evt.path, evt.success);
        });
        (void)bus::event::subscribe<event::ConfigSavedEvent>([](const event::ConfigSavedEvent &evt) {
            log::info("ConfigSavedEvent: path={}, success={}", {}, evt.path, evt.success);
        });
        (void)bus::event::subscribe<event::ConfigUpdatedEvent>([](const event::ConfigUpdatedEvent &evt) {
            log::debug("ConfigUpdatedEvent: lang={}, backgroundType={}", {}, evt.config.main.lang, evt.config.main.backgroundType);
        });
        (void)bus::event::subscribe<event::UpdateAvailableEvent>([](const event::UpdateAvailableEvent &evt) {
            log::info("UpdateAvailableEvent received: {} -> {}", {}, evt.title, evt.resourceVersion);
        });
        (void)bus::event::subscribe<event::UpdateCompleteEvent>([](const event::UpdateCompleteEvent &) {
            log::info("UpdateCompleteEvent received: Application has been updated successfully.");
            // Trigger UI text refresh so version/resource labels update immediately.
            bus::event::publish(event::RefreshTextEvent{});
        });
        (void)bus::event::subscribe<event::UpdateFailedEvent>([](const event::UpdateFailedEvent &evt) {
            log::error("UpdateFailedEvent received: {}", {}, evt.reason);
        });

    }
} // namespace neko::core