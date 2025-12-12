// Utility for loading/saving themes from json files and resolving by name
#pragma once

#include <neko/schema/types.hpp>

#include "neko/ui/theme.hpp"

#include <optional>
#include <string>
#include <vector>

namespace neko::ui::themeio {

    // Known built-in names
    inline constexpr neko::cstr kLightName = "Light";
    inline constexpr neko::cstr kDarkName = "Dark";

    // Load theme by name (built-ins first, then themes/<name>.json). Returns nullopt if not found/failed.
    std::optional<Theme> loadThemeByName(const std::string &name, const std::string &themeDir);

    // List available theme names (built-ins + *.json in folder without extension).
    std::vector<std::string> listThemeNames(const std::string &themeDir);

    // Save theme to themeDir/<name>.json (name from theme.info.name). Returns true on success.
    bool saveTheme(const Theme &theme, const std::string &themeDir, std::string &error);

} // namespace neko::ui::themeio
