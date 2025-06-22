#pragma once

#include "neko/log/nlog.hpp"

#include "neko/ui/uiMsg.hpp"

#include "neko/schema/nekodefine.hpp"

#include "neko/minecraft/account.hpp"
#include "neko/minecraft/launcherMinecraft.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace neko::core {

    // Called when the user clicks.
    inline void launcher(std::function<void(const neko::ui::HintMsg &)> showHint = nullptr, std::function<void()> onStart = nullptr, std::function<void(int)> onExit = nullptr) {
        nlog::autoLog log;

        if constexpr (std::string_view("custom") == schema::definitions::launcherMode) {
            // Custom launcher
            // auto customLauncher = exec::getConfigObj().getCustomLauncher();
            // if (customLauncher.empty()) {
            //     hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.customLauncher), "", 1});
            //     return;
            // }
            // std::string command = "\"" + customLauncher + "\"";
            // launcherProcess(command, onStart, onExit);
        }

        if constexpr (std::string_view("minecraft") == schema::definitions::launcherMode) {
            minecraft::launcherMinecraftAuthlibAndPrefetchedCheck(showHint);
            if (!minecraft::launcherMinecraftTokenValidate(showHint))
                return;
            minecraft::launcherMinecraft(exec::getConfigObj(), showHint, onStart, onExit);
        }
    }
} // namespace neko::core
