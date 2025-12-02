#pragma once

// Neko Module

#include <neko/log/nlog.hpp>

// NekoLc project
#include "neko/app/nekoLc.hpp"
#include "neko/bus/configBus.hpp"

#include "neko/minecraft/authMinecraft.hpp"
#include "neko/minecraft/launcherMinecraft.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace neko::core {

    // Called when the user clicks.
    inline void launcher() {
        log::autoLog log;

        if constexpr (std::string_view("custom") == lc::LauncherMode) {
            // Custom launcher
        }

        if constexpr (std::string_view("minecraft") == lc::LauncherMode) {
            minecraft::auth::AuthMode authMode = minecraft::auth::AuthMode::AuthlibInjector;
            try {
                if (authMode == minecraft::auth::AuthMode::AuthlibInjector) {
                    minecraft::auth::authMinecraftAuthlibAndPrefetchedCheck();    
                }
                minecraft::auth::authMinecraftTokenRefresh(authMode);
                minecraft::launcherMinecraft(bus::config::getClientConfig());
            } catch (const ex::Exception &e) {
                log::error("Exception: " + std::string(e.what()));
                throw;
            } catch (const std::exception &e) {
                log::error("Unexpected error: " + std::string(e.what()));
                throw;
            }
        }
    }
} // namespace neko::core