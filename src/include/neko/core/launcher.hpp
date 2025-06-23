#pragma once

#include "neko/core/resources.hpp"
#include "neko/log/nlog.hpp"

#include "neko/ui/uiMsg.hpp"

#include "neko/schema/nekodefine.hpp"

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

        if constexpr (std::string_view("custom") == schema::definitions::launcherMode) {
            // Custom launcher
        }

        if constexpr (std::string_view("minecraft") == schema::definitions::launcherMode) {
            try {
                minecraft::account::launcherMinecraftAuthlibAndPrefetchedCheck();
                minecraft::account::launcherMinecraftTokenRefresh();
                minecraft::launcherMinecraft(core::getConfigObj(), onStart, onExit);
            } catch (const ex::NetworkError &e) {
                log::Err(log::SrcLoc::current(), "Network error: %s", e.what());
                internal::displayHint(ui::HintMsg(info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error::minecraftNetworkError), "", {info::lang::translations(info::lang::LanguageKey::Button::ok)}), showHint);
            } catch (const ex::Parse &e) {
                log::Err(log::SrcLoc::current(), "Parse error: %s", e.what());
                internal::displayHint(ui::HintMsg(info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error::jsonParse), "", {info::lang::translations(info::lang::LanguageKey::Button::ok)}), showHint);
            } catch (const ex::OutOfRange &e) {
                log::Err(log::SrcLoc::current(), "Out of range error: %s", e.what());
                internal::displayHint(ui::HintMsg(info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error::minecraftVersionKeyOutOfRange), "", {info::lang::translations(info::lang::LanguageKey::Button::ok)}), showHint);
            } catch (const ex::FileError &e) {
                log::Err(log::SrcLoc::current(), "File error: %s", e.what());
                internal::displayHint(ui::HintMsg(info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error::minecraftFileError), "", {info::lang::translations(info::lang::LanguageKey::Button::ok)}), showHint);
            } catch (const ex::Exception &e) {
                log::Err(log::SrcLoc::current(), "Exception: %s", e.what());
                internal::displayHint(ui::HintMsg(info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error::minecraftException), "", {info::lang::translations(info::lang::LanguageKey::Button::ok)}), showHint);
            } catch (const std::exception &e) {
                log::Err(log::SrcLoc::current(), "Unexpected error: %s", e.what());
                internal::displayHint(ui::HintMsg(info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error::minecraftUnexpected), "", {info::lang::translations(info::lang::LanguageKey::Button::ok)}), showHint);
            } catch (...) {
                log::Err(log::SrcLoc::current(), "An unknown error occurred during the launcher process.");
                internal::displayHint(ui::HintMsg(info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error::minecraftUnknownError), "", {info::lang::translations(info::lang::LanguageKey::Button::ok)}), showHint);
            }
        }
    }
} // namespace neko::core
