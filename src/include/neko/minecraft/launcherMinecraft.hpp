#include "neko/schema/clientconfig.hpp"
#include "neko/schema/nekodefine.hpp"
#include "neko/schema/wmsg.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

#include "neko/system/memoryinfo.hpp"

#include "neko/network/network.hpp"

#include "library/nlohmann/json.hpp"


namespace neko::minecraft {

    inline void launcherMinecraft(ClientConfig cfg, std::function<void(const neko::ui::hintMsg &)> hintFunc, std::function<void()> onStart, std::function<void(int)> onExit) {
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
            gameArgsName = cfg.minecraft.displayName, // player name
            gameArgsVerName = "Neko Launcher",
            gameArgsAssetsDir = gameDir + "/assets",            // game dir + /assets
            gameArgsAssetsId = verJsonData.value("assets", ""), // assets id , e.g 1.16
            gameArgsUuid = cfg.minecraft.uuid,
            gameArgsToken = cfg.minecraft.accessToken,
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
                bool allow = (rules.osArch == info::getOsArchS() && rules.action == "allow") || (rules.osArch != info::getOsArchS() && rules.action == "disallow");
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
                    single.url.c_str(), single.path.c_str(), &code};
                net.Do(networkBase::Opt::downloadFile, args);
                if (code != 200) {
                    hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftPatchDownload), "", 1});
                    nlog::Err(FI, LI, "%s : faild to archives patch download , file : %s , url : %s ", FN, single.path.c_str(), single.url.c_str());
                    return false;
                }
                auto hash = exec::hashFile(single.path, exec::hash::Algorithm::sha1);
                if (hash != single.sha1) {
                    hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftPatchDownloadHash), "", 1});
                    nlog::Err(FI, LI, "%s : faild to archives patch download , file : %s , ex sha1 : %s , sha1 : %s , size : %zu , url : %s", FN, single.path.c_str(), hash.c_str(), single.sha1.c_str(), single.size, single.url.c_str());
                    return false;
                }
                return true;
            };

            for (const auto &it : vec) {
                if (std::filesystem::exists(it.path)) {
                    auto hash = exec::hashFile(it.path, exec::hash::Algorithm::sha1);
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
        
        
        std::string maxMemory =  "-Xmx7G";

        // Get system memory information
        
        if (auto memoryInfo = system::getSystemMemoryInfo()) {
            if (memoryInfo.value().totalBytes / (1024*1024*1024) <7) {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftMemoryNotEnough) + "8GB", "", 1});
                nlog::Err(FI, LI, "%s : system memory is not enough , total memory : %zu MB", FN, memoryInfo.value().totalBytes / (1024*1024));
                return;
            }
            maxMemory = "-Xmx" + std::to_string(memoryInfo.value().totalBytes / (1024*1024*1024) - 1) + "G"; // 1 GB less than total memory
        }

        // jvm optimize args
        std::vector<std::string> jvmOptimizeArgs = {
            "-XX:+UnlockExperimentalVMOptions",
            "-XX:+UseG1GC",
            "-XX:G1NewSizePercent=20",
            "-XX:G1ReservePercent=20", 
            "-XX:MaxGCPauseMillis=50", 
            "-XX:G1HeapRegionSize=16m", 
            "-XX:-UseAdaptiveSizePolicy", 
            "-XX:-OmitStackTraceInFastThrow", 
            "-XX:-DontCompileHugeMethods", 
            "-Xmn128m", 
            maxMemory, 
            "-Dfml.ignoreInvalidMinecraftCertificates=true", 
            "-Dfml.ignorePatchDiscrepancies=true"};

        // gameArgsVec.push_back("--server");
        // gameArgsVec.push_back("");
        // gameArgsVec.push_back("--port");
        // gameArgsVec.push_back("25566");
        
        // authlib Injector
        std::string authlibPrefrtched = std::string(cfg.minecraft.authlibPrefetched);
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
            nlog::Info(FI, LI, "%s : command len : %zu , command : %s", FN, command.length(), command.c_str());
            launcherProcess(command, onStart, onExit);
        } else {
            std::filesystem::current_path("." + minecraftDir);
            std::string command = "\"" + javaPath + "\"" + plusArgs(jvmOptimizeArgs) + plusArgs(jvmArgsVec) + plusArgs(authlibInjector) + plusArgs({mainClass}) + plusArgs(gameArgsVec);
            nlog::Info(FI, LI, "%s : command len : %zu , command : %s", FN, command.length(), command.c_str());
            launcherProcess(command.c_str(), onStart, onExit);
            std::filesystem::current_path(std::filesystem::current_path().parent_path());
        }
    }
} // namespace neko::minecraft
