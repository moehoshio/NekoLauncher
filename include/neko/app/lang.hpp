#pragma once

#include <neko/schema/types.hpp>

#include <neko/function/utilities.hpp>
#include <neko/log/nlog.hpp>
#include <neko/system/platform.hpp>

#include "neko/app/nekoLc.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

/**
 * @namespace neko::lang
 * @brief Language and localization.
 */
namespace neko::lang {

    /**
     * @brief Sets or gets the preferred language (file name without extension).
     */
    inline std::string language(const std::string &langCode = "") {
        static std::shared_mutex languageMutex;
        static std::string preferredLanguage = "en";

        if (!langCode.empty()) {
            std::unique_lock lock(languageMutex);
            preferredLanguage = langCode;
        }

        std::shared_lock lock(languageMutex);
        return preferredLanguage;
    }

    /**
     * @brief Gets the path to the language directory.
     * @return The path to the language directory.
     */
    inline std::string getLanguageFolder() {
        return neko::system::workPath() + std::string("/") + lc::LanguageFolderName.data();
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
    inline nlohmann::json loadTranslations(const std::string &langCode = language(), const std::string &langFolder = getLanguageFolder()) {
        // cached lang
        static std::shared_mutex languageMutex;
        static std::string cachedLangCode;
        static std::string cachedLangFolder;
        static nlohmann::json cachedJson;

        {
            std::shared_lock lock(languageMutex);
            if (langCode == cachedLangCode && langFolder == cachedLangFolder) {
                return cachedJson;
            }
        }

        std::unique_lock uniqueLock(languageMutex);
        std::string filePath = langFolder + "/" + langCode + ".json";
        std::ifstream i(filePath);
        if (!std::filesystem::exists(filePath) || !i.is_open()) {
            log::error("Language file : {} , does not exist or cannot be opened !", {}, filePath);
            cachedJson = nlohmann::json::object();
            return cachedJson;
        }

        try {
            cachedJson = nlohmann::json::parse(i);
        } catch (const nlohmann::json::parse_error &e) {
            log::error("Failed to parse language : {} , file : {}", {}, e.what(), filePath);
            cachedJson = nlohmann::json::object();
            return cachedJson;
        }
        log::info("Loaded language file : " + filePath);
        cachedLangCode = langCode;
        cachedLangFolder = langFolder;
        return cachedJson;
    }

    /**
     * @brief Retrieves a list of available languages.
     * @param langPath Path to the directory containing language files.
     * @return A vector of pairs, each containing the language code and its display name.
     */
    inline std::vector<std::pair<std::string, std::string>> getLanguages(const std::string &langFolder = getLanguageFolder()) {
        std::vector<std::pair<std::string, std::string>> result;
        for (const auto &it : std::filesystem::directory_iterator(langFolder)) {
            if (it.is_regular_file() && util::string::matchExtensionName(it.path().string(), "json")) {
                std::string langCode = it.path().stem().string();
                auto json = loadTranslations(langCode, langFolder);
                if (json.contains("language")) {
                    result.emplace_back(langCode, json["language"].get<std::string>());
                } else {
                    result.emplace_back(langCode, langCode);
                }
            }
        }
        return result;
    }

    /**
     * @brief Translates a key within a specified category.
     * @param category The category or subject under which to look up the key.
     * @param key The translation key to look up.
     * @param fallback The fallback message if the key is not found.
     * @param langFile The JSON object containing translations.
     * @return The translated string, or a fallback message if not found.
     *
     * @details Falls back to English if the key is not found in the specified language.
     * Uses nlog for warning about missing translations.
     */
    inline std::string tr(
        const std::string &category,
        const std::string &key,
        const std::string &fallback = "Translation not found",
        const nlohmann::json &langFile = loadTranslations()) {

        auto check = [&category, &key, &fallback](const nlohmann::json &obj) -> std::string {
            if (obj.empty() || obj.is_discarded() || !obj.contains(category) || !obj[category].contains(key)) {
                return fallback;
            }
            return obj[category].value(key, fallback);
        };

        return check(langFile);
    }

    /**
     * @brief Replace placeholders in a string with their corresponding values.
     * @param input The input string with placeholders.
     * @param replacements A map of placeholder names to their replacement values.
     * @return The modified string with placeholders replaced.
     */
    inline std::string withPlaceholdersReplaced(
        const std::string &input,
        std::map<std::string, std::string> replacements = {
            {"{key}", "value"}}) {

        std::string output = input;
        for (const auto &[key, value] : replacements) {
            std::string::size_type pos;
            while ((pos = output.find(key)) != std::string::npos) {
                output.replace(pos, key.length(), value);
            }
        }
        return output;
    }

    /**
     * @brief Translates a key within a specified category and replaces placeholders.
     * @param category The category or subject under which to look up the key.
     * @param key The translation key to look up.
     * @param replacements A map of placeholder names to their replacement values.
     * @return The translated string with placeholders replaced.
     */
    inline std::string trWithReplaced(const std::string &category, const std::string &key, std::map<std::string, std::string> replacements) {
        std::string input = tr(category, key);
        return withPlaceholdersReplaced(input, replacements);
    };

    namespace keys {

        constexpr neko::cstr language = "language";

        namespace setting {
            constexpr neko::cstr
                category = "setting",
                tabAccount = "tabAccount",
                tabMain = "tabMain",
                tabAdvanced = "tabAdvanced",
                groupMain = "groupMain",
                groupStyle = "groupStyle",
                groupNetwork = "groupNetwork",
                groupOther = "groupOther",
                groupMinecraft = "groupMinecraft",
                groupAdvanced = "groupAdvanced",
                language = "language",
                backgroundType = "backgroundType",
                background = "background",
                selectBackground = "selectBackground",
                imageFileFilter = "imageFileFilter",
                windowSize = "windowSize",
                launcherMethod = "launcherMethod",
                useSysWindowFrame = "useSysWindowFrame",
                headBarKeepRight = "headBarKeepRight",
                theme = "theme",
                blurEffect = "blurEffect",
                blurRadius = "blurRadius",
                fontSize = "fontSize",
                fontFamilies = "fontFamilies",
                threads = "threads",
                useSystemProxy = "useSystemProxy",
                proxyPlaceholder = "proxyPlaceholder",
                customTempDir = "customTempDir",
                selectTempDir = "selectTempDir",
                javaPath = "javaPath",
                browseJava = "browseJava",
                javaExecutableFilter = "javaExecutableFilter",
                downloadSource = "downloadSource",
                playerName = "playerName",
                customResolution = "customResolution",
                joinServerAddress = "joinServerAddress",
                joinServerPort = "joinServerPort",
                devEnable = "devEnable",
                devDebug = "devDebug",
                devTls = "devTls",
                devServer = "devServer",
                useDefaultServer = "useDefaultServer",
                devServerPlaceholder = "devServerPlaceholder",
                notLoggedIn = "notLoggedIn",
                login = "login",
                logout = "logout",
                close = "close";
        } // namespace setting

        namespace loading {
            constexpr neko::cstr
                category = "loading",
                starting = "starting...",
                preparing = "preparing...",
                downloading = "downloading...",
                extracting = "extracting...",
                finalizing = "finalizing...";
        } // namespace loading


        namespace launcher {
            constexpr neko::cstr
                category = "launcher",
                launchFailedTitle = "launchFailedTitle",
                launchFailedMessage = "launchFailedMessage";
        } // namespace launcher


        namespace input {
            constexpr neko::cstr
                category = "input",
                title = "title",
                message = "message",
                placeholder = "placeholder",
                password = "password";
        } // namespace input

        namespace maintenance {
            constexpr neko::cstr
                category = "maintenance",
                title = "title",
                message = "message",
                checkingStatus = "checkingStatus",
                parseIng = "parseIng",
                downloadPoster = "downloadPoster";
        } // namespace maintenance

        namespace update {
            constexpr neko::cstr
                category = "update",
                title = "title",
                startingUpdate = "startingUpdate",
                checkingForUpdates = "checkingForUpdates",
                parsingUpdateData = "parsingUpdateData",
                updateAvailable = "updateAvailable",
                noUpdateAvailable = "noUpdateAvailable",
                downloadingUpdate = "downloadingUpdate",
                applyingUpdate = "applyingUpdate";
            
        } // namespace update

        namespace button {
            constexpr neko::cstr
                category = "button",
                open = "open",
                close = "close",
                ok = "ok",
                cancel = "cancel",
                yes = "yes",
                no = "no",
                start = "start",
                menu = "menu",
                maximize = "maximize",
                minimize = "minimize",
                restore = "restore",
                apply = "apply",
                quit = "quit",
                retry = "retry",
                input = "input";
        } // namespace button

        namespace minecraft {
            constexpr neko::cstr
                category = "minecraft",
                missingAccessToken = "missingAccessToken",
                installStart = "installStart",
                fetchVersionList = "fetchVersionList",
                fetchVersionInfo = "fetchVersionInfo",
                downloadingAssetIndex = "downloadingAssetIndex",
                downloadingLibrary = "downloadingLibrary",
                downloadingClient = "downloadingClient",
                downloadingAssets = "downloadingAssets",
                savingVersion = "savingVersion",
                installing = "installing",
                completed = "completed";
        } // namespace minecraft

        namespace error {
            constexpr neko::cstr
                category = "error",
                invalidInput = "invalidInput",
                networkError = "networkError",
                parseError = "parseError",
                updateFailed = "updateFailed",
                launchFailed = "launchFailed",
                seeLog = "seeLog";
        } // namespace error

        

    } // namespace keys

} // namespace neko::lang