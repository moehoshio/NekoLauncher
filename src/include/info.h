#pragma once
#include "exec.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <string>
namespace neko {

    class info {
    private:
        struct Data {

            constexpr static const char *version =
#include "../data/version"
                ;
            //             constexpr static const char *website =
            // #include "../data/website"
            //                 ;
            constexpr static const char *osName =
#if _WIN32
                "windows";
#elif __APPLE__
                "osx";
#elif __linux__
                "linux";
#else
                "unknown";
#endif
        };

    public:
        inline static std::string temp(const std::string &setTempDir = "") {
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);
            auto init = [] {
                if (std::string temp = exec::getConfigObj().GetValue("more", "temp", "");
                    std::filesystem::is_directory(temp))
                    return temp | exec::unifiedPaths;
                else
                    return (std::filesystem::temp_directory_path().string() + "Nekolc/") | exec::unifiedPaths;
            };
            static std::string tempDir = init();

            if (!setTempDir.empty() && std::filesystem::is_directory(setTempDir)) {
                tempDir = setTempDir | exec::unifiedPaths;
            }

            if (!std::filesystem::exists(tempDir))
                std::filesystem::create_directory(tempDir);

            return tempDir;
        }

        inline static std::string workPath(const std::string &setPath = "") {
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);
            if (!setPath.empty() && std::filesystem::is_directory(setPath))
                std::filesystem::current_path(setPath);

            return std::filesystem::current_path().string() | exec::unifiedPaths;
        }

        inline static std::string getHome() {
            const char *path = std::getenv(
#ifdef _WIN32
                "USERPROFILE"
#else
                "HOME"
#endif
            );
            if (path)
                return exec::unifiedThePaths<std::string>(path);
            return std::string();
        }

        inline static std::string getVersion() {
            return Data::version;
        };

        inline static std::string getResVersion() {
            return exec::getConfigObj().GetValue("more", "resVersion", "");
        }

        constexpr inline static auto getOsName() {
            return Data::osName;
        }

        inline static std::string getOsNameS() {
            return Data::osName;
        }

        struct LanguageKey {
            struct General {
                std::string
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
                    incompleteApplied = "general_incompleteApplied";
            };
            struct Title {
                std::string
                    error = "title_error",
                    warning = "title_warning",
                    maintenance = "title_maintenance",
                    reStart = "title_reStart",
                    incomplete = "title_incomplete",
                    notLogin = "title_notLogin",
                    inputLogin = "title_inputLogin",
                    inputNotEnoughParameters = "title_inputNotEnoughParameters";
            };

            struct Loading {
                std::string
                    maintenanceInfoReq = "loading_maintenanceInfoReq",
                    maintenanceInfoParse = "loading_maintenanceInfoParse",
                    downloadMaintenancePoster = "loading_downloadMaintenancePoster",
                    checkUpdate = "loading_checkUpdate",
                    updateInfoParse = "loading_updateInfoParse",
                    downloadUpdatePoster = "loading_downloadUpdatePoster",
                    settingDownload = "loading_settingDownload",
                    downloadUpdate = "loading_downloadUpdate";
            };
            struct Network {
                std::string
                    testtingNetwork = "network_testtingNetwork";
            };
            struct Error {
                std::string
                    jsonParse = "error_jsonParse",
                    tokenJsonParse = "error_tokenJsonParse",
                    apiMetaParse = "error_apiMetaParse",
                    networkConnectionRetryMax = "error_networkConnectionRetryMax",
                    maintenanceInfoReq = "error_maintenanceInfoReq",
                    maintenanceInfoParse = "error_maintenanceInfoParse",
                    downloadPoster = "error_downloadPoster",
                    downloadUpdate = "error_downloadUpdate",
                    minecraftVersionEmpty = "error_minecraftVersionEmpty",
                    minecraftVersionParse = "error_minecraftVersionParse",
                    minecraftAuthlibConnection = "error_minecraftAuthlibConnection",
                    minecraftPatchDownload = "error_minecraftPatchDownload",
                    minecraftPatchDownloadHash = "error_minecraftPatchDownloadHash",
                    minecraftGetAuthlibVersion = "error_minecraftGetAuthlibVersion",
                    minecraftAuthlibJsonParse = "error_minecraftAuthlibJsonParse",
                    minecraftAuthlibDownload = "error_minecraftAuthlibDownload",
                    minecraftAuthlibDownloadHash = "error_minecraftAuthlibDownloadHash";
            };

            std::string language = "language";
            General general;
            Title title;
            Loading loading;
            Network network;
            Error error;
        };
        static LanguageKey lang;

        inline static std::string language(const std::string &lang = "") {
            static std::string preferredLanguage = "en";

            if (!lang.empty())
                preferredLanguage = lang;

            return preferredLanguage;
        }

        inline static std::vector<std::string> getLanguages() {
            std::vector<std::string> res;
            for (const auto &it : std::filesystem::directory_iterator(info::workPath() + "/lang/")) {
                if (it.is_regular_file() && exec::matchExtName(it.path().string(), "json")) {
                    std::string fileName = it.path().stem().string();
                    nlog::Info(FI, LI, "%s : lang file push : %s", FN, fileName.c_str());
                    res.push_back(fileName | exec::move);
                }
            }
            return res;
        }

        inline static nlohmann::json loadTranslations(const std::string &lang = language()) {
            std::string fileName = "lang/" + lang + ".json";
            std::ifstream i;
            if (std::filesystem::exists(fileName) && [&i,&fileName]{i.open(fileName); return i.is_open();}() )
            {
                auto j = nlohmann::json::parse(i, nullptr, false);
                nlog::Info(FI, LI, "%s : lang : %s , is open : %s , json is discarded : %s ", FN, lang.c_str(), exec::boolTo<const char *>(i.is_open()), exec::boolTo<const char *>(j.is_discarded()));
                return j;
            }
            return nlohmann::json::object();
        }

        inline static std::string translations(const std::string &key, const nlohmann::json &langFile = loadTranslations()) {
            nlog::autoLog log{FI, LI, FN, "Enter , key : " + key};
            auto check = [&key](const nlohmann::json &obj) -> std::string {
                if (obj.is_discarded() || !obj.contains(key)) {
                    nlog::Warn(FI, LI, "%s : faild to load translations(include faild to json parse ) or not contains key : %s , try to load defauld file", FN, key.c_str());
                    return "Null";
                }
                auto res = obj.value(key,"Null");
                nlog::Info(FI, LI, "%s : key : %s , res : %s", FN, key.c_str(), res.c_str());
                return res;
            };

            if (auto res = check(langFile); res != "Null") {
                return res;
            } else {
                nlog::Warn(FI, LI, "%s : try to load default file", FN);
                return check(loadTranslations("en"));
            }

            return "null";
        }

    }; // class info

} // namespace neko