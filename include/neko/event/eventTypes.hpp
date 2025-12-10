/**
 * @brief NekoLc Event Types
 * @file eventTypes.hpp
 * @author moehoshio
 */

#pragma once

#include <neko/schema/types.hpp>

#include "neko/app/api.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/ui/page.hpp"
#include "neko/ui/uiMsg.hpp"

namespace neko::event {

    /*****************/
    /** App Events **/
    /*****************/

    struct NekoStartEvent {};
    struct NekoQuitEvent {};

    struct ConfigLoadedEvent {
        std::string path;
        bool success = false;
    };

    struct ConfigSavedEvent {
        std::string path;
        bool success = false;
    };

    struct ConfigUpdatedEvent {
        neko::ClientConfig config;
    };

    /*****************/
    /*** UI Events ***/
    /*****************/

    struct CurrentPageChangeEvent {
        ui::Page page;
    };
    struct ShowNoticeEvent : public neko::ui::NoticeMsg {
        ShowNoticeEvent(const neko::ui::NoticeMsg &msg) : neko::ui::NoticeMsg(msg) {}
    };
    struct ShowLoadingEvent : public neko::ui::LoadingMsg {
        ShowLoadingEvent(const neko::ui::LoadingMsg &msg) : neko::ui::LoadingMsg(msg) {}
    };
    struct ShowInputEvent : public neko::ui::InputMsg {
        ShowInputEvent(const neko::ui::InputMsg &msg) : neko::ui::InputMsg(msg) {}
    };
    struct LoadingValueChangedEvent {
        neko::uint32 progressValue;
    };
    struct LoadingStatusChangedEvent {
        std::string statusMessage;
    };
    struct LoadingChangedEvent : public LoadingValueChangedEvent, public LoadingStatusChangedEvent {
        LoadingChangedEvent(const std::string &statusMessage, neko::uint32 progressValue)
            : LoadingValueChangedEvent{progressValue}, LoadingStatusChangedEvent{statusMessage} {}
    };
    struct HideInputEvent {};


    /*********************/
    /** Launching Events **/
    /*********************/

    struct LaunchRequestEvent {};
    struct LaunchStartedEvent {};
    struct LaunchFinishedEvent {
        int exitCode = 0;
    };
    struct LaunchFailedEvent {
        std::string reason;
        int exitCode = -1;
    };


    /*****************/
    /** Core Events **/
    /*****************/

    struct MaintenanceEvent : public neko::ui::NoticeMsg {
        MaintenanceEvent() = default;
        MaintenanceEvent(const neko::ui::NoticeMsg &msg) : neko::ui::NoticeMsg(msg) {}
    };
    struct UpdateAvailableEvent : api::UpdateResponse {
        UpdateAvailableEvent() = default;
        explicit UpdateAvailableEvent(const api::UpdateResponse &resp) : api::UpdateResponse(resp) {}
    };
    struct UpdateCompleteEvent {};
    struct UpdateFailedEvent {
        std::string reason;
    };
    
} // namespace neko::event