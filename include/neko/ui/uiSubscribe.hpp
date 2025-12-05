#pragma once

#include <QtCore/QCoreApplication>

#include <neko/log/nlog.hpp>

#include "neko/ui/uiMsg.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"
#include "neko/ui/uiEventDispatcher.hpp"

namespace neko::ui {

    inline void subscribeToUiEvent() {

        bus::event::subscribe<event::UpdateCompleteEvent>(
            [](const event::UpdateCompleteEvent &) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->switchToPageD(Page::home);
                    emit nekoWindow->showNoticeD(NoticeMsg{
                        .title = "Update Complete",
                        .message = "The application has been updated successfully.\nPlease restart to apply the changes.\nThank you for using NekoLauncher!",
                        .posterPath = "",
                        .buttonText = {
                            "OK",
                            "Close"
                        },
                        .callback = [=](neko::uint32 i){
                            log::info("User clicked button index: " + std::to_string(i));
                            // emit nekoWindow->showInputD(InputMsg{
                            //     .title = "Restart Required",
                            //     .message = "Please restart the application to complete the update process.",
                            //     .posterPath = "",
                            //     .lineText = {
                            //         "Restart Now",
                            //         "Later"
                            //     },
                            //     .callback = [=](bool v){
                            //         if (!v){
                            //             emit nekoWindow->hideInputD();
                            //             return;
                            //         }

                            //         auto input = emit nekoWindow->getLinesD();
                            //         for (const auto &line : input) {
                            //             log::info("User selected option: " + line);
                            //         }
                            //     }
                            // });
                        }
                    });
                }
            });

        bus::event::subscribe<event::LoadingValueChangedEvent>(
            [](const event::LoadingValueChangedEvent &e) {
                if (auto nekoWindow = UiEventDispatcher::getNekoWindow()) {
                    emit nekoWindow->setLoadingValueD(static_cast<neko::uint32>(e.progressValue));
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
