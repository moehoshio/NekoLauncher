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
    /* Global Events */
    /*****************/
    struct NekoStartEvent {};
    struct NekoQuitEvent {};

    /*****************/
    /* Window Events */
    /*****************/
    struct ChangeWindowTitleEvent {
        std::string title;
    };
    struct ChangeWindowIconEvent {
        std::string iconPath;
    };
    struct CloseWindowEvent {};
    struct MinimizeWindowEvent {};
    struct MaximizeWindowEvent {};
    struct RestoreWindowEvent {};
    struct FocusWindowEvent {};

    struct ResizeWindowEvent {
        neko::uint32 width;
        neko::uint32 height;
    };
    struct MoveWindowEvent {
        neko::int32 x;
        neko::int32 y;
    };
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
    struct UpdateLoadingValEvent {
        neko::uint32 progressVal;
    };
    struct UpdateLoadingNowEvent {
        std::string process;
    };
    struct UpdateLoadingEvent : public UpdateLoadingValEvent, public UpdateLoadingNowEvent {
        UpdateLoadingEvent(const std::string &process, neko::uint32 progressVal)
            : UpdateLoadingValEvent{progressVal}, UpdateLoadingNowEvent{process} {}
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