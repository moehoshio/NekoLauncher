#pragma once

// Neko Module

#include <neko/log/nlog.hpp>

// NekoLc project
#include "neko/ui/uiMsg.hpp"

#include "neko/core/resources.hpp"

#include "neko/minecraft/account.hpp"
#include "neko/minecraft/launcherMinecraft.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace neko::core {

    // Called when the user clicks.
    inline void launcher() {
        log::autoLog log;

        if constexpr (std::string_view("custom") == schema::definitions::LauncherMode) {
            // Custom launcher
        }

        if constexpr (std::string_view("minecraft") == schema::definitions::LauncherMode) {
            try {
                minecraft::account::launcherMinecraftAuthlibAndPrefetchedCheck();
                minecraft::account::launcherMinecraftTokenRefresh();
                minecraft::launcherMinecraft();
            } catch (const ex::Exception &e) {
                log::error({}, "Exception: {}", e.what());
            } catch (const std::exception &e) {
                log::error({}, "Unexpected error: {}", e.what());
            } catch (...) {
                log::error("An unknown error occurred during the launcher process.");
            }
        }
    }
} // namespace neko::core
