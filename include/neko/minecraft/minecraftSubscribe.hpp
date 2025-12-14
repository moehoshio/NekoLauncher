#pragma once

#include <neko/log/nlog.hpp>

#include "neko/app/lang.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/bus/configBus.hpp"
#include "neko/bus/threadBus.hpp"
#include "neko/core/auth.hpp"
#include "neko/core/launcher.hpp"
#include "neko/event/eventTypes.hpp"

namespace neko::minecraft {
    inline void subscribeToMinecraftEvents() {
        static std::atomic<int> pendingStarts{0};
        static std::atomic<int> activeProcesses{0};

        // Map process lifecycle to launch lifecycle
        bus::event::subscribe<event::ProcessStartedEvent>([](const event::ProcessStartedEvent &e) {
            if (pendingStarts.load(std::memory_order_acquire) == 0)
                return;
            pendingStarts.fetch_sub(1, std::memory_order_acq_rel);
            activeProcesses.fetch_add(1, std::memory_order_acq_rel);
            bus::event::publish<event::LaunchStartedEvent>({});
            bus::event::publish<event::CurrentPageChangeEvent>({ui::Page::home});
            log::info("Process started (detached={}): {}", {}, e.detached, e.command);
        });

        bus::event::subscribe<event::ProcessExitedEvent>([](const event::ProcessExitedEvent &e) {
            if (activeProcesses.load(std::memory_order_acquire) == 0)
                return;
            activeProcesses.fetch_sub(1, std::memory_order_acq_rel);
            bus::event::publish<event::LaunchFinishedEvent>({e.exitCode});
            if (e.exitCode != 0) {
                const auto reason = lang::tr(lang::keys::error::category, lang::keys::error::seeLog, "See log for details.") +
                                   " (code " + std::to_string(e.exitCode) + ")";
                bus::event::publish<event::LaunchFailedEvent>({reason, e.exitCode});

                ui::NoticeMsg notice;
                notice.title = lang::tr(lang::keys::error::category, lang::keys::error::launchFailed, "Launch Failed");
                notice.message = reason;
                notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                bus::event::publish<event::ShowNoticeEvent>(notice);
                bus::event::publish<event::CurrentPageChangeEvent>({ui::Page::home});
            }
            log::info("Process exited (detached={}): {} code={} ", {}, e.detached, e.command, e.exitCode);
        });

        (void)bus::event::subscribe<event::LaunchRequestEvent>([](const event::LaunchRequestEvent &) {
            if (!core::auth::isLoggedIn()) {
                ui::NoticeMsg notice;
                notice.title = lang::tr(lang::keys::error::category, lang::keys::error::launchFailed, "Launch Failed");
                notice.message = lang::tr(lang::keys::setting::category, lang::keys::setting::login, "Login required.");
                notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                bus::event::publish<event::ShowNoticeEvent>(notice);
                return;
            }

            const auto cfg = bus::config::getClientConfig();
            const auto launcherMethod = cfg.main.launcherMethod;

            ui::LoadingMsg loadingMsg;
            loadingMsg.type = ui::LoadingMsg::Type::OnlyRaw;
            bus::event::publish<event::ShowLoadingEvent>(loadingMsg);
            bus::event::publish<event::LoadingStatusChangedEvent>(
                {lang::tr(lang::keys::loading::category, lang::keys::loading::starting, "Starting...")});

            pendingStarts.fetch_add(1, std::memory_order_acq_rel);

            bus::thread::submit([launcherMethod]() {
                try {
                    const bool detach = (launcherMethod == "launchExit");
                    core::launcher(nullptr, nullptr, detach);
                } catch (const ex::Exception &e) {
                    const std::string reason = e.what();
                    bus::event::publish<event::LaunchFinishedEvent>({-1});
                    bus::event::publish<event::LaunchFailedEvent>({reason, -1});
                    ui::NoticeMsg notice;
                    notice.title = lang::tr(lang::keys::error::category, lang::keys::error::launchFailed, "Launch Failed");
                    notice.message = !reason.empty() ? reason : lang::tr(lang::keys::error::category, lang::keys::error::seeLog, "See log for details.");
                    notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                    bus::event::publish<event::ShowNoticeEvent>(notice);
                    bus::event::publish<event::CurrentPageChangeEvent>({ui::Page::home});
                } catch (const std::exception &e) {
                    const std::string reason = e.what();
                    bus::event::publish<event::LaunchFinishedEvent>({-1});
                    bus::event::publish<event::LaunchFailedEvent>({reason, -1});
                    ui::NoticeMsg notice;
                    notice.title = lang::tr(lang::keys::error::category, lang::keys::error::launchFailed, "Launch Failed");
                    notice.message = !reason.empty() ? reason : lang::tr(lang::keys::error::category, lang::keys::error::seeLog, "See log for details.");
                    notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                    bus::event::publish<event::ShowNoticeEvent>(notice);
                    bus::event::publish<event::CurrentPageChangeEvent>({ui::Page::home});
                }
            });
        });
    }
} // namespace neko::minecraft