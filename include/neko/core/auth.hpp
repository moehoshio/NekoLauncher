#pragma once

#include "neko/app/nekoLc.hpp"
#include "neko/minecraft/authMinecraft.hpp"

namespace neko::core::auth {

    inline bool isLoggedIn() {
        if constexpr (lc::LauncherMode == "minecraft") {
            return minecraft::auth::isLoggedIn();
        }
    }

    inline std::string getDisplayName() {
        if constexpr (lc::LauncherMode == "minecraft") {
            return minecraft::auth::getPlayerName();
        }
    }

    inline auto authLogin(const std::vector<std::string> &inData) {
        if constexpr (lc::LauncherMode == "minecraft") {
            minecraft::auth::AuthMode authMode = minecraft::auth::AuthMode::AuthlibInjector;
            return minecraft::auth::authLogin(inData, authMode);
        }
    }

    inline void authLogout() {
        if constexpr (lc::LauncherMode == "minecraft") {
            minecraft::auth::AuthMode authMode = minecraft::auth::AuthMode::AuthlibInjector;
            minecraft::auth::authLogout(authMode);
        }
    }

    inline void validAndRefreshLogin() {
        if constexpr (lc::LauncherMode == "minecraft") {
            minecraft::auth::authMinecraftAuthlibAndPrefetchedCheck();
        }
    }
} // namespace neko::core::auth