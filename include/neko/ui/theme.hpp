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
            .primary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(3,169,244,255), stop:0.5 rgba(66,165,245,220), stop:1 rgba(144,202,249,200))",
            .secondary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(255,128,171,220), stop:0.6 rgba(255,183,197,200), stop:1 rgba(255,224,178,180))",
            .background = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255,255,255,255), stop:1 rgba(245,250,255,240))",
            .text = "rgba(33,33,33,230)",
            .accent = "rgba(255,152,0,230)",
            .success = "rgba(76,175,80,220)",
            .warning = "rgba(255,152,0,220)",
            .error = "rgba(244,67,54,220)",
            .info = "rgba(33,150,243,220)",
            .surface = "rgba(245,245,245,255)",
            .disabled = "rgba(189,189,189,220)",
            .hover = "rgba(227,242,253,255)",
            .focus = "rgba(25,118,210,200)"}};

    constexpr Theme darkTheme = {
        ThemeInfo{
            .name = "Dark",
            .description = "Dark Theme",
            .author = "Hoshi",
            .type = ThemeType::Dark},
        ThemeColors{
            .primary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(100,181,246,240), stop:0.6 rgba(41,121,255,220), stop:1 rgba(26,35,126,200))",
            .secondary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(244,143,177,220), stop:1 rgba(171,71,188,200))",
            .background = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(28,28,30,255), stop:1 rgba(16,18,25,230))",
            .text = "rgba(255,255,255,230)",
            .accent = "rgba(255,183,77,210)",
            .success = "rgba(129,199,132,210)",
            .warning = "rgba(255,183,77,210)",
            .error = "rgba(239,83,80,220)",
            .info = "rgba(100,181,246,210)",
            .surface = "rgba(30,30,30,255)",
            .disabled = "rgba(117,117,117,200)",
            .hover = "rgba(51,51,51,230)",
            .focus = "rgba(144,202,249,200)"}};

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