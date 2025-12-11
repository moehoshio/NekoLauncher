#pragma once

#include <QtCore/QCoreApplication>

#include <neko/log/nlog.hpp>

#include "neko/ui/uiMsg.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"
#include "neko/ui/uiEventDispatcher.hpp"

namespace neko::ui {

    inline void subscribeToUiEvent() {

        bus::event::subscribe<event::ShowNoticeEvent>(
            [](const event::ShowNoticeEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->showNoticeD(static_cast<const neko::ui::NoticeMsg &>(e));
                }
            });
        bus::event::subscribe<event::ShowLoadingEvent>(
            [](const event::ShowLoadingEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->showLoadingD(static_cast<const neko::ui::LoadingMsg &>(e));
                }
            });
        
        bus::event::subscribe<event::ShowInputEvent>(
            [](const event::ShowInputEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->showInputD(static_cast<const neko::ui::InputMsg &>(e));
                }
            });

        bus::event::subscribe<event::HideInputEvent>(
            [](const event::HideInputEvent &) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->hideInputD();
                }
            });

        bus::event::subscribe<event::CurrentPageChangeEvent>(
            [](const event::CurrentPageChangeEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->switchToPageD(e.page);
                }
            });

        bus::event::subscribe<event::LaunchStartedEvent>(
            [](const event::LaunchStartedEvent &) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->switchToPageD(ui::Page::home);
                }
            });

        bus::event::subscribe<event::LaunchFailedEvent>(
            [](const event::LaunchFailedEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    nekoWindow->showNotice({.title = lang::tr(lang::keys::launcher::category,lang::keys::launcher::launchFailedTitle),
                                            .message = lang::tr(lang::keys::launcher::category ,lang::keys::launcher::launchFailedMessage) + e.reason,
                                            .buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok)}});
                }
            });

        bus::event::subscribe<event::LoadingValueChangedEvent>(
            [](const event::LoadingValueChangedEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->setLoadingValueD(e.progressValue);
                }
            });

        bus::event::subscribe<event::LoadingStatusChangedEvent>(
            [](const event::LoadingStatusChangedEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->setLoadingStatusD(e.statusMessage);
                }
            });
    }
} // namespace neko::ui
