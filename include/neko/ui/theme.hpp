#pragma once

#include <neko/schema/types.hpp>

#include <string_view>

namespace neko::ui {

    enum class ThemeType {
        Light,
        Dark,
        Custom
    };

    struct ThemeInfo {
        neko::strview name;
        neko::strview description;
        neko::strview author;
        ThemeType type;
    };

    struct ThemeColors {
        neko::strview primary;
        neko::strview secondary;
        neko::strview background;
        neko::strview text;
        neko::strview accent;
        neko::strview success;
        neko::strview warning;
        neko::strview error;
        neko::strview info;
        neko::strview surface;
        neko::strview disabled;
        neko::strview hover;
        neko::strview focus;
    };

    struct Theme {
        ThemeInfo info;
        ThemeColors colors;
    };

    // default themes
    constexpr Theme lightTheme = {
        ThemeInfo{
            .name = "Light",
            .description = "Light Theme",
            .author = "Hoshi",
            .type = ThemeType::Light},
        ThemeColors{
            .primary = "#1976D2",
            .secondary = "#DC004E",
            .background = "#FFFFFF",
            .text = "#212121",
            .accent = "#FF9800",
            .success = "#4CAF50",
            .warning = "#FF9800",
            .error = "#F44336",
            .info = "#2196F3",
            .surface = "#F5F5F5",
            .disabled = "#BDBDBD",
            .hover = "#E3F2FD",
            .focus = "#1976D2"}};

    constexpr Theme darkTheme = {
        ThemeInfo{
            .name = "Dark",
            .description = "Dark Theme",
            .author = "Hoshi",
            .type = ThemeType::Dark},
        ThemeColors{
            .primary = "#90CAF9",
            .secondary = "#F48FB1",
            .background = "#121212",
            .text = "#FFFFFF",
            .accent = "#FFB74D",
            .success = "#81C784",
            .warning = "#FFB74D",
            .error = "#EF5350",
            .info = "#64B5F6",
            .surface = "#1E1E1E",
            .disabled = "#757575",
            .hover = "#333333",
            .focus = "#90CAF9"}};

    constexpr Theme homeTheme = {
        ThemeInfo{
            .name = "Home",
            .description = "Home Theme",
            .author = "Hoshi",
            .type = ThemeType::Custom},
        ThemeColors{
            .primary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba( 248,248,255,105), stop:0.51 rgba(150,188,215,165), stop:1 rgba( 248,248,255,100))",
            .secondary = "rgba(240, 240, 240, 225)",
            .background = "transparent",
            .text = "#212121",
            .accent = "",
            .success = "",
            .warning = "",
            .error = "",
            .info = "",
            .surface = "#F5F5F5",
            .disabled = "#BDBDBD",
            .hover = "#E3F2FD",
            .focus = "#1976D2"}};

    class ThemeManager {
    public:
        static ThemeManager &instance() {
            static ThemeManager instance;
            return instance;
        }

        void setTheme(const Theme &theme) {
            currentTheme = theme;
        }

        Theme getCurrentTheme() const {
            return currentTheme;
        }

    private:
        ThemeManager() = default;
        Theme currentTheme;
    };

    inline Theme getCurrentTheme() {
        return ThemeManager::instance().getCurrentTheme();
    }

    inline void setCurrentTheme(const Theme &theme) {
        ThemeManager::instance().setTheme(theme);
    }

} // namespace neko::ui