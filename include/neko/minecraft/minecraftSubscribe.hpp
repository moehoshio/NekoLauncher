#pragma once

#include <neko/log/nlog.hpp>

#include "neko/app/lang.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/bus/threadBus.hpp"
#include "neko/core/auth.hpp"
#include "neko/core/launcher.hpp"
#include "neko/event/eventTypes.hpp"

namespace neko::minecraft {
    inline void subscribeToMinecraftEvents() {
        (void)bus::event::subscribe<event::LaunchRequestEvent>([](const event::LaunchRequestEvent &) {
            if (!core::auth::isLoggedIn()) {
                ui::NoticeMsg notice;
                notice.title = lang::tr(lang::keys::error::category, lang::keys::error::launchFailed, "Launch Failed");
                notice.message = lang::tr(lang::keys::setting::category, lang::keys::setting::login, "Login required.");
                notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                bus::event::publish<event::ShowNoticeEvent>(notice);
                return;
            }

            ui::LoadingMsg loadingMsg;
            loadingMsg.type = ui::LoadingMsg::Type::OnlyRaw;
            bus::event::publish<event::ShowLoadingEvent>(loadingMsg);
            bus::event::publish<event::LoadingStatusChangedEvent>(
                {lang::tr(lang::keys::loading::category, lang::keys::loading::starting, "Starting...")});

            bus::thread::submit([]() {
                auto publishFailure = [](const std::string &reason, int exitCode) {
                    bus::event::publish<event::LaunchFinishedEvent>({exitCode});
                    bus::event::publish<event::LaunchFailedEvent>({reason, exitCode});

                    ui::NoticeMsg notice;
                    notice.title = lang::tr(lang::keys::error::category, lang::keys::error::launchFailed, "Launch Failed");
                    notice.message = !reason.empty() ? reason : lang::tr(lang::keys::error::category, lang::keys::error::seeLog, "See log for details.");
                    notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                    bus::event::publish<event::ShowNoticeEvent>(notice);
                    bus::event::publish<event::CurrentPageChangeEvent>({ui::Page::home});
                };

                try {
                    core::launcher(
                        []() {
                            bus::event::publish<event::LaunchStartedEvent>({});
                            bus::event::publish<event::CurrentPageChangeEvent>({ui::Page::home});
                        },
                        [publishFailure](int exitCode) {
                            if (exitCode == 0) {
                                bus::event::publish<event::LaunchFinishedEvent>({exitCode});
                                return;
                            }

                            const auto reason = lang::tr(lang::keys::error::category, lang::keys::error::seeLog, "See log for details.") +
                                               " (code " + std::to_string(exitCode) + ")";
                            publishFailure(reason, exitCode);
                        });
                } catch (const ex::Exception &e) {
                    publishFailure(e.what(), -1);
                } catch (const std::exception &e) {
                    publishFailure(e.what(), -1);
                }
            });
        });
    }
} // namespace neko::minecraft