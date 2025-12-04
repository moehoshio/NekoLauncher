#pragma once

#include <atomic>

#include "neko/ui/windows/nekoWindow.hpp"

namespace neko::ui {

    /**
     * @brief Thin bridge that lets non-UI initialization code emit signals on the main window once it exists.
     */
    class UiEventDispatcher {
    public:
        static void setMainWindow(window::NekoWindow *window) noexcept {
            mainWindow.store(window, std::memory_order_release);
        }

        static void clearMainWindow() noexcept {
            mainWindow.store(nullptr, std::memory_order_release);
        }

        static window::NekoWindow *getMainWindow() noexcept {
            return mainWindow.load(std::memory_order_acquire);
        }

    private:
        static inline std::atomic<window::NekoWindow *> mainWindow{nullptr};
    };

} // namespace neko::ui
