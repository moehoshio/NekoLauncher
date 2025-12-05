#pragma once

#include <atomic>

#include "neko/ui/windows/nekoWindow.hpp"

namespace neko::ui {

    /**
     * @brief Thin bridge that lets non-UI initialization code emit signals on the neko window once it exists.
     */
    class UiEventDispatcher {
    public:
        static void setNekoWindow(window::NekoWindow *window) noexcept {
            nekoWindow.store(window, std::memory_order_release);
        }

        static void clearNekoWindow() noexcept {
            nekoWindow.store(nullptr, std::memory_order_release);
        }

        static window::NekoWindow *getNekoWindow() noexcept {
            return nekoWindow.load(std::memory_order_acquire);
        }

    private:
        static inline std::atomic<window::NekoWindow *> nekoWindow{nullptr};
    };

} // namespace neko::ui
