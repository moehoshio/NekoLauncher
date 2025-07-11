#pragma once
#include "neko/schema/types.hpp"
#include "neko/ui/uiMsg.hpp"

namespace neko::event {

    struct ExitEvent {};

    struct ShowHintEvent : public neko::ui::HintMsg {
        ShowHintEvent(const neko::ui::HintMsg &msg) : neko::ui::HintMsg(msg) {}
    };

    struct ShowLoadEvent : public neko::ui::LoadMsg {
        ShowLoadEvent(const neko::ui::LoadMsg &msg) : neko::ui::LoadMsg(msg) {}
    };

    struct UpdateLoadingEvent {
        std::string process;
        neko::uint32 progressVal;
    };

    struct ShowInputEvent : public neko::ui::InputMsg {
        ShowInputEvent(const neko::ui::InputMsg &msg) : neko::ui::InputMsg(msg) {}
    };
} // namespace neko::event