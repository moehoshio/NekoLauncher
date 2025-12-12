#include "neko/ui/themeIO.hpp"

#include <neko/app/nekoLc.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <optional>

namespace fs = std::filesystem;

namespace neko::ui::themeio {

    namespace {
        std::string typeToString(ThemeType t) {
            switch (t) {
                case ThemeType::Light: return "light";
                case ThemeType::Dark: return "dark";
                default: return "custom";
            }
        }

        ThemeType typeFromString(const std::string &s) {
            const auto lower = [&]() {
                std::string out = s;
                for (auto &c : out) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
                return out;
            }();
            if (lower == "light") return ThemeType::Light;
            if (lower == "dark") return ThemeType::Dark;
            return ThemeType::Custom;
        }

        nlohmann::json toJson(const Theme &t) {
            nlohmann::json j;
            j["info"] = {
                {"name", t.info.name},
                {"description", t.info.description},
                {"author", t.info.author},
                {"type", typeToString(t.info.type)}
            };
            j["colors"] = {
                {"primary", t.colors.primary},
                {"secondary", t.colors.secondary},
                {"background", t.colors.background},
                {"canvas", t.colors.canvas},
                {"text", t.colors.text},
                {"accent", t.colors.accent},
                {"success", t.colors.success},
                {"warning", t.colors.warning},
                {"error", t.colors.error},
                {"info", t.colors.info},
                {"surface", t.colors.surface},
                {"panel", t.colors.panel},
                {"disabled", t.colors.disabled},
                {"hover", t.colors.hover},
                {"focus", t.colors.focus}
            };
            return j;
        }

        std::optional<Theme> fromJson(const nlohmann::json &j) {
            if (!j.contains("info") || !j.contains("colors")) return std::nullopt;
            Theme t;
            const auto &info = j["info"];
            const auto &c = j["colors"];
            auto getStr = [](const nlohmann::json &obj, const char *key, const char *defVal = "") {
                return obj.contains(key) && obj[key].is_string() ? obj[key].get<std::string>() : std::string(defVal);
            };
            t.info.name = getStr(info, "name");
            t.info.description = getStr(info, "description");
            t.info.author = getStr(info, "author");
            t.info.type = typeFromString(getStr(info, "type", "custom"));

            t.colors.primary = getStr(c, "primary");
            t.colors.secondary = getStr(c, "secondary");
            t.colors.background = getStr(c, "background");
            t.colors.canvas = getStr(c, "canvas");
            t.colors.text = getStr(c, "text");
            t.colors.accent = getStr(c, "accent");
            t.colors.success = getStr(c, "success");
            t.colors.warning = getStr(c, "warning");
            t.colors.error = getStr(c, "error");
            t.colors.info = getStr(c, "info");
            t.colors.surface = getStr(c, "surface");
            t.colors.panel = getStr(c, "panel");
            t.colors.disabled = getStr(c, "disabled");
            t.colors.hover = getStr(c, "hover");
            t.colors.focus = getStr(c, "focus");
            return t;
        }

        std::optional<Theme> builtinByName(const std::string &name) {
            auto lower = name;
            for (auto &c : lower) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            if (lower == "light") return lightTheme;
            if (lower == "dark") return darkTheme;
            return std::nullopt;
        }

        fs::path ensureFolder(const std::string &themeDir) {
            fs::path dir(themeDir);
            if (dir.empty()) dir = fs::path(lc::ThemeFolderName.data());
            std::error_code ec;
            fs::create_directories(dir, ec);
            (void)ec;
            return dir;
        }
    } // namespace

    std::optional<Theme> loadThemeByName(const std::string &name, const std::string &themeDir) {
        if (name.empty()) return std::nullopt;
        if (auto b = builtinByName(name)) return b;

        const auto dir = ensureFolder(themeDir);
        fs::path filePath = dir / (name + ".json");
        if (!fs::exists(filePath)) return std::nullopt;
        std::ifstream ifs(filePath);
        if (!ifs.is_open()) return std::nullopt;
        nlohmann::json j;
        try {
            ifs >> j;
        } catch (...) {
            return std::nullopt;
        }
        return fromJson(j);
    }

    std::vector<std::string> listThemeNames(const std::string &themeDir) {
        std::vector<std::string> names = {kLightName, kDarkName};
        const auto dir = ensureFolder(themeDir);
        if (!fs::exists(dir)) return names;
        for (const auto &entry : fs::directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;
            const auto path = entry.path();
            if (path.extension() == ".json") {
                names.push_back(path.stem().string());
            }
        }
        return names;
    }

    bool saveTheme(const Theme &theme, const std::string &themeDir, std::string &error) {
        auto name = theme.info.name;
        if (name.empty()) name = "Custom";
        const auto dir = ensureFolder(themeDir);
        const fs::path filePath = dir / (name + ".json");
        std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
        if (!ofs.is_open()) {
            error = "Failed to open " + filePath.string();
            return false;
        }
        try {
            ofs << toJson(theme).dump(4);
        } catch (const std::exception &e) {
            error = e.what();
            return false;
        }
        return true;
    }

} // namespace neko::ui::themeio
