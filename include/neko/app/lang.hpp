#pragma once

#include <neko/schema/types.hpp>

#include <neko/log/nlog.hpp>
#include <neko/function/utilities.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

/**
 * @namespace neko::lang
 * @brief Language and localization.
 */
namespace neko::lang {

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
                res.push_back(fileName);
                log::info("lang file push : " + fileName);
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
                log::error({},"Language file : {} , does not exist or cannot be opened !", fileName);
                cachedJson = nlohmann::json::object();
                return cachedJson;
            }

            try {
                cachedJson = nlohmann::json::parse(i);
            } catch (const nlohmann::json::parse_error &e) {
                log::error({},"Failed to parse language : {} , file : {}", e.what(), fileName);
                cachedJson = nlohmann::json::object();
                return cachedJson;
            }
            log::info({},"lang : {} , json is discarded : {}", lang, util::logic::boolTo(cachedJson.is_discarded()));
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
    inline std::string tr(const std::string &key, const std::string &fallback = "Translation not found", const nlohmann::json &langFile = loadTranslations()) {
        auto check = [&key,fallback](const nlohmann::json &obj) -> std::string {
            if (obj.empty() || obj.is_discarded() || !obj.contains(key)) {
                return fallback;
            }
            return obj.value(key, fallback);
        };

        auto res = check(langFile);

        if (res == fallback) {
            log::warn({}, "Failed to load key : {} for : {} , try to load default file", key, langFile.value("language", "Empty lang"));
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

    auto withReplaced = [](const std::string &input, std::map<std::string, std::string> replacements) -> std::string {
        return withPlaceholdersReplaced(input, replacements);
    };

    namespace keys {

        /**
         * @namespace neko::lang::keys::action
         * @brief Action-related text
         */
        namespace action {
            constexpr neko::strview
                object = "Action",
                networkRequest = "networkRequest",
                uploadFile = "uploadFile",
                downloadFile = "downloadFile",
                readFile = "readFile",
                writeFile = "writeFile",
                removeFile = "removeFile",
                createFile = "createFile",
                retryMaxReached = "retryMaxReached",
                doingAction = "doingAction",
                parseJson = "parseJson"
                ;
        }

        /**
         * @namespace neko::lang::keys::object
         * @brief Object-related text
         */
        namespace object {
            constexpr neko::strview
                object = "Object",
                maintenance = "maintenance",
                update = "update";
        }

        /**
         * @namespace neko::lang::keys::button
         * @brief Button-related text
         */
        namespace button {
            constexpr neko::strview
                object = "Button",
                ok = "ok",
                open = "open",
                retry = "retry",
                cancel = "cancel",
                close = "close",
                quit = "quit";

        }

        namespace maintenance {
            constexpr neko::strview
                object = "Maintenance",
                message = "message";

        }

        /**
         * @namespace neko::lang::keys::general
         * @brief General purpose text
         */
        namespace general {
            constexpr neko::strview
                object = "General";

        }

        namespace minecraft {
            constexpr neko::strview
                object = "Minecraft";
        }

        namespace error {
            constexpr neko::strview
                error = "Error",
                unknownError = "unknownError",
                networkError = "networkError",
                parseError = "parseError",
                timeout = "timeout",
                notFound = "notFound";

        }
        constexpr neko::strview language = "language";

    } // namespace keys

} // namespace neko::lang