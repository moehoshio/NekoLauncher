/**
 * @brief NekoLc Event Types
 * @file eventTypes.hpp
 * @author moehoshio
 */

#pragma once

#include <neko/schema/types.hpp>

#include "neko/app/api.hpp"
#include "neko/ui/page.hpp"
#include "neko/ui/uiMsg.hpp"

namespace neko::event {

    /*****************/
    /** App Events **/
    /*****************/

    struct NekoStartEvent {};
    struct NekoQuitEvent {};

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


    /*****************/
    /** Core Events **/
    /*****************/

    struct MaintenanceEvent : public neko::ui::NoticeMsg {
        MaintenanceEvent(const neko::ui::NoticeMsg &msg) : neko::ui::NoticeMsg(msg) {}
    };
    struct UpdateAvailableEvent : api::UpdateResponse {};
    struct UpdateCompleteEvent {};
    
} // namespace neko::event