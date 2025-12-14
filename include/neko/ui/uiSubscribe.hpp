#pragma once

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

#include <neko/log/nlog.hpp>

#include "neko/ui/uiMsg.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/bus/configBus.hpp"
#include "neko/core/launcherProcess.hpp"
#include "neko/event/eventTypes.hpp"
#include "neko/ui/uiEventDispatcher.hpp"
#include "neko/app/lang.hpp"
#include "neko/system/platform.hpp"

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
                    emit nekoWindow->switchToPageD(ui::Page::loading);
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
                const auto cfg = bus::config::getClientConfig();
                const auto &method = cfg.main.launcherMethod;
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    if (method == "launchExit") {
                        nekoWindow->close();
                        app::quit();
                        return;
                    }
                    if (method == "launchHideRestore") {
                        emit nekoWindow->hideWindowD();
                        return;
                    }
                    emit nekoWindow->switchToPageD(ui::Page::home);
                }
            });

        bus::event::subscribe<event::LaunchFinishedEvent>(
            [](const event::LaunchFinishedEvent &) {
                const auto cfg = bus::config::getClientConfig();
                const auto &method = cfg.main.launcherMethod;
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    if (method == "launchHideRestore") {
                        emit nekoWindow->showWindowD();
                        emit nekoWindow->switchToPageD(ui::Page::home);
                    } else if (method == "launchVisible") {
                        emit nekoWindow->switchToPageD(ui::Page::home);
                    }
                }
            });

        bus::event::subscribe<event::LaunchFailedEvent>(
            [](const event::LaunchFailedEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    NoticeMsg notice{
                        .title = lang::tr(lang::keys::launcher::category, lang::keys::launcher::launchFailedTitle),
                        .message = lang::tr(lang::keys::launcher::category, lang::keys::launcher::launchFailedMessage) + e.reason,
                        .buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok)}};
                    emit nekoWindow->showNoticeD(notice);
                }
            });

        bus::event::subscribe<event::UpdateCompleteEvent>(
            [](const event::UpdateCompleteEvent &) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    NoticeMsg notice{
                        .title = "Update",
                        .message = "Update completed. You can continue using the launcher.",
                        .buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")}};
                    QMetaObject::invokeMethod(nekoWindow, [nekoWindow, notice]() {
                        emit nekoWindow->switchToPageD(ui::Page::home);
                        nekoWindow->showNotice(notice);
                    }, Qt::QueuedConnection);
                }
            });

        bus::event::subscribe<event::UpdateFailedEvent>(
            [](const event::UpdateFailedEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    NoticeMsg notice{
                        .title = "Update failed",
                        .message = e.reason,
                        .buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")}};
                    QMetaObject::invokeMethod(nekoWindow, [nekoWindow, notice]() {
                        emit nekoWindow->switchToPageD(ui::Page::home);
                        nekoWindow->showNotice(notice);
                    }, Qt::QueuedConnection);
                }
            });

        bus::event::subscribe<event::RestartRequestEvent>(
            [](const event::RestartRequestEvent &e) {
                log::info("Restart requested: {}", {}, e.reason);
                try {
                    core::launcherNewProcess(e.command, system::workPath());
                } catch (const std::exception &ex) {
                    log::error("Failed to start updater: {}", {}, ex.what());
                }

                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    QMetaObject::invokeMethod(nekoWindow, [nekoWindow]() {
                        emit nekoWindow->quitAppD();
                    }, Qt::QueuedConnection);
                } else {
                    QMetaObject::invokeMethod(QCoreApplication::instance(), []() {
                        QCoreApplication::quit();
                    }, Qt::QueuedConnection);
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

        bus::event::subscribe<event::RefreshTextEvent>(
            [](const event::RefreshTextEvent &) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->refreshTextD();
                }
            });
    }
} // namespace neko::ui
