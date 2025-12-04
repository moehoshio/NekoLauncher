#pragma once

#include <QtCore/QCoreApplication>

#include <neko/log/nlog.hpp>

#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"
#include "neko/ui/uiEventDispatcher.hpp"

namespace neko::ui {

    inline void subscribeToUiEvent() {

        bus::event::subscribe<event::UpdateCompleteEvent>(
            [](const event::UpdateCompleteEvent &) {
                if (auto mainWindow = UiEventDispatcher::getMainWindow()) {
                    emit mainWindow->switchToPageD(Page::home);
                }
            });

        bus::event::subscribe<event::LoadingValueChangedEvent>(
            [](const event::LoadingValueChangedEvent &e) {
                if (auto mainWindow = UiEventDispatcher::getMainWindow()) {
                    emit mainWindow->setLoadingValueD(static_cast<neko::uint32>(e.progressValue));
                }
            });

        bus::event::subscribe<event::LoadingStatusChangedEvent>(
            [](const event::LoadingStatusChangedEvent &e) {
                if (auto mainWindow = UiEventDispatcher::getMainWindow()) {
                    emit mainWindow->setLoadingStatusD(e.statusMessage);
                }
            });
    }
} // namespace neko::ui
