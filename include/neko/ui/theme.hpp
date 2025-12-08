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
        neko::strview canvas; // large area/window backdrop
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
            .disabled = "#cbd5e1",
            .hover = "rgba(37,99,235,0.14)",
            .focus = "rgba(14,165,233,0.32)"}};

    constexpr Theme darkTheme = {
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
            .disabled = "#4b5563",
            .hover = "rgba(255,255,255,0.08)",
            .focus = "rgba(56,189,248,0.32)"}};

    constexpr Theme homeTheme = {
        ThemeInfo{
            .name = "Home",
            .description = "Home Theme",
            .author = "Hoshi",
            .type = ThemeType::Custom},
        ThemeColors{
            .primary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #b7c7ff, stop:1 #6ec1ff)",
            .secondary = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #ffe0b2, stop:1 #ffb374)",
            .background = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #fdf2ff, stop:0.35 #dbeafe, stop:0.7 #cffafe, stop:1 #dcfce7)",
            .canvas = "qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 #fdf2ff, stop:0.33 #fce7f3, stop:0.66 #dbeafe, stop:1 #e0f2fe)",
            .text = "#1f2937",
            .accent = "#f97316",
            .success = "#16a34a",
            .warning = "#f59e0b",
            .error = "#ef4444",
            .info = "#0ea5e9",
            .surface = "rgba(255,255,255,0.96)",
            .disabled = "#cbd5e1",
            .hover = "rgba(15,118,110,0.12)",
            .focus = "rgba(14,165,233,0.28)"}};

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