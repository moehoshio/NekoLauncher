#pragma once

#include <neko/schema/types.hpp>

#include <string>

namespace neko::ui {

    enum class ThemeType {
        Light,
        Dark,
        Custom
    };

    struct ThemeInfo {
        std::string name;
        std::string description;
        std::string author;
        ThemeType type;
    };

    struct ThemeColors {
        std::string primary;
        std::string secondary;
        std::string background;
        std::string canvas; // large area/window backdrop
        std::string text;
        std::string accent;
        std::string success;
        std::string warning;
        std::string error;
        std::string info;
        std::string surface;
        std::string panel;   // card/dialog backgrounds
        std::string disabled;
        std::string hover;
        std::string focus;
    };

    struct Theme {
        ThemeInfo info;
        ThemeColors colors;
    };

    // default themes
    inline const Theme lightTheme = {
        ThemeInfo{
            .name = "Light",
            .description = "Light Theme",
            .author = "Hoshi",
            .type = ThemeType::Light},
        ThemeColors{
            .primary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #8cc5ff, stop:1 #4f93ff)",
            .secondary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #7ae0d6, stop:1 #36cfc9)",
            .background = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #f9fafb, stop:1 #eef2f7)",
            .canvas = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #fff1d6, stop:0.28 #f6c4ff, stop:0.56 #c7ddff, stop:0.8 #8ee0ff, stop:1 #c7f9ff)",
            .text = "#0f172a",
            .accent = "#f59e0b",
            .success = "#16a34a",
            .warning = "#fbbf24",
            .error = "#dc2626",
            .info = "#0ea5e9",
            .surface = "rgba(255,255,255,0.94)",
            .panel = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255,255,255,0.98), stop:0.45 rgba(246,250,255,0.96), stop:1 rgba(230,238,250,0.94))",
            .disabled = "#cbd5e1",
            .hover = "rgba(37,99,235,0.14)",
            .focus = "rgba(14,165,233,0.32)"}};

    inline const Theme darkTheme = {
        ThemeInfo{
            .name = "Dark",
            .description = "Dark Theme",
            .author = "Hoshi",
            .type = ThemeType::Dark},
        ThemeColors{
            .primary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #5ba8ff, stop:1 #2563eb)",
            .secondary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #22d3ee, stop:1 #10b981)",
            .background = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #0b1220, stop:1 #0f172a)",
            .canvas = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #090f1c, stop:0.28 #111c34, stop:0.58 #0e2a47, stop:0.78 #0f3c5f, stop:1 #0f172a)",
            .text = "#e5e7eb",
            .accent = "#f59e0b",
            .success = "#22c55e",
            .warning = "#f59e0b",
            .error = "#f87171",
            .info = "#38bdf8",
            .surface = "#1f2937",
            .panel = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(32,38,50,0.96), stop:0.5 rgba(26,33,45,0.94), stop:1 rgba(22,30,42,0.9))",
            .disabled = "#4b5563",
            .hover = "rgba(255,255,255,0.08)",
            .focus = "rgba(56,189,248,0.32)"}};

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
        ThemeManager() : currentTheme(lightTheme) {}
        Theme currentTheme;
    };

    inline Theme getCurrentTheme() {
        return ThemeManager::instance().getCurrentTheme();
    }

    inline void setCurrentTheme(const Theme &theme) {
        ThemeManager::instance().setTheme(theme);
    }

} // namespace neko::ui