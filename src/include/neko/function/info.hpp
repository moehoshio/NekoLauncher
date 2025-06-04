/**
 * @file info.hpp
 * @brief Provides system, application and language information utilities.
 * @author moehoshio
 * @date 2025-06-05
 *
 * This header contains utilities for:
 * - System information (OS, architecture, paths)
 * - Application configuration and version information
 * - Language/localization support
 *
 * @dependencies
 * - exec.hpp: Used for path unification and file operations
 * - nlog.hpp: Used for logging
 * - clientconfig.hpp: Used for loading configuration
 * - nekodefine.hpp: Contains version definitions
 * - nlohmann/json.hpp: Used for JSON parsing in language support
 */
#pragma once

// Neko Modules
#include "neko/log/nlog.hpp"

#include "neko/function/exec.hpp"

#include "neko/schema/clientconfig.hpp"
#include "neko/schema/nekodefine.hpp"

#include "library/nlohmann/json.hpp"

// C++ Standard Library
#include <filesystem>
#include <string>
#include <string_view>

#include <optional>

/**
 * @namespace neko::info
 * @brief Contains utilities for system, application and language information.
 */
namespace neko::info {

    /**
     * @namespace neko::info::system
     * @brief System-related information and utilities.
     */
    namespace system {

        /**
         * @struct PlatformInfo
         * @brief Provides static platform identification information.
         *
         * Contains compile-time constants for operating system name and architecture.
         */
        struct PlatformInfo {

            /**
             * @brief Operating system name identifier.
             * @return String representing the OS ("windows", "osx", "linux", or "unknown")
             */
            constexpr static const char *osName =
#if defined(_WIN32)
                "windows";
#elif defined(__APPLE__)
                "osx";
#elif defined(__linux__)
                "linux";
#else
                "unknown";
#endif

            /**
             * @brief CPU architecture identifier.
             * @return String representing the architecture ("x64", "x86", "arm64", "arm", or "unknown")
             */
            constexpr static const char *osArch =
#if defined(__x86_64__) || defined(_M_X64)
                "x64";
#elif defined(__i386__) || defined(_M_IX86)
                "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
                "arm64";
#elif defined(__arm__) || defined(_M_ARM)
                "arm";
#else
                    "unknown";
#endif
        }; // class PlatformInfo

        /**
         * @brief Gets or sets the temporary directory path.
         * @param setTempDir Optional parameter to set a new temporary directory.
         * @return The current temporary directory path.
         *
         * @details Uses exec::unifiedPaths from exec.hpp for path normalization.
         * Creates the directory if it doesn't exist.
         */
        inline std::string temporaryFolder(const std::string &setTempDir = "") {
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);

            auto init = []() -> std::string {
                ClientConfig cfg(exec::getConfigObj());
                if (std::filesystem::is_directory(cfg.more.tempDir))
                    return std::string(cfg.more.tempDir) | exec::unifiedPaths;
                else
                    return (std::filesystem::temp_directory_path().string() + "/Nekolc") | exec::unifiedPaths;
            };

            static std::string tempDir = init();

            if (!setTempDir.empty() && std::filesystem::is_directory(setTempDir)) {
                tempDir = setTempDir | exec::unifiedPaths;
            }

            if (!std::filesystem::exists(tempDir))
                std::filesystem::create_directory(tempDir);

            return tempDir;
        }

        /**
         * @brief Gets or sets the current working directory.
         * @param setPath Optional parameter to set a new working directory.
         * @return The current working directory path.
         *
         * @details Uses exec::unifiedPaths from exec.hpp for path normalization.
         */
        inline std::string workPath(const std::string &setPath = "") {
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);
            if (!setPath.empty() && std::filesystem::is_directory(setPath))
                std::filesystem::current_path(setPath);

            return std::filesystem::current_path().string() | exec::unifiedPaths;
        }

        /**
         * @brief Gets the user's home directory.
         * @return The path to the user's home directory.
         *
         * @details Uses exec::unifiedThePaths from exec.hpp for path normalization.
         * Platform-specific implementation using environment variables.
         */
        inline std::optional<std::string> getHome() {
            const char *path = std::getenv(
#ifdef _WIN32
                "USERPROFILE"
#else
                "HOME"
#endif
            );
            if (path)
                return exec::unifiedThePaths<std::string>(path);
            return std::nullopt;
        }

        /**
         * @brief Gets the operating system name.
         * @return String identifier for the operating system.
         */
        constexpr inline const char *getOsName() {
            return PlatformInfo::osName;
        }

        /**
         * @brief Gets the system architecture.
         * @return String identifier for the CPU architecture.
         */
        constexpr inline const char *getOsArch() {
            return PlatformInfo::osArch;
        }

    } // namespace system

    /**
     * @namespace neko::info::app
     * @brief Application-specific information and utilities.
     */
    namespace app {

        /**
         * @brief The application version string.
         * @details Defined in nekodefine.hpp.
         */
        constexpr static const char *version = NekoLcCoreVersionDefine;

        /**
         * @brief The default configuration file name.
         */
        constexpr static const char *configFileName = "config.ini";

        /**
         * @brief Gets the application version.
         * @return The application version string.
         */
        constexpr inline const char *getVersion() {
            return version;
        };

        /**
         * @brief Gets the resource version from configuration.
         * @return The resource version string.
         *
         * @details Uses exec::getConfigObj from exec.hpp to access configuration.
         */
        inline std::string getResVersion() {
            ClientConfig cfg(exec::getConfigObj());
            return cfg.more.resourceVersion;
        }

        /**
         * @brief Gets the configuration file name.
         * @return The configuration file name.
         */
        constexpr inline const char *getConfigFileName() {
            return configFileName;
        }

    } // namespace app

    /**
     * @namespace neko::info::lang
     * @brief Language and localization utilities.
     */
    namespace lang {

        /**
         * @struct LanguageKey
         * @brief Contains all translation keys organized by category.
         *
         * Provides a structured way to access translation keys to ensure
         * consistency across the application.
         */
        struct LanguageKey {
            /**
             * @struct General
             * @brief General-purpose UI translation keys.
             */
            struct General {
                std::string_view
                    general = "general_general",
                    menu = "general_menu",
                    start = "general_start",
                    ok = "general_ok",
                    setting = "general_setting",
                    lang = "genreal_lang",
                    close = "general_close",
                    maximize = "general_maximize",
                    minimize = "general_minimize",
                    login = "general_login",
                    logout = "general_logout",
                    account = "general_account",
                    username = "general_username",
                    password = "general_password",
                    background = "general_background",
                    style = "general_style",
                    window = "general_window",
                    launcher = "general_launcher",
                    network = "general_network",
                    proxy = "general_proxy",
                    more = "general_more",
                    none = "general_none",
                    image = "general_image",
                    type = "general_type",
                    font = "general_font",
                    width = "general_width",
                    height = "general_height",
                    size = "general_windowSize",
                    blurHint = "general_blurHint",
                    animation = "general_animation",
                    performance = "general_performance",
                    quality = "general_quality",
                    blurValue = "general_blurValue",
                    launcherMode = "general_launcherMode",
                    keepWindow = "general_keepWindow",
                    endProcess = "general_endProcess",
                    barKeepRight = "general_barKeepRight",
                    customTempDir = "general_customTempDir",
                    hideAndOverReShow = "general_hideAndOverReShow",
                    useSystemWindowFrame = "general_useSystemWindowFrame",
                    notAutoSetThreadNums = "general_notAutoSetThreadNums",
                    notLogin = "general_notLogin",
                    pointSize = "general_pointSize",
                    tempDir = "general_tempDir",
                    proxyPlaceholder = "general_proxyPlaceholder",
                    updateOverReStart = "general_updateOverReStart",
                    needLogin = "general_needLogin",
                    notEnoughParameters = "general_notEnoughParameters",
                    incompleteApplied = "general_incompleteApplied",
                    loginOrRegister = "general_loginOrRegister",
                    logoutConfirm = "general_logoutConfirm",
                    installMinecraft = "general_installMinecraft";
            };
            /**
             * @struct Title
             * @brief Window and dialog title translation keys.
             */
            struct Title {
                std::string_view
                    error = "title_error",
                    warning = "title_warning",
                    maintenance = "title_maintenance",
                    reStart = "title_reStart",
                    incomplete = "title_incomplete",
                    notLogin = "title_notLogin",
                    inputLogin = "title_inputLogin",
                    inputNotEnoughParameters = "title_inputNotEnoughParameters",
                    loginOrRegister = "title_loginOrRegister",
                    logoutConfirm = "title_logoutConfirm";
            };

            /**
             * @struct Loading
             * @brief Loading-related message translation keys.
             */
            struct Loading {
                std::string_view
                    maintenanceInfoReq = "loading_maintenanceInfoReq",
                    maintenanceInfoParse = "loading_maintenanceInfoParse",
                    downloadMaintenancePoster = "loading_downloadMaintenancePoster",
                    checkUpdate = "loading_checkUpdate",
                    updateInfoParse = "loading_updateInfoParse",
                    downloadUpdatePoster = "loading_downloadUpdatePoster",
                    settingDownload = "loading_settingDownload",
                    downloadUpdate = "loading_downloadUpdate";
            };
            /**
             * @struct Network
             * @brief Network-related message translation keys.
             */
            struct Network {
                std::string_view
                    testtingNetwork = "network_testtingNetwork";
            };
            /**
             * @struct Error
             * @brief Error message translation keys.
             */
            struct Error {
                std::string_view
                    clickToRetry = "error_clickToRetry",
                    clickToQuit = "error_clickToQuit",
                    jsonParse = "error_jsonParse",
                    tokenJsonParse = "error_tokenJsonParse",
                    apiMetaParse = "error_apiMetaParse",
                    networkConnectionRetryMax = "error_networkConnectionRetryMax",
                    maintenanceInfoReq = "error_maintenanceInfoReq",
                    maintenanceInfoParse = "error_maintenanceInfoParse",
                    downloadPoster = "error_downloadPoster",
                    downloadUpdate = "error_downloadUpdate",
                    installMinecraft = "error_installMinecraft",
                    minecraftVersionEmpty = "error_minecraftVersionEmpty",
                    minecraftVersionParse = "error_minecraftVersionParse",
                    minecraftAuthlibConnection = "error_minecraftAuthlibConnection",
                    minecraftPatchDownload = "error_minecraftPatchDownload",
                    minecraftPatchDownloadHash = "error_minecraftPatchDownloadHash",
                    minecraftGetAuthlibVersion = "error_minecraftGetAuthlibVersion",
                    minecraftAuthlibJsonParse = "error_minecraftAuthlibJsonParse",
                    minecraftAuthlibDownload = "error_minecraftAuthlibDownload",
                    minecraftAuthlibDownloadHash = "error_minecraftAuthlibDownloadHash",
                    minecraftMemoryNotEnough = "error_minecraftMemoryNotEnough";
            };

            std::string_view language = "language";
            General general;
            Title title;
            Loading loading;
            Network network;
            Error error;
        };

        /**
         * @brief Global language key instance.
         */
        constexpr inline LanguageKey lang;

        /**
         * @brief Gets or sets the preferred language.
         * @param lang Optional parameter to set a new preferred language.
         * @return The current preferred language code.
         *
         * @details Defaults to "en" if no language is set.
         */
        inline std::string language(const std::string &lang = "") {
            static std::string preferredLanguage = "en";

            if (!lang.empty())
                preferredLanguage = lang;

            return preferredLanguage;
        }

        /**
         * @brief Gets the path to the language directory.
         * @return The path to the language directory.
         *
         * @details Uses info::workPath() from info.hpp to construct the path.
         */
        inline std::string getLanguageFolder() {
            return info::system::workPath() + "/lang/";
        }

        /**
         * @brief Gets a list of available language files.
         * @param langPath Path to the directory containing language files.
         * @return A vector of language codes.
         *
         * @details Uses exec::matchExtName from exec.hpp to filter JSON files.
         * Uses nlog for logging information about found language files.
         */
        inline std::vector<std::string> getLanguages(const std::string &langPath = getLanguageFolder()) {
            std::vector<std::string> res;
            for (const auto &it : std::filesystem::directory_iterator(langPath)) {
                if (it.is_regular_file() && exec::matchExtName(it.path().string(), "json")) {
                    std::string fileName = it.path().stem().string();
                    nlog::Info(FI, LI, "%s : lang file push : %s", FN, fileName.c_str());
                    res.push_back(fileName);
                }
            }
            return res;
        }

        /**
         * @brief Loads translation data from a language file.
         * @param lang Language code to load.
         * @param langPath Path to the directory containing language files.
         * @return JSON object containing the translations.
         *
         * @details Caches the loaded language file to avoid repeated disk access.
         * Falls back to an empty JSON object if the file cannot be loaded.
         * Uses nlog for logging the loading process.
         */
        inline nlohmann::json loadTranslations(const std::string &lang = language(), const std::string &langPath = getLanguageFolder()) {
            // cached lang
            static std::string cachedLang;
            static std::string cachedLangPath;
            static nlohmann::json cachedJson;

            if (lang != cachedLang || langPath != cachedLangPath) {
                std::string fileName = langPath + lang + ".json";
                std::ifstream i;
                if (std::filesystem::exists(fileName) && [&i, &fileName] {i.open(fileName); return i.is_open(); }()) {
                    auto j = nlohmann::json::parse(i, nullptr, false);
                    nlog::Info(FI, LI, "%s : lang : %s , is open : %s , json is discarded : %s ", FN, lang.c_str(), exec::boolTo<const char *>(i.is_open()), exec::boolTo<const char *>(j.is_discarded()));
                    cachedJson = j;
                } else {
                    cachedJson = nlohmann::json::object();
                }
                cachedLang = lang;
                cachedLangPath = langPath;
            }
            return cachedJson;
        }

        /**
         * @brief Gets a translated string for a specific key.
         * @param key The translation key to look up.
         * @param langFile The JSON object containing translations.
         * @return The translated string, or a fallback message if not found.
         *
         * @details Falls back to English if the key is not found in the specified language.
         * Uses nlog for warning about missing translations.
         */
        inline std::string translations(const std::string &key, const nlohmann::json &langFile = loadTranslations()) {
            auto check = [&key](const nlohmann::json &obj) -> std::string {
                if (obj.empty() || obj.is_discarded() || !obj.contains(key)) {
                    return "translation not found";
                }
                return obj.value(key, "translation not found");
            };

            auto res = check(langFile);

            if (res == "translation not found") {
                nlog::Warn(FI, LI, "%s : Faild to load key : %s for : %s , try to load default file", FN, key.c_str(), langFile.value("language", "Empty lang").c_str());
                return check(loadTranslations("en"));
            }

            return res;
        }

    } // namespace lang

} // namespace neko::info