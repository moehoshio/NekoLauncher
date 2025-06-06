#pragma once
#include "neko/log/nlog.hpp"

#include "neko/schema/wmsg.hpp"

#include "neko/minecraft/account.hpp"
#include "neko/minecraft/launcherMinecraft.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace neko::core {

    // Called when the user clicks.
    inline void launcher(std::function<void(const neko::ui::hintMsg &)> hintFunc, std::function<void()> onStart, std::function<void(int)> onExit) {
        nlog::autoLog log{FI, LI, FN};

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
            launcherMinecraftAuthlibAndPrefetchedCheck(hintFunc);
            if (!launcherMinecraftTokenValidate(hintFunc))
                return;
            launcherMinecraft(exec::getConfigObj(), hintFunc, onStart, onExit);
        }
    }
} // namespace neko::core
