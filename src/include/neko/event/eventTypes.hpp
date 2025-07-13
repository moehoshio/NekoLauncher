#pragma once

#include "neko/schema/types.hpp"
#include "neko/ui/uiMsg.hpp"

namespace neko::event {

    struct StartEvent {};

    struct ExitEvent {};

    struct ShowHintEvent : public neko::ui::HintMsg {
        ShowHintEvent(const neko::ui::HintMsg &msg) : neko::ui::HintMsg(msg) {}
    };

    struct ShowLoadEvent : public neko::ui::LoadMsg {
        ShowLoadEvent(const neko::ui::LoadMsg &msg) : neko::ui::LoadMsg(msg) {}
    };

    struct UpdateLoadingValEvent {
        neko::uint32 progressVal = 0;
    };

    struct UpdateLoadingNowEvent {
        std::string process;
    };

    struct UpdateLoadingEvent : public UpdateLoadingValEvent, public UpdateLoadingNowEvent {
        UpdateLoadingEvent(const std::string &process, neko::uint32 progressVal = 0)
            : UpdateLoadingValEvent{progressVal}, UpdateLoadingNowEvent{process} {}
    };

    struct ShowInputEvent : public neko::ui::InputMsg {
        ShowInputEvent(const neko::ui::InputMsg &msg) : neko::ui::InputMsg(msg) {}
    };
} // namespace neko::event