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

    namespace internal {
        void displayHint(const ui::HintMsg &hintMsg, std::function<void(const ui::HintMsg &)> showHint) {
            if (showHint) {
                showHint(hintMsg);
            }
        }
    } // namespace internal

    // Called when the user clicks.
    inline void launcher(std::function<void(const neko::ui::HintMsg &)> showHint = nullptr, std::function<void()> onStart = nullptr, std::function<void(int)> onExit = nullptr) {
        nlog::autoLog log;

        if constexpr (std::string_view("custom") == schema::definitions::LauncherMode) {
            // Custom launcher
        }

        if constexpr (std::string_view("minecraft") == schema::definitions::LauncherMode) {
            try {
                minecraft::account::launcherMinecraftAuthlibAndPrefetchedCheck();
                minecraft::account::launcherMinecraftTokenRefresh();
                minecraft::launcherMinecraft(core::getConfigObj(), onStart, onExit);
            } catch (const ex::Exception &e) {
                log::error({}, "Exception: {}", e.what());
                internal::displayHint(ui::HintMsg(info::lang::tr(info::lang::Keys::Title::error), info::lang::tr(info::lang::Keys::Error::minecraftException), "", {info::lang::tr(info::lang::Keys::Button::ok)}), showHint);
            } catch (const std::exception &e) {
                log::error({}, "Unexpected error: {}", e.what());
                internal::displayHint(ui::HintMsg(info::lang::tr(info::lang::Keys::Title::error), info::lang::tr(info::lang::Keys::Error::minecraftUnexpected), "", {info::lang::tr(info::lang::Keys::Button::ok)}), showHint);
            } catch (...) {
                log::error("An unknown error occurred during the launcher process.");
                internal::displayHint(ui::HintMsg(info::lang::tr(info::lang::Keys::Title::error), info::lang::tr(info::lang::Keys::Error::minecraftUnknownError), "", {info::lang::tr(info::lang::Keys::Button::ok)}), showHint);
            }
        }
    }
} // namespace neko::core
