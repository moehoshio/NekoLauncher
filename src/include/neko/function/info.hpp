/**
 * @file info.hpp
 * @brief Provides application and language information utilities.
 * @author moehoshio
 * @date 2025-06-05
 *
 * This header contains utilities for:
 * - Application configuration and version information
 * - Language/localization support
 *
 * @dependencies
 * - utilities.hpp: Used for various utility functions
 * - nlog.hpp: Used for logging
 * - clientconfig.hpp: Used for loading configuration
 * - nekodefine.hpp: Contains version definitions
 * - nlohmann/json.hpp: Used for JSON parsing in language support
 */
#pragma once

// Neko Modules
#include "neko/log/nlog.hpp"

#include "neko/core/resources.hpp"

#include "neko/function/utilities.hpp"

#include "neko/schema/clientconfig.hpp"
#include "neko/schema/nekodefine.hpp"
#include "neko/schema/types.hpp"

#include "library/nlohmann/json.hpp"

// C++ Standard Library
#include <string>
#include <string_view>

/**
 * @namespace neko::info
 * @brief Contains utilities for application and language information.
 */
namespace neko::info {

    /**
     * @namespace neko::info::app
     * @brief Application-specific information and utilities.
     */
    namespace app {

        /**
         * @brief Gets the application version.
         * @return The application version string.
         */
        constexpr inline neko::cstr getVersion() {
            return schema::definitions::NekoLcCoreVersion;
        };

        /**
         * @brief Gets the resource version from configuration.
         * @return The resource version string.
         *
         * @details Uses core::getConfigObj from resources.hpp to access configuration.
         */
        inline std::string getResVersion() {
            ClientConfig cfg(core::getConfigObj());
            return cfg.other.resourceVersion;
        }

        /**
         * @brief Gets the configuration file name.
         * @return The configuration file name.
         */
        constexpr inline neko::cstr getConfigFileName() {
            return schema::definitions::clientConfigFileName;
        }

    } // namespace app

    /**
     * @namespace neko::info::lang
     * @brief Language and localization utilities.
     */
    namespace lang {

        /**
         * @struct Keys
         * @brief Contains all translation keys organized by category.
         */
        struct Keys {

            struct Button {
                std::string_view
                    ok = "button_ok",
                    cancel = "button_cancel",
                    retry = "button_retry",
                    close = "button_close",
                    start = "button_start",
                    menu = "button_menu",
                    maximize = "button_maximize",
                    minimize = "button_minimize",
                    login = "button_login",
                    logout = "button_logout",
                    register_ = "button_register",
                    account = "button_account",
                    update = "button_update",
                    download = "button_download",
                    install = "button_install",
                    uninstall = "button_uninstall",
                    patch = "button_patch",
                    setting = "button_setting",
                    other = "button_other",;
            } button;

            /**
             * @struct General
             * @brief General-purpose UI translation keys.
             */
            struct General {
                std::string_view
                    general = "general_general",
                    input = "general_input",
                    setting = "general_setting",
                    language = "general_lang",
                    close = "general_close",
                    accessToken = "general_accessToken",
                    username = "general_username",
                    password = "general_password",
                    background = "general_background",
                    style = "general_style",
                    window = "general_window",
                    launcher = "general_launcher",
                    network = "general_network",
                    proxy = "general_proxy",
                    other = "general_other",
                    none = "general_none",
                    image = "general_image",
                    type = "general_type",
                    font = "general_font",
                    width = "general_width",
                    height = "general_height",
                    size = "general_windowSize",
                    blurEffect = "general_blurEffect",
                    animation = "general_animation",
                    performance = "general_performance",
                    quality = "general_quality",
                    blurRadius = "general_blurRadius",
                    launcherMethod = "general_launcherMethod",
                    keepWindow = "general_keepWindow",
                    endProcess = "general_endProcess",
                    headBarKeepRight = "general_headBarKeepRight",
                    customTempDir = "general_customTempDir",
                    hideAndOverReShow = "general_hideAndOverReShow",
                    useSystemWindowFrame = "general_useSystemWindowFrame",
                    notAutoSetThreadNums = "general_notAutoSetThreadNums",
                    pointSize = "general_pointSize",
                    tempFolder = "general_tempFolder",
                    proxyPlaceholder = "general_proxyPlaceholder",
                    notEnoughParameters = "general_notEnoughParameters",
                    incompleteApplied = "general_incompleteApplied";
            } general;
            /**
             * @struct Title
             * @brief Window and dialog title translation keys.
             */
            struct Title {
                std::string_view
                    error = "title_error",
                    networkError = "title_networkError",
                    parseError = "title_parseError",
                    warning = "title_warning",
                    maintenance = "title_maintenance",
                    reStart = "title_reStart",
                    incomplete = "title_incomplete",
                    needLogin = "title_needLogin",
                    inputLogin = "title_inputLogin",
                    inputNotEnoughParameters = "title_inputNotEnoughParameters",
                    loginOrRegister = "title_loginOrRegister",
                    logoutConfirm = "title_logoutConfirm";
            } title;

            /**
             * @struct Auth
             * @brief Authentication-related message translation keys.
             */
            struct Auth {
                std::string_view
                    notLogin = "auth_notLogin",
                    noRegisterLink = "auth_noRegisterLink",
                    needLogin = "auth_needLogin",
                    loginOrRegister = "auth_loginOrRegister",
                    logoutConfirm = "auth_logoutConfirm";
            } auth;

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
            } loading;
            /**
             * @struct Network
             * @brief Network-related message translation keys.
             */
            struct Network {
                std::string_view
                    testtingNetwork = "network_testtingNetwork",
                    networkError = "network_networkError",
                    networkConnectionRetryMax = "network_networkConnectionRetryMax",
                    networkConnectionRetry = "network_networkConnectionRetry",
                    downloadFileError = "network_downloadFileError",
                    downloadPosterError = "network_downloadPosterError",
                    downloadUpdateError = "network_downloadUpdateError",
                    updateOverReStart = "network_updateOverReStart";
            } network;

            /**
             * @struct Minecraft
             * @brief Minecraft-related message translation keys.
             */
            struct Minecraft {
                std::string_view
                    installMinecraft = "minecraft_installMinecraft",
                    installMinecraftError = "minecraft_installMinecraftError",
                    installMinecraftSuccess = "minecraft_installMinecraftSuccess",

                    minecraftVersionEmpty = "minecraft_versionEmpty",

                    minecraftAuthlibConnection = "minecraft_authlibConnection",
                    minecraftPatchDownloadError = "minecraft_patchDownloadError",
                    minecraftPatchDownloadHash = "minecraft_patchDownloadHash",

                    minecraftAuthlibHash = "minecraft_authlibHash",

                    minecraftNeedMemory = "minecraft_needMemory",
                    minecraftMemoryNotEnough = "minecraft_memoryNotEnough",
                    minecraftVersionKeyOutOfRange = "minecraft_versionKeyOutOfRange",

                    minecraftFileError = "minecraft_fileError",
                    minecraftNetworkError = "minecraft_networkError",
                    minecraftException = "minecraft_exception",
                    minecraftUnexpected = "minecraft_unexpected",
                    minecraftUnknownError = "minecraft_unknownError";
            } minecraft;

            /**
             * @struct Error
             * @brief Error message translation keys.
             */
            struct Error {
                std::string_view
                    unexpected = "error_unexpected",
                    unknownError = "error_unknownError",
                    missingError = "error_missingError",
                    invalidInput = "error_invalidInput",
                    clickToRetry = "error_clickToRetry",
                    clickToQuit = "error_clickToQuit",
                    parse = "error_parse";
            } error;

            std::string_view language = "language";
        } constexpr inline keys;

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
         */
        inline std::string getLanguageFolder() {
            using namespace neko::ops::pipe;
            return (std::filesystem::current_path() / "lang").string() | util::unifiedPath;
        }

        /**
         * @brief Gets a list of available language files.
         * @param langPath Path to the directory containing language files.
         * @return A vector of language codes.
         */
        inline std::vector<std::string> getLanguages(const std::string &langPath = getLanguageFolder()) {
            std::vector<std::string> res;
            for (const auto &it : std::filesystem::directory_iterator(langPath)) {
                if (it.is_regular_file() && util::string::matchExtensionName(it.path().string(), "json")) {
                    std::string fileName = it.path().stem().string();
                    log::Info(log::SrcLoc::current(), "lang file push : %s", fileName.c_str());
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
         * @details Falls back to an empty JSON object if the file cannot be loaded.
         * Caches the loaded language file
         * Uses nlog for logging the loading process.
         */
        inline nlohmann::json loadTranslations(const std::string &lang = language(), const std::string &langFolder = getLanguageFolder()) {
            // cached lang
            static std::string cachedLang;
            static std::string cachedLangFolder;
            static nlohmann::json cachedJson;

            if (lang != cachedLang || langFolder != cachedLangFolder) {
                std::string fileName = langFolder + "/" + lang + ".json";
                std::ifstream i(fileName);
                if (!std::filesystem::exists(fileName) || !i.is_open()) {
                    log::Err(log::SrcLoc::current(), "Language file : '%s' , does not exist or cannot be opened !", fileName.c_str());
                    cachedJson = nlohmann::json::object();
                    return cachedJson;
                }

                try {
                    cachedJson = nlohmann::json::parse(i);
                } catch (const nlohmann::json::parse_error &e) {
                    log::Err(log::SrcLoc::current(), "Failed to parse language : %s , file : %s", e.what(), fileName.c_str());
                    cachedJson = nlohmann::json::object();
                    return cachedJson;
                }
                log::Info(log::SrcLoc::current(), "lang : %s , json is discarded : %s ", lang.c_str(), util::logic::boolTo<neko::cstr>(cachedJson.is_discarded()));
                cachedLang = lang;
                cachedLangFolder = langFolder;
            }
            return cachedJson;
        }

        /**
         * @brief Gets a translated string for a specific key.
         * @param key The translation key to look up.
         * @param fallback The fallback message if the key is not found.
         * @param langFile The JSON object containing translations.
         * @return The translated string, or a fallback message if not found.
         *
         * @details Falls back to English if the key is not found in the specified language.
         * Uses nlog for warning about missing translations.
         */
        inline std::string tr(const std::string &key,const std::string& fallback = "Translation not found", const nlohmann::json &langFile = loadTranslations()) {
            auto check = [&key](const nlohmann::json &obj) -> std::string {
                if (obj.empty() || obj.is_discarded() || !obj.contains(key)) {
                    return fallback;
                }
                return obj.value(key, fallback);
            };

            auto res = check(langFile);

            if (res == fallback) {
                log::Warn(log::SrcLoc::current(), "Failed to load key : %s for : %s , try to load default file", key.c_str(), langFile.value("language", "Empty lang").c_str());
                return check(loadTranslations("en"));
            }

            return res;
        }

        /**
         * @brief Replace placeholders in a string with their corresponding values.
         * @param input The input string with placeholders.
         * @param replacements A map of placeholder names to their replacement values.
         * @return The modified string with placeholders replaced.
         */
        std::string withPlaceholdersReplaced(
            const std::string &input,
            std::map<std::string, std::string> replacements = {
                {"{version}", info::app::getVersion()},
                {"{resVersion}", info::app::getResVersion()},
                {"{lang}", language()}}) {
            std::string output = input;
            for (const auto &[key, value] : replacements) {
                std::string::size_type pos;
                while ((pos = output.find(key)) != std::string::npos) {
                    output.replace(pos, key.length(), value);
                }
            }
            return output;
        }

        auto withReplaced = [](const std::string &input, std::map<std::string, std::string> replacements) -> std::string {
            return withPlaceholdersReplaced(input, replacements);
        };

    } // namespace lang

} // namespace neko::info