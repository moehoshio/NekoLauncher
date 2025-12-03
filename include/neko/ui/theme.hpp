#pragma once

#include <neko/schema/types.hpp>

#include <string_view>

namespace neko::ui {

    enum class ThemeType {
        Light,
        Dark,
        Custom
    };

    struct Theme {
        neko::strview
            name,
            description,
            author;
        ThemeType type;

        neko::strview
            primaryColor,
            secondaryColor,
            backgroundColor,
            textColor,
            accentColor,
            successColor,
            warningColor,
            errorColor,
            infoColor,
            borderColor,
            surfaceColor,
            disabledColor,
            hoverColor,
            focusColor;
    };

    // default themes
    constexpr Theme lightTheme = {
        .name = "Light",
        .description = "Light Theme",
        .author = "Hoshi",
        .type = ThemeType::Light,
        .primaryColor = "#1976D2",
        .secondaryColor = "#DC004E",
        .backgroundColor = "#FFFFFF",
        .textColor = "#212121",
        .accentColor = "#FF9800",
        .successColor = "#4CAF50",
        .warningColor = "#FF9800",
        .errorColor = "#F44336",
        .infoColor = "#2196F3",
        .borderColor = "#E0E0E0",
        .surfaceColor = "#F5F5F5",
        .disabledColor = "#BDBDBD",
        .hoverColor = "#E3F2FD",
        .focusColor = "#1976D2"
    };

    constexpr Theme darkTheme = {
        .name = "Dark",
        .description = "Dark Theme",
        .author = "Hoshi",
        .type = ThemeType::Dark,
        .primaryColor = "#90CAF9",
        .secondaryColor = "#F48FB1",
        .backgroundColor = "#121212",
        .textColor = "#FFFFFF",
        .accentColor = "#FFB74D",
        .successColor = "#81C784",
        .warningColor = "#FFB74D",
        .errorColor = "#EF5350",
        .infoColor = "#64B5F6",
        .borderColor = "#424242",
        .surfaceColor = "#1E1E1E",
        .disabledColor = "#757575",
        .hoverColor = "#333333",
        .focusColor = "#90CAF9"
    };

    constexpr Theme homeTheme = {
        .name = "Home",
        .description = "Home Theme",
        .author = "Hoshi",
        .type = ThemeType::Custom,
        .primaryColor = "#1976D2",
        .secondaryColor = "#DC004E",
        .backgroundColor = "#FFFFFF",
        .textColor = "#212121",
        .accentColor = "#FF9800",
        .successColor = "#4CAF50",
        .warningColor = "#FF9800",
        .errorColor = "#F44336",
        .infoColor = "#2196F3",
        .borderColor = "#E0E0E0",
        .surfaceColor = "#F5F5F5",
        .disabledColor = "#BDBDBD",
        .hoverColor = "#E3F2FD",
        .focusColor = "#1976D2"
    };

    class ThemeManager {
    public:
        static ThemeManager &instance() {
            static ThemeManager instance;
            return instance;
        }

        void setTheme(const Theme &theme) {
            currentTheme = theme;
        }

        const Theme &getCurrentTheme() const {
            return currentTheme;
        }
        
    private:
        ThemeManager() = default;
        Theme currentTheme;
    };

} // namespace neko::ui