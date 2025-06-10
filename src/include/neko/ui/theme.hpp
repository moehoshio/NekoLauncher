#pragma once

#include <string_view>

namespace neko::ui {

    enum class ThemeType {
        Light,
        Dark,
        Custom
    };

    struct Theme {
        std::string_view name;
        ThemeType type;
        std::string_view backgroundColor;
        std::string_view secondaryColor; // Secondary color, e.g. button background
        std::string_view textColor;      // Main text and button text
        std::string_view accentColor;    // Accent color, for buttons, focus, links, etc.
        std::string_view hoverColor;     // For mouse hover
        std::string_view selectedColor;  // For selected (highlighted) state
        std::string_view disabledColor;  // For disabled state
        std::string_view borderColor;    // Border color
        std::string_view separatorColor; // Separator line color
        std::string_view borderRadius;   // Border radius, can be pixel value or percentage
        std::string_view fontFamily;
        int fontSize;
        int padding;

        setTheme(std::string_view n, ThemeType t, std::string_view bgColor, std::string_view txtColor, std::string_view accColor, std::string_view hvrColor, std::string_view selColor, std::string_view disColor, std::string_view brdColor, std::string_view sepColor, std::string_view brdRadius, std::string_view fntFamily, int fntSize, int pad)
            : name(n), type(t),
              backgroundColor(bgColor), textColor(txtColor), accentColor(accColor),
              hoverColor(hvrColor), selectedColor(selColor), disabledColor(disColor),
              borderColor(brdColor), separatorColor(sepColor), borderRadius(brdRadius),
              fontFamily(fntFamily), fontSize(fntSize), padding(pad) {}
        Theme &setName(std::string_view n) {
            name = n;
            return *this;
        }
        Theme &setType(ThemeType t) {
            type = t;
            return *this;
        }

        Theme &setBackgroundColor(std::string_view bgColor) {
            backgroundColor = bgColor;
            return *this;
        }
        Theme &setSecondaryColor(std::string_view secColor) {
            secondaryColor = secColor;
            return *this;
        }
        Theme &setTextColor(std::string_view txtColor) {
            textColor = txtColor;
            return *this;
        }
        Theme &setAccentColor(std::string_view accColor) {
            accentColor = accColor;
            return *this;
        }
        Theme &setHoverColor(std::string_view hvrColor) {
            hoverColor = hvrColor;
            return *this;
        }
        Theme &setSelectedColor(std::string_view selColor) {
            selectedColor = selColor;
            return *this;
        }
        Theme &setDisabledColor(std::string_view disColor) {
            disabledColor = disColor;
            return *this;
        }
        Theme &setBorderColor(std::string_view brdColor) {
            borderColor = brdColor;
            return *this;
        }
        Theme &setSeparatorColor(std::string_view sepColor) {
            separatorColor = sepColor;
            return *this;
        }
        Theme &setBorderRadius(std::string_view brdRadius) {
            borderRadius = brdRadius;
            return *this;
        }
        Theme &setFontFamily(std::string_view fntFamily) {
            fontFamily = fntFamily;
            return *this;
        }
        Theme &setFontSize(int fntSize) {
            fontSize = fntSize;
            return *this;
        }
        Theme &setPadding(int pad) {
            padding = pad;
            return *this;
        }
    };

    // default themes
    constexpr Theme LightTheme = {
        "Light",
        ThemeType::Light,
        "rgba(255,255,255,1)", // backgroundColor
        "rgba(240,240,240,1)", // secondaryColor
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
        "rgba(240,240,240,1)", // secondaryColor
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
        "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba( 248,248,255,105), stop:0.51 rgba(150,188,215,165), stop:1 rgba( 248,248,255,100))", // secondaryColor
        "rgba(50,50,50,1)",    // textColor
        "rgb(224, 100, 255)", // accentColor
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