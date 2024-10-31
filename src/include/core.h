#pragma once
#include "cconfig.h"
#include "exec.h"
#include "info.h"
#include "msgtypes.h"
#include "network.h"
#include "nlohmann/json.hpp"

#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
// openurl
#include <QtGui/QDesktopServices>

#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>

#include <iostream>

constexpr const char *launcherMode = "minecraft"; // lua or minecraft

namespace neko {

    enum class launcherOpt {
        keep,
        endProcess,
        hideProcessAndOverReShow
    };

    void launchNewProcess(const std::string &command);

    void launcherProcess(const std::string &command, launcherOpt opt, std::function<void(bool)> winFunc = nullptr);

    // example lacunher lua
    inline bool launcherLuaPreCheck() {
        const char *path = std::getenv("LUA_PATH");
        if (path == nullptr) {
            nlog::Err(FI, LI, "%s : lua path is null!", FN);
            return false;
        }
        std::string scriptPath = "helloLua/helloLua.luac";
        if (!std::filesystem::exists(scriptPath)) {
            nlog::Err(FI, LI, "%s : script is not exists!", FN);
            return false;
        }
        return true;
    }

    inline bool launcherJavaPreCheck(const std::string &str) {
        const char *path = std::getenv("JAVA");
        if (path == nullptr) {
            nlog::Err(FI, LI, "%s : java env is null!", FN);
            return false;
        }
        return true;
    }

    inline bool launcherMinecraftTokenValidate(std::function<void(const ui::hintMsg &)> hintFunc = nullptr) {
        nlog::autoLog log{FI, LI, FN};
        network net;
        auto url = networkBase::buildUrl(neko::networkBase::Api::Authlib::validate, neko::networkBase::Api::Authlib::host);

        nlohmann::json json = {{"accessToken", exec::getConfigObj().GetValue("manage", "accessToken", "")}};
        auto data = json.dump();
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.data = data.c_str();
        args.header = "Content-Type: application/json";
        net.Do(networkBase::Opt::postText, args);
        if (code != 204) {
            nlog::Info(FI, LI, "%s : token is not validate", FN);
            auto refUrl = networkBase::buildUrl(neko::networkBase::Api::Authlib::refresh, neko::networkBase::Api::Authlib::host);
            int refCode = 0;
            nlohmann::json refJson = {
                {"accessToken", exec::getConfigObj().GetValue("manage", "accessToken", "")}, {"requestUser", false}};
            auto refData = refJson.dump();
            decltype(net)::Args refArgs{refUrl.c_str(), nullptr, &refCode};
            refArgs.data = refData.c_str();
            refArgs.header = "Content-Type: application/json";
            auto res = net.get(networkBase::Opt::postText, refArgs);
            auto jsonData = nlohmann::json::parse(res, nullptr, false);
            if (jsonData.is_discarded()) {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.tokenJsonParse), "", 1});
                nlog::Err(FI, LI, "%s : faild to token json parse", FN);
                return false;
            }
            auto error = jsonData.value("error", ""),
                 errorMsg = jsonData.value("errorMessage", "");
            if (!error.empty() || !errorMsg.empty()) {
                hintFunc({error, errorMsg, "", 1});
                return false;
            }

            auto accessToken = jsonData["accessToken"].get<std::string>();
            std::string uuid;
            std::string name;
            if (!jsonData["selectedProfile"].empty()) {
                uuid = jsonData["selectedProfile"].value("id", "");
                name = jsonData["selectedProfile"].value("name", "");

                exec::getConfigObj().SetValue("manage", "uuid", uuid.c_str());
                exec::getConfigObj().SetValue("manage", "displayName", name.c_str());
            }
            exec::getConfigObj().SetValue("manage", "accessToken", accessToken.c_str());
        }
        return true;
    }

    inline void launcherMinecraftAuthlibAndPrefetchedCheck(std::function<void(const ui::hintMsg &)> hintFunc = nullptr) {
        nlog::autoLog log{FI, LI, FN};
        std::string authlibPrefetched = exec::getConfigObj().GetValue("manage", "authlibPrefetched", "");
        if (!authlibPrefetched.empty())
            return;

        auto url = networkBase::buildUrl(neko::networkBase::Api::Authlib::root, neko::networkBase::Api::Authlib::host);
        network net;
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        auto res = net.autoRetryGet(networkBase::Opt::getContent, {args});

        if (res.empty()) {
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftAuthlibConnection), "", 1});
            nlog::Err(FI, LI, "%s : faild to connection authlib server!", FN);
            return;
        }

        auto resJson = nlohmann::json::parse(res, nullptr, false);
        if (resJson.is_discarded()) {
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.apiMetaParse), "", 1});
            nlog::Err(FI, LI, "%s : faild to api mete data parse", FN);
            return;
        }
        authlibPrefetched = exec::base64Encode(res);
        exec::getConfigObj().SetValue("manage", "authlibPrefetched", authlibPrefetched.c_str());
    }

    inline void launcherMinecraft(launcherOpt opt, Config cfg, std::function<void(const ui::hintMsg &)> hintFunc = nullptr, std::function<void(bool)> winFunc = nullptr) {
        nlog::autoLog log{FI, LI, FN};
        // /.minecraft
        std::string minecraftDir;

#if _WIN32
        minecraftDir = "/.minecraft";
#elif __APPLE__
        minecraftDir = "/minecraft";
#elif __linux__
        minecraftDir = "/minecraft";
#else
        minecraftDir = "/minecraft";
#endif

        std::string osArch =
#if __x86_64 || _M_X64
            "x64";
#else
            "x86";
#endif

        bool isDemoUser = false;
        bool hasCustomResolution = false;
        // powershell
        auto psPlusArgs = [](const std::vector<std::string> &list) {
            std::string res;
            for (const auto &it : list) {
                res += (" '" + it + "'");
            }
            return res;
        };

        auto plusArgs = [](const std::vector<std::string> &list) {
            std::string res;
            for (const auto &it : list) {
                res += " \"" + it + "\"";
            }
            return res;
        };

        // Assume the Minecraft folder is located under the working directory.

        // ./.minecraft/version
        std::string gameVerDir;
        // ./.minecraft/version/name/name.json
        std::string gameVerFileStr;
        std::fstream gameVerFile;
        for (const auto &it : std::filesystem::directory_iterator(info::workPath() + minecraftDir + "/versions")) {
            if (it.is_directory()) {
                gameVerDir = std::filesystem::absolute(it).string() | exec::unifiedPaths;
                gameVerFileStr = gameVerDir + "/" + it.path().filename().string() + ".json";
                gameVerFile.open(gameVerFileStr);
                break;
            }
        }
        // transition to verJsonData
        std::string gameVerStr;
        // file stream transition to gameVerStr
        std::ostringstream gameVerOss;

        gameVerOss << gameVerFile.rdbuf();
        gameVerStr = gameVerOss.str();
        nlog::Info(FI, LI, "%s : version file : %s , is open : %s ,gameVerStr len : %zu", FN, gameVerFileStr.c_str(), exec::boolTo<const char *>(gameVerFile.is_open()), gameVerStr.length());

        if (gameVerStr.empty()) {
            nlog::Err(FI, LI, "%s : game version string is empty!", FN);
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftVersionEmpty)});
            return;
        }

        auto verJsonData = nlohmann::json::parse(gameVerStr);
        if (verJsonData.is_discarded()) {
            nlog::Err(FI, LI, "%s : faild to json parse! file : %s ", FN, gameVerFileStr.c_str());
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftVersionParse)});
            return;
        }

        auto baseArgs = verJsonData["arguments"];
        auto jvmArgs = baseArgs["jvm"];
        auto gameArgs = baseArgs["game"];
        auto libraries = verJsonData["libraries"];

        // jvm
        std::string
            javaPath = (info::workPath() + "/java/bin/java"), // Assume built-in Java is distributed with the executable.
            gameDir = info::workPath() + minecraftDir,        // work dir + ./minecraft
            mainClass = verJsonData.value("mainClass", "net.minecraft.client.main.Main"),
            clientJarPath = gameVerDir + "/" + verJsonData.value("jar", "") + ".jar", // ./ game ver dir + name.jar
            nativesPath = gameVerDir + "/natives",                                    // game ver dir + /natives
            librariesPath = gameDir + "/libraries",                                   // game dir + /libraries
            classPath;

        // game
        std::string
            gameArgsName = cfg.manage.displayName, // player name
            gameArgsVerName = "Neko Launcher",
            gameArgsAssetsDir = gameDir + "/assets",            // game dir + /assets
            gameArgsAssetsId = verJsonData.value("assets", ""), // assets id , e.g 1.16
            gameArgsUuid = cfg.manage.uuid,
            gameArgsToken = cfg.manage.accessToken,
            gameArgsUserType = "mojang",
            gameArgsVerType = gameArgsVerName; // Nekolc

        std::vector<std::string> jvmArgsVec;
        std::vector<std::string> gameArgsVec;

        struct RulesMap {
            std::string
                action,
                osName,
                osVersion,
                osArch;
        };

        struct ArtifactMap {
            std::string
                path,
                url,
                sha1,
                natives;
            size_t size;
            struct Classifiers {
                std::string
                    path,
                    url,
                    sha1;
                size_t size;
                bool empty() {
                    for (auto it : std::vector<bool>{path.empty(), url.empty(), sha1.empty()}) {
                        if (!it) {
                            return false;
                        }
                    }
                    return true;
                }
            };
            Classifiers classifiers;
            bool empty() {
                for (auto it : std::vector<bool>{path.empty(), url.empty(), sha1.empty(), natives.empty(), classifiers.empty()}) {
                    if (!it) {
                        return false;
                    }
                }
                return true;
            };
        };

        auto checkCondition = [=](const RulesMap &rules, const nlohmann::json &features) -> bool {
            if (!features.empty()) {
                if (features.contains("is_demo_user") && features["is_demo_user"].get<bool>() == isDemoUser)
                    return true;
                if (features.contains("has_custom_resolution") && features["has_custom_resolution"].get<bool>() == hasCustomResolution)
                    return true;
            }

            if (!rules.osName.empty()) {
                bool allow = (rules.osName == info::getOsNameS() && rules.action == "allow") || (rules.osName != info::getOsNameS() && rules.action == "disallow");
                if (allow)
                    return true;
            }

            if (!rules.osArch.empty()) {
                bool allow = (rules.osArch == osArch && rules.action == "allow") || (rules.osArch != osArch && rules.action == "disallow");
                if (allow)
                    return true;
            }

            return false;
        };

        auto processArgs = [=](const nlohmann::json &args, std::vector<std::string> &argsVec) {
            for (const auto &it : args) {
                bool allow = false;
                if (it.is_string()) {
                    nlog::Info(FI, LI, "%s : is string : %s", FN, it.get<std::string>().c_str());
                    allow = true;
                } else if (it.is_object()) {

                    for (const auto &ruless : it["rules"]) {

                        auto rules = ruless;
                        RulesMap rulesMap;

                        rulesMap.action = rules.value("action", "");

                        if (rules.contains("os")) {
                            auto os = rules["os"];
                            rulesMap.osName = os.value("name", "");
                            rulesMap.osVersion = os.value("version", "");
                            rulesMap.osArch = os.value("arch", "");
                        }

                        allow = checkCondition(rulesMap, rules["features"]);
                    }

                } else {
                    nlog::Warn(FI, LI, "%s : Unexpected not obj and str , type : %s", FN, it.type_name());
                }

                if (allow) {
                    if (it.is_string()) {
                        argsVec.push_back(it.get<std::string>());
                    } else {
                        for (const auto &pushArg : it["value"]) {
                            if (cfg.dev.enable && cfg.dev.debug)
                                nlog::Info(FI, LI, "%s : push arg : %s", FN, pushArg.get<std::string>().c_str());
                            argsVec.push_back(pushArg.get<std::string>());
                        }
                    }
                }
            }
        };

        processArgs(jvmArgs, jvmArgsVec);
        processArgs(gameArgs, gameArgsVec);

        auto constructPath = [](const std::string &rawName) -> std::string {
            std::smatch match;
            if (std::regex_match(rawName, match, std::regex("([^:]+):([^:]+):([^:]+)"))) {
                std::string package = match[1].str();
                std::string name = match[2].str();
                std::string version = match[3].str();
                std::replace(package.begin(), package.end(), '.', '/');
                return package + "/" + name + "/" + version + "/" + name + "-" + version + ".jar";
            }
            return {};
        };

        auto constructClassPath = [](const std::vector<std::string> &paths, const std::string &osName) -> std::string {
            const std::string separator = (osName == "windows") ? ";" : ":";
            return std::accumulate(std::next(paths.begin()), paths.end(), paths[0],
                                   [&](std::string acc, const std::string &path) {
                                       return acc + separator + path;
                                   });
        };

        auto checkArchives = [=](const ArtifactMap &artifact) {
            struct Single {
                std::string
                    path,
                    url,
                    sha1;
                size_t size;
            };
            std::vector<Single> vec;

            if (!artifact.natives.empty()) {
                vec.push_back({artifact.classifiers.path,
                               artifact.classifiers.url,
                               artifact.classifiers.sha1,
                               artifact.classifiers.size});
            }
            vec.push_back({artifact.path,
                           artifact.url,
                           artifact.sha1,
                           artifact.size});

            auto downloadTask = [=](const Single &single) {
                network net;
                int code = 0;
                decltype(net)::Args args{
                    single.url.c_str(), nullptr, &code};
                args.writeCallback = networkBase::WriteCallbackFile;
                net.Do(networkBase::Opt::downloadFile, args);
                if (code != 200) {
                    hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftPatchDownload), "", 1});
                    nlog::Err(FI, LI, "%s : faild to archives patch download , file : %s , url : %s ", FN, single.path.c_str(), single.url.c_str());
                    return false;
                }
                auto hash = exec::hashFile(single.path, exec::hashs::Algorithm::sha1);
                if (hash != single.sha1) {
                    hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftPatchDownloadHash), "", 1});
                    nlog::Err(FI, LI, "%s : faild to archives patch download , file : %s , ex sha1 : %s , sha1 : %s , size : %zu , url : %s", FN, single.path.c_str(), hash.c_str(), single.sha1.c_str(), single.size, single.url.c_str());
                    return false;
                }
                return true;
            };

            for (const auto &it : vec) {
                if (std::filesystem::exists(it.path)) {
                    auto hash = exec::hashFile(it.path, exec::hashs::Algorithm::sha1);
                    if (hash != it.sha1) {
                        nlog::Info(FI, LI, "%s : archives exists but hash not match , ex sha1 : %s , sha1 : %s ", FN, it.sha1.c_str(), hash.c_str());
                        if (!downloadTask(it))
                            return false;
                    }
                } else {
                    nlog::Info(FI, LI, "%s : archives not exists , path : %s , ready to download", FN, it.path.c_str());
                    if (!downloadTask(it))
                        return false;
                }
            }
            return true;
        };

        std::vector<std::string> libPaths;
        for (const auto &lib : libraries) {

            bool allow = true;
            nlog::Info(FI, LI, "%s : lib type : %s", FN, lib.type_name());
            if (lib.contains("rules") && lib["rules"].is_array()) {
                for (const auto &ruless : lib["rules"]) {

                    auto rules = ruless;
                    RulesMap rulesMap;
                    rulesMap.action = rules.value("action", "");
                    if (rules.contains("os")) {
                        auto os = rules["os"];
                        rulesMap.osName = os.value("name", "");
                        rulesMap.osVersion = os.value("version", "");
                        rulesMap.osArch = os.value("arch", "");
                    }

                    allow = checkCondition(rulesMap, rules["features"]);
                }
            }

            if (allow) {

                // check and patch archives
                if (lib.contains("downloads") && lib["downloads"].contains("artifact")) {
                    ArtifactMap artifact;
                    artifact.path = librariesPath + "/" + lib["downloads"]["artifact"]["path"].get<std::string>();
                    artifact.url = lib["downloads"]["artifact"]["url"].get<std::string>();
                    artifact.sha1 = lib["downloads"]["artifact"]["sha1"].get<std::string>();
                    artifact.size = lib["downloads"]["artifact"]["size"].get<size_t>();

                    if (lib.contains("natives")) {
                        for (auto natives : lib["natives"].items()) {
                            if (natives.key() == info::getOsNameS()) {
                                artifact.natives = natives.value();
                                artifact.classifiers.path = librariesPath + "/" + lib["downloads"]["classifiers"][artifact.natives]["path"].get<std::string>();
                                artifact.classifiers.url = lib["downloads"]["classifiers"][artifact.natives]["url"].get<std::string>();
                                artifact.classifiers.sha1 = lib["downloads"]["classifiers"][artifact.natives]["sha1"].get<std::string>();
                                artifact.classifiers.size = lib["downloads"]["classifiers"][artifact.natives]["size"].get<size_t>();
                            }
                        }
                    }
                    if (!checkArchives(artifact)) // in func already keep a log
                        return;
                }

                std::string path = librariesPath + "/" + constructPath(lib["name"].get<std::string>());
                if (cfg.dev.enable && cfg.dev.debug)
                    nlog::Info(FI, LI, "%s : push path : %s", FN, path.c_str());

                libPaths.push_back(path);
            }
        }

        classPath = constructClassPath(libPaths, info::getOsNameS()) + ((info::getOsNameS() == "windows") ? ";" : ":") + clientJarPath;

        // replace placeholders
        auto replacePlaceholders = [&](std::vector<std::string> &argsVec, const std::map<std::string, std::string> &placeholders) {
            for (auto &arg : argsVec) {
                for (const auto &[key, value] : placeholders) {
                    std::string::size_type pos;
                    while ((pos = arg.find(key)) != std::string::npos) {
                        arg.replace(pos, key.length(), value);
                    }
                }
            }
        };

        // jvm
        replacePlaceholders(jvmArgsVec, {{"${natives_directory}", nativesPath},
                                         {"${library_directory}", librariesPath},
                                         {"${launcher_name}", "Neko Launcher"},
                                         {"${launcher_version}", info::getVersion()},
                                         {"${classpath}", classPath}});

        // game
        replacePlaceholders(gameArgsVec, {{"${auth_player_name}", gameArgsName},
                                          {"${version_name}", gameArgsVerName},
                                          {"${game_directory}", gameDir},
                                          {"${assets_root}", gameArgsAssetsDir},
                                          {"${assets_index_name}", gameArgsAssetsId},
                                          {"${auth_uuid}", gameArgsUuid},
                                          {"${auth_access_token}", gameArgsToken},
                                          {"${user_type}", gameArgsUserType},
                                          {"${version_type}", gameArgsVerType}});
        std::vector<std::string> jvmOptimizeArgs = {
            "-XX:+UnlockExperimentalVMOptions", "-XX:+UseG1GC", "-XX:G1NewSizePercent=20", "-XX:G1ReservePercent=20", "-XX:MaxGCPauseMillis=50", "-XX:G1HeapRegionSize=16m", "-XX:-UseAdaptiveSizePolicy", "-XX:-OmitStackTraceInFastThrow", "-XX:-DontCompileHugeMethods", "-Xmn128m", "-Xmx10240m", "-Dfml.ignoreInvalidMinecraftCertificates=true", "-Dfml.ignorePatchDiscrepancies=true"};

        // authlib Injector
        std::string authlibPrefrtched = std::string(cfg.manage.authlibPrefetched);
        authlibPrefrtched.erase(std::remove(authlibPrefrtched.begin(), authlibPrefrtched.end(), '\\'), authlibPrefrtched.end());
        std::string authlibPath = gameDir + "/authlib-injector.jar";

        if (!std::filesystem::exists(authlibPath)) {
            network net;
            auto url = networkBase::buildUrl(networkBase::Api::Authlib::Injector::latest, networkBase::Api::Authlib::Injector::downloadHost);
            int code = 0;
            decltype(net)::Args args{url.c_str(), nullptr, &code};

            auto authlibVersionInfo = net.get(networkBase::Opt::getContent, args);
            auto authlibVersionData = nlohmann::json::parse(authlibVersionInfo, nullptr, false);
            if (code != 200) {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftGetAuthlibVersion), "", 1});
                nlog::Err(FI, LI, "%s : in download authlib injector ,faild to get authlib Injector version info", FN);
                return;
            }
            if (authlibVersionData.is_discarded()) {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftAuthlibJsonParse), "", 1});
                nlog::Err(FI, LI, "%s : in download authlib injector ,faild to parse authlib Injector version info", FN);
                return;
            }

            auto downloadUrl = authlibVersionData["download_url"].get<std::string>();
            args.url = downloadUrl.c_str();
            args.fileName = authlibPath.c_str();
            args.writeCallback = networkBase::WriteCallbackFile;

            net.Do(networkBase::Opt::downloadFile, args);
            if (code != 200) {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftAuthlibDownload), "", 1});
                nlog::Err(FI, LI, "%s : in download authlib injector ,faild to download authlib Injector archive", FN);
                return;
            }
            auto hash = exec::hashFile(authlibPath);
            auto exHash = authlibVersionData["checksums"].value("sha256", "");
            if (hash != exHash) {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftAuthlibDownloadHash), "", 1});
                nlog::Err(FI, LI, "%s : in download authlib injector , download is ok but hash not match , path : %s ,ex hash : %s , hash : %s ", FN, authlibPath.c_str(), exHash.c_str(), hash.c_str());
                return;
            }
        } // authlib injector download

        std::vector<std::string> authlibInjector = {
            "-javaagent:" + authlibPath + "=" + networkBase::buildUrl(networkBase::Api::Authlib::root, networkBase::Api::Authlib::host),
            "-Dauthlibinjector.side=client",
            "-Dauthlibinjector.yggdrasil.prefetched=" + authlibPrefrtched};

        if constexpr (info::getOsName() == std::string_view("windows")) {
            std::string command = "Set-Location -Path " + psPlusArgs({gameDir}) + "\n& " + psPlusArgs({javaPath}) + psPlusArgs(jvmOptimizeArgs) + psPlusArgs(jvmArgsVec) + psPlusArgs(authlibInjector) + psPlusArgs({mainClass}) + psPlusArgs(gameArgsVec);
            std::fstream file2(info::workPath() + "/Nekolc.ps1", std::ios::in | std::ios::out | std::ios::trunc);
            file2 << command;
            file2.close();
            nlog::Info(FI, LI, "%s : command len : %zu , command : %s", FN, command.length(), command.c_str());
            std::string cmd = "cmd.exe /C powershell " + info::workPath() + "/Nekolc.ps1";
            launcherProcess(cmd.c_str(), opt, winFunc);
        } else {
            std::filesystem::current_path("." + minecraftDir);
            std::string command = "\"" + javaPath + "\"" + plusArgs(jvmOptimizeArgs) + plusArgs(jvmArgsVec) + plusArgs(authlibInjector) + plusArgs({mainClass}) + plusArgs(gameArgsVec);
            nlog::Info(FI, LI, "%s : command len : %zu , command : %s", FN, command.length(), command.c_str());
            launcherProcess(command.c_str(), opt, winFunc);
            std::filesystem::current_path(std::filesystem::current_path().parent_path());
        }
    }
    // Called when the user clicks.
    inline void launcher(launcherOpt opt, std::function<void(const ui::hintMsg &)> hintFunc = nullptr, std::function<void(bool)> winFunc = nullptr) {
        nlog::autoLog log{FI, LI, FN};
        // It can launch Lua, Java, scripts, executables, or anything else.
        // includes pre-execution checks; in short, you can fully customize it.

        if constexpr (std::string_view("minecraft")  == launcherMode) {
            launcherMinecraftAuthlibAndPrefetchedCheck(hintFunc);
            if (!launcherMinecraftTokenValidate(hintFunc))
                return;
            launcherMinecraft(opt, exec::getConfigObj(), hintFunc, winFunc);
        }

        if constexpr (std::string_view("lua") == launcherMode) {
            if (!launcherLuaPreCheck()) {
                nlog::Err(FI, LI, "%s : Error  Lua or scriptPath not exists !", FN);
                return;
            }

            std::string luaPath = std::getenv("LUA_PATH");
            // e.g: /apps/lua /apps/workdir/lua/hello.luac
            std::string command = luaPath + " " + info::workPath() + "/lua/hello.luac";
            launcherProcess(command, opt, winFunc);
        }
    }

    enum class State {
        over,
        undone,
        tryAgainLater,
    };

    struct updateInfo {
        std::string title,
            msg,
            poster,
            time;
        std::string resVersion;
        bool mandatory;

        struct urlInfo {
            std::string url;
            std::string name;
            std::string hash;
            std::string hashAlgorithm;
            bool multis;
            bool temp;
            bool randName;
            bool absoluteUrl;
            inline bool empty() {
                std::vector<bool> vec{
                    url.empty(), name.empty(), hash.empty(), hashAlgorithm.empty()};

                for (auto it : vec) {
                    if (!it)
                        return false;
                }
                return true;
            }
        };

        std::vector<urlInfo> urls;

        inline bool empty() {
            std::vector<bool> res{
                title.empty(), msg.empty(), poster.empty(), time.empty(), urls.empty()};
            for (auto it : res) {
                if (!it) {
                    return false;
                }
            }
            return true;
        }
    };

    // Return file name, if the download fails a null T value.
    template <typename T = std::string>
    inline T downloadPoster(std::function<void(const ui::hintMsg &)> hintFunc, const std::string &url) {
        if (!url.empty()) {
            network net;
            auto fileName = info::temp() + "update_" + exec::generateRandomString(10) + ".png";
            int code = 0;

            network<std::string>::Args args{url.c_str(), fileName.c_str(), &code};
            args.writeCallback = networkBase::WriteCallbackFile;
            net.Do(networkBase::Opt::downloadFile, args);
            if (code != 200) {
                nlog::Warn(FI, LI, "%s : failed to poster download", FN);
                hintFunc({info::translations(info::lang.title.warning), info::translations(info::lang.error.downloadPoster), "", 1, [](bool) {}});
                return T();
            }
            return fileName.c_str();
        }
        return T();
    }

    // over : not maintenance, undone : in maintenance
    inline State checkMaintenance(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc) {
        nlog::autoLog log{FI, LI, FN};

        loadFunc({ui::loadMsg::Type::OnlyRaw, info::translations(info::lang.loading.maintenanceInfoReq)});

        std::string res;
        std::mutex mtx;
        std::condition_variable condVar;
        bool stop = false;

        for (size_t i = 0; i < 5; ++i) {
            nlog::autoLog log{FI, LI, "Get maintenance req - " + std::to_string(i)};
            std::unique_lock<std::mutex> lock(mtx);

            network net;
            auto url = networkBase::buildUrl(networkBase::Api::mainenance + std::string("?os=") + info::getOsNameS() + "&lang=" + info::language());
            int code = 0;
            decltype(net)::Args args{url.c_str(), nullptr, &code};
            auto temp = net.get(networkBase::Opt::getContent, args);

            if (code == 200) {
                res = temp | exec::move;
                break;
            }
            // if (code ==400)
            // {
            //     hintFunc({"Error","","",1});
            // }

            if (i == 4) {

                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.networkConnectionRetryMax), "", 1, [](bool) {
                              nlog::Err(FI, LI, "%s : Retried multiple times but still unable to establish a connection. Exit", FN);
                              QApplication::quit();
                          }});
                return State::undone;
            } else {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.maintenanceInfoReq) + networkBase::errCodeReason(code), "", 2, [=, &condVar, &stop](bool check) {
                              if (!check) {
                                  stop = true;
                                  condVar.notify_one();
                                  QApplication::quit();
                              } else {
                                  condVar.notify_one();
                              }
                          }});
            }

            condVar.wait(lock);
            if (stop) {
                return State::undone;
            }
        }

        nlog::Info(FI, LI, "%s : res : %s", FN, res.c_str());
        setLoadInfoFunc(0, info::translations(info::lang.loading.maintenanceInfoParse).c_str());

        auto jsonData = nlohmann::json::parse(res, nullptr, false);
        if (jsonData.is_discarded()) {
            nlog::Info(FI, LI, "%s : failed to maintenance parse!", FN);
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.maintenanceInfoParse), "", 1, [](bool) {
                          nlog::Err(FI, LI, "%s : click , quit programs", FN);
                          QApplication::quit();
                      }});
            return State::tryAgainLater;
        }

        bool enable = jsonData["enable"].get<bool>();
        nlog::Info(FI, LI, "%s : maintenance enable : %s", FN, exec::boolTo<const char *>(enable));
        if (!enable)
            return State::over;

        std::string msg = jsonData["msg"].get<std::string>(),
                    poster = jsonData["poster"].get<std::string>(),
                    time = jsonData["time"].get<std::string>(),
                    link = jsonData["link"].get<std::string>();
        msg = time + "\n" + msg;

        setLoadInfoFunc(0, info::translations(info::lang.loading.downloadMaintenancePoster).c_str());
        auto fileName = downloadPoster(hintFunc, poster);

        ui::hintMsg hmsg{info::translations(info::lang.title.maintenance), msg, fileName, 1, [link](bool) {
                             QDesktopServices::openUrl(QUrl(link.c_str()));
                             QApplication::quit();
                         }};
        hintFunc(hmsg);
        return State::undone;
    }
    // over : none update , undone : update
    inline State checkUpdate(std::string &res) {
        nlog::autoLog log{FI, LI, FN};
        network net;
        auto url = net.buildUrl(networkBase::Api::checkUpdates);
        nlohmann::json dataJson = {
            {"core", info::getVersion()},
            {"res", info::getResVersion()},
            {"os", info::getOsName()},
            {"lang", info::language()}};
        auto data = dataJson.dump();
        auto id = std::string(FN) + "-" + exec::generateRandomString(6);
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.data = data.c_str();
        args.id = id.c_str();
        res = net.get(networkBase::Opt::postText, args);
        if (code == 204)
            return State::over;
        if (!res.empty() && code == 200) {
            return State::undone;
        } else {
            nlog::Warn(FI, LI, "%s : code : %d , res : %s", FN, code, res.c_str());
            res.clear();
            return State::tryAgainLater;
        }
    }
    // If any error occurs, return an empty object (an empty method is provided for checking).
    inline updateInfo parseUpdate(const std::string &res) {
        nlog::autoLog log{FI, LI, FN};

        nlog::Info(FI, LI, "%s : res : %s ", FN, res.c_str());
        auto jsonData = nlohmann::json::parse(res, nullptr, false);
        if (jsonData.is_discarded()) {
            nlog::Err(FI, LI, "%s : failed to update parse!", FN);
            return {};
        }
        updateInfo info{
            jsonData["title"].get<std::string>(),
            jsonData["msg"].get<std::string>(),
            jsonData["poster"].get<std::string>(),
            jsonData["time"].get<std::string>(),
            jsonData["resVersion"].get<std::string>()};

        info.mandatory = jsonData["mandatory"].get<bool>();

        for (const auto &it : jsonData["update"]) {
            info.urls.push_back({it["url"].get<std::string>(),
                                 it["name"].get<std::string>(),
                                 it["hash"].get<std::string>(),
                                 it["meta"]["hashAlgorithm"].get<std::string>(),
                                 it["meta"]["multis"].get<bool>(),
                                 it["meta"]["temp"].get<bool>(),
                                 it["meta"]["randName"].get<bool>(),
                                 it["meta"]["absoluteUrl"].get<bool>()});
        }

        if (info.urls.empty()) {
            nlog::Err(FI, LI, "%s : urls is empty!", FN);
            return {};
        }
        return info;
    }

    inline State autoUpdate(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc) {
        nlog::autoLog log{FI, LI, FN};
        std::string res;

        auto maintenanceState = checkMaintenance(hintFunc, loadFunc, setLoadInfoFunc);
        if (maintenanceState != State::over)
            return maintenanceState;

        setLoadInfoFunc(0, info::translations(info::lang.loading.checkUpdate).c_str());

        auto updateState = checkUpdate(res);
        if (updateState != State::undone)
            return updateState;

        setLoadInfoFunc(0, info::translations(info::lang.loading.updateInfoParse).c_str());
        auto data = parseUpdate(res);
        if (data.empty())
            return State::undone;

        setLoadInfoFunc(0, info::translations(info::lang.loading.downloadUpdatePoster).c_str());
        auto fileName = downloadPoster(hintFunc, data.poster);

        if (!data.mandatory) {
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            bool select = true;
            hintFunc({data.title, (data.time + "\n" + data.msg), "", 2, [&condVar, &select](bool check) {
                          if (check) {
                              select = true;
                          } else {
                              select = false;
                          }

                          condVar.notify_one();
                      }});

            condVar.wait(lock);
            if (!select) {
                return State::over;
            }
        }

        ui::loadMsg lmsg{ui::loadMsg::All, info::translations(info::lang.loading.settingDownload), data.title, data.time, data.msg, fileName, 100, 0, static_cast<int>(data.urls.size() * 2)};
        loadFunc(lmsg);

        std::vector<std::future<neko::State>> result;
        int progress = 0;
        bool stop = false;

        for (auto &it : data.urls) {
            if (it.randName)
                it.name = exec::generateRandomString(16);

            if (it.temp)
                it.name = info::temp() + it.name;

            if (!it.absoluteUrl)
                it.url = networkBase::buildUrl(it.url);
        }

        auto downloadTask = [=, &progress, &stop](int id, updateInfo::urlInfo info) {
            network net;
            int code = 0;
            decltype(net)::Args args{
                info.url.c_str(),
                info.name.c_str(),
                &code};
            std::string ids = "update-" + std::to_string(id);
            args.id = ids.c_str();
            args.writeCallback = networkBase::WriteCallbackFile;
            if (stop)
                return State::undone;

            if (info.multis) {
                if (!net.Multi(networkBase::Opt::downloadFile, {args}))
                    return State::tryAgainLater;
            } else {
                if (!net.autoRetry(networkBase::Opt::downloadFile, {args}))
                    return State::tryAgainLater;
            }
            ++progress;
            setLoadInfoFunc(progress, info::translations(info::lang.loading.downloadUpdate).c_str());
            return State::over;
        };

        auto checkHash = [=, &progress](const std::string &file, const std::string &exHash, const std::string hashAlgortihm) {
            auto hash = exec::hashFile(file, exec::mapAlgorithm(hashAlgortihm));
            if (hash == exHash) {
                nlog::Info(FI, LI, "%s : Everything is OK , file : %s  hash is matching", FN, file.c_str());
                ++progress;
                setLoadInfoFunc(progress, info::translations(info::lang.loading.downloadUpdate).c_str());
                return State::over;
            } else {
                nlog::Err(FI, LI, "%s : Hash Non-matching : file : %s  expect hash : %s , real hash : %s", FN, file.c_str(), exHash.c_str(), hash.c_str());
                return State::tryAgainLater;
            }
        };

        // push task
        for (size_t i = 0; i < data.urls.size(); ++i) {
            result.push_back(exec::getThreadObj().enqueue([=, &stop] {
                if (stop)
                    return State::undone;

                auto state1 = downloadTask(i, data.urls[i]);
                if (state1 != State::over)
                    return state1;

                return checkHash(data.urls[i].name, data.urls[i].hash, data.urls[i].hashAlgorithm);
            }));
        }

        // check result
        for (auto &it : result) {

            if (it.get() != State::over) {
                stop = true;
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.downloadUpdate), "", 2, [=](bool check) {
                              if (check) {
                                  exec::getThreadObj().enqueue([=] {
                                      autoUpdate(hintFunc, loadFunc, setLoadInfoFunc);
                                  });
                              } else {
                                  QApplication::quit();
                              }
                          }});
                return State::undone;
            }
        }

        nlog::Info(FI, LI, "%s : update is ok", FN);

        bool needExecUpdate = false;
        std::string cmd = info::workPath() + "/update " + info::workPath();

        for (const auto &it : data.urls) {
            if (it.temp) {
                needExecUpdate = true;
                cmd += (" " + it.name);
            }
        }
        if (!data.resVersion.empty()) {
            exec::getConfigObj().SetValue("more", "resVersion", data.resVersion.c_str());
        }

        if (needExecUpdate) {
            nlog::Info(FI, LI, "%s : need exec update", FN);
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);

            auto execUpdate = [=, &condVar](bool) {
                condVar.notify_all();
                QApplication::quit();
            };

            hintFunc({info::translations(info::lang.title.reStart), info::translations(info::lang.general.updateOverReStart), "", 1, execUpdate});
            auto resState = condVar.wait_for(lock, std::chrono::seconds(6));

            if (resState == std::cv_status::timeout) {
                QApplication::quit();
            }
            launchNewProcess(cmd);
        }

        return State::over;
    }

    inline State authLogin(const std::vector<std::string> &inData, std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const std::string &)> callBack) {
        nlog::autoLog log{FI, LI, FN};
        if (inData.size() < 2)
            return State::undone;

        nlohmann::json json = {
            {"username", inData[0]},
            {"password", inData[1]},
            {"requestUser", false},
            {"agent", {{"name", "Minecraft"}, {"version", 1}}}};

        auto data = json.dump();
        auto url = neko::networkBase::buildUrl(neko::networkBase::Api::Authlib::authenticate, neko::networkBase::Api::Authlib::host);
        neko::network net;
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.header = "Content-Type: application/json";
        args.data = data.c_str();
        auto res = net.get(neko::networkBase::Opt::postText, args);
        auto resData = nlohmann::json::parse(res, nullptr, false);

        if (resData.is_discarded()) {
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.jsonParse), "", 1});
            return State::undone;
        }

        auto error = resData.value("error", ""),
             errorMsg = resData.value("errorMessage", "");
        if (!error.empty() || !errorMsg.empty()) {
            hintFunc({error, errorMsg, "", 1});
            return State::tryAgainLater;
        }

        auto accessToken = resData.value("accessToken", "");
        auto uuid = resData["selectedProfile"].value("id", "");
        auto name = resData["selectedProfile"].value("name", "");

        exec::getConfigObj().SetValue("manage", "accessToken", accessToken.c_str());
        exec::getConfigObj().SetValue("manage", "uuid", uuid.c_str());
        exec::getConfigObj().SetValue("manage", "account", inData[0].c_str());
        exec::getConfigObj().SetValue("manage", "displayName", name.c_str());

        callBack(name);

        return State::over;
    }

} // namespace neko
