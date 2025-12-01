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

    struct NekoStartEvent {};
    struct NekoQuitEvent {};

    struct ChangeCurrentPageEvent {
        ui::Page page;
    };
    struct ShowHintEvent : public neko::ui::HintMsg {
        ShowHintEvent(const neko::ui::HintMsg &msg) : neko::ui::HintMsg(msg) {}
    };
    struct ShowLoadEvent : public neko::ui::LoadMsg {
        ShowLoadEvent(const neko::ui::LoadMsg &msg) : neko::ui::LoadMsg(msg) {}
    };
    struct ShowInputEvent : public neko::ui::InputMsg {
        ShowInputEvent(const neko::ui::InputMsg &msg) : neko::ui::InputMsg(msg) {}
    };
    struct UpdateLoadingValueEvent {
        neko::uint32 progressValue;
    };
    struct UpdateLoadingStatusEvent {
        std::string process;
    };
    struct UpdateLoadingEvent : public UpdateLoadingValueEvent, public UpdateLoadingStatusEvent {
        UpdateLoadingEvent(const std::string &process, neko::uint32 progressValue)
            : UpdateLoadingValueEvent{progressValue}, UpdateLoadingStatusEvent{process} {}
    };
    

    /*****************/
    /** Core Events **/
    /*****************/

    struct MaintenanceEvent : public neko::ui::HintMsg {
        MaintenanceEvent(const neko::ui::HintMsg &msg) : neko::ui::HintMsg(msg) {}
    };
    struct UpdateAvailableEvent : api::UpdateResponse {};
    struct UpdateCompleteEvent {};
    
} // namespace neko::event