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
        neko::strview name;
        ThemeType type;
        neko::strview backgroundColor;
        neko::strview textColor;      // Main text and button text
        neko::strview accentColor;    // Accent color, for buttons, focus, links, etc.
        neko::strview hoverColor;     // For mouse hover
        neko::strview selectedColor;  // For selected (highlighted) state
        neko::strview disabledColor;  // For disabled state
        neko::strview borderColor;    // Border color
        neko::strview separatorColor; // Separator line color
        neko::strview borderRadius;   // Border radius, can be pixel value or percentage
        neko::strview fontFamily;
        int fontSize;
        int padding;
    };

    // default themes
    constexpr Theme LightTheme = {
        "Light",
        ThemeType::Light,
        "rgba(255,255,255,1)", // backgroundColor
        "rgba(0,0,0,1)",       // textColor
        "rgba(0,123,255,1)",   // accentColor
        "rgba(224,224,224,1)", // hoverColor
        "rgba(208,208,208,1)", // selectedColor
        "rgba(160,160,160,1)", // disabledColor
        "rgba(204,204,204,1)", // borderColor
        "rgba(238,238,238,1)", // separatorColor
        "25%",                 // borderRadius
        "Arial",               // fontFamily
        12,                    // fontSize
        10                     // padding
    };

    constexpr Theme DarkTheme = {
        "Dark",
        ThemeType::Dark,
        "rgba(24,26,27,1)",    // backgroundColor
        "rgba(245,246,250,1)", // textColor
        "rgba(79,142,247,1)",  // accentColor
        "rgba(35,39,42,1)",    // hoverColor
        "rgba(44,47,51,1)",    // selectedColor
        "rgba(85,89,92,1)",    // disabledColor
        "rgba(54,57,63,1)",    // borderColor
        "rgba(35,39,42,1)",    // separatorColor
        "25%",                 // borderRadius
        "Segoe UI",            // fontFamily
        12,                    // fontSize
        10                     // padding
    };

    constexpr Theme homeTheme = {
        "Home",
        ThemeType::Custom,
        "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(238, 130, 238, 255), stop:0.33 rgba(155,120,236,255) , stop:0.75 rgba(79,146,245,255),stop:1 rgba(40,198, 177,255))", // backgroundColor
        "rgba(50,50,50,1)",    // textColor
        "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba( 248,248,255,105), stop:0.51 rgba(150,188,215,165), stop:1 rgba( 248,248,255,100))", // accentColor
        "rgba(200,200,200,1)", // hoverColor
        "rgba(180,180,180,1)", // selectedColor
        "rgba(150,150,150,1)", // disabledColor
        "rgba(220,220,220,1)", // borderColor
        "rgba(200,200,200,1)", // separatorColor
        "30%",                 // borderRadius
        "Helvetica",           // fontFamily
        12,                    // fontSize
        10                     // padding
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