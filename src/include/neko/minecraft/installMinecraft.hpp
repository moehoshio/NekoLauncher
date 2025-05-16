#pragma once


#include "neko/log/nlog.hpp"

#include "neko/schema/nerr.hpp"
#include "neko/schema/wmsg.hpp"
#include "neko/schema/clientconfig.hpp"

#include "neko/network/network.hpp"

#include "library/nlohmann/json.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <functional>

namespace neko {

    enum class DownloadSource {
        Official,
        BMCLAPI
    };

    inline std::map<DownloadSource, std::string_view> downloadSourceMap = {
        {DownloadSource::Official, "Official"},
        {DownloadSource::BMCLAPI, "BMCLAPI"}};

    inline std::string getMinecraftListUrl(DownloadSource downloadSource = DownloadSource::Official) {
        nlog::autoLog log{FI, LI, FN};

        static const std::map<DownloadSource, std::string_view> urlMap = {
            {DownloadSource::Official, "https://piston-meta.mojang.com/mc/game/version_manifest.json"},
            {DownloadSource::BMCLAPI, "https://bmclapi2.bangbang93.com/mc/game/version_manifest.json"}};

        auto it = urlMap.find(downloadSource);
        if (it != urlMap.end()) {
            return std::string(it->second);
        }
        return std::string(urlMap.at(DownloadSource::Official));
    }

    inline std::string replaceWithBMCLAPI(const std::string &url) {
        std::string newUrl = url;
        const std::vector<std::pair<std::string, std::string>> mojangUrls = {
            {"https://piston-meta.mojang.com", "https://bmclapi2.bangbang93.com"},
            {"https://launchermeta.mojang.com", "https://bmclapi2.bangbang93.com"},
            {"https://launcher.mojang.com", "https://bmclapi2.bangbang93.com"},
            {"https://libraries.minecraft.net", "https://bmclapi2.bangbang93.com/maven"},
            {"https://resources.download.minecraft.net", "https://bmclapi2.bangbang93.com/assets"},
            {"https://files.minecraftforge.net", "https://bmclapi2.bangbang93.com/maven"},
            {"https://maven.minecraftforge.net", "https://bmclapi2.bangbang93.com/maven"},
            {"https://launchermeta.mojang.com/v1/products/java-runtime", "https://bmclapi2.bangbang93.com/v1/products/java-runtime"},
            {"http://dl.liteloader.com/versions/versions.json", "https://bmclapi.bangbang93.com/maven/com/mumfrey/liteloader/versions.json"}};

        for (const auto &oldUrl : mojangUrls) {
            size_t pos = newUrl.find(oldUrl.first);
            if (pos != std::string::npos) {
                newUrl.replace(pos, oldUrl.first.length(), oldUrl.second);
            }
        }

        return newUrl;
    }

    // Should not be called from the main thread, as it will block the incoming thread until completion.
    inline void installMinecraftDownloads(DownloadSource downloadSource, const std::string &versionId, const nlohmann::json &versionJson, const std::string &installPath = "./.minecraft", std::function<void(const ui::loadMsg &)> loadFunc = nullptr, std::function<void(unsigned int val, const char *msg)> setLoadInfo = nullptr) {
        nlog::autoLog log{FI, LI, FN};

        std::size_t now = 0;

        auto ensureDirectoryExists = [](const std::string &path) {
            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directories(path);
            }
        };

        auto downloadLibrary = [=, &now](const nlohmann::json &library) {
            std::string libraryUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI(library["downloads"]["artifact"]["url"]) : library["downloads"]["artifact"].value("url", "");
            std::string libraryPath = installPath + "/libraries/" + library["downloads"]["artifact"]["path"].get<std::string>();
            ensureDirectoryExists(libraryPath.substr(0, libraryPath.find_last_of('/')));
            setLoadInfo(now, library["name"].get<std::string>().c_str());
            network net;
            int code = 0;
            decltype(net)::Args args{libraryUrl.c_str(), libraryPath.c_str(), &code};
            args.writeCallback = networkBase::WriteCallbackFile;
            nlog::Info(FI, LI, "%s : Downloading library: %s", FN, libraryUrl.c_str());
            if (!net.autoRetry(networkBase::Opt::downloadFile, {args})) {
                throw nerr::Error("Failed to download library!", FI, LI, FN);
            }
            ++now;
            setLoadInfo(now, library["name"].get<std::string>().c_str());
        };

        auto downloadClient = [=]() {
            ensureDirectoryExists(installPath);
            ensureDirectoryExists(installPath + "/versions/NekoServer_" + versionId);

            std::string clientJarPath = installPath + "/versions/NekoServer_" + versionId + "/NekoServer_" + versionId + ".jar";
            std::string clientJarUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI(versionJson["downloads"]["client"]["url"]) : versionJson["downloads"]["client"].value("url", "");

            network net;
            int code = 0;
            decltype(net)::Args args{clientJarUrl.c_str(), clientJarPath.c_str(), &code};
            args.writeCallback = networkBase::WriteCallbackFile;
            nlog::Info(FI, LI, "%s : Downloading client jar: %s", FN, clientJarUrl.c_str());
            if (!net.autoRetry(networkBase::Opt::downloadFile, {args})) {
                throw nerr::Error("Failed to download client jar!", FI, LI, FN);
            }
        };

        auto downloadAssetIndex = [=]() {
            std::string assetIndexUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI(versionJson["assetIndex"]["url"]) : versionJson["assetIndex"].value("url", "");
            std::string assetIndexPath = installPath + "/assets/indexes/" + versionJson["assetIndex"]["id"].get<std::string>() + ".json";
            ensureDirectoryExists(installPath + "/assets/indexes");
            setLoadInfo(now, "Downloading asset index");
            network net;
            int code = 0;
            decltype(net)::Args args{assetIndexUrl.c_str(), assetIndexPath.c_str(), &code};
            args.writeCallback = networkBase::WriteCallbackFile;
            nlog::Info(FI, LI, "%s : Downloading asset index: %s", FN, assetIndexUrl.c_str());
            if (!net.autoRetry(networkBase::Opt::downloadFile, {args})) {
                throw nerr::Error("Failed to download asset index!", FI, LI, FN);
            }
            setLoadInfo(now, "asset Download is ok");
        };

        auto downloadAsset = [=](const nlohmann::json &asset) {
            network net;
            int code = 0;
            std::string assetHash = asset.value("hash", " ");
            std::string assetUrl = ((downloadSource == DownloadSource::BMCLAPI) ? "https://bmclapi2.bangbang93.com/assets/" : "https://resources.download.minecraft.net/") + assetHash.substr(0, 2) + "/" + assetHash;

            std::string assetPath = installPath + "/assets/objects/" + assetHash.substr(0, 2) + "/" + assetHash;
            ensureDirectoryExists(assetPath.substr(0, assetPath.find_last_of('/')));

            decltype(net)::Args args{assetUrl.c_str(), assetPath.c_str(), &code};
            args.writeCallback = &networkBase::WriteCallbackFile;
            nlog::Info(FI, LI, "%s : Downloading asset: %s", FN, assetUrl.c_str());
            if (!net.autoRetry(networkBase::Opt::downloadFile, {args})) {
                throw nerr::Error("Failed to download asset!", FI, LI, FN);
            }
        };

        std::size_t libSize = versionJson["libraries"].size();

        downloadAssetIndex();

        auto assetIndexJson = nlohmann::json::parse(std::ifstream(installPath + "/assets/indexes/" + versionJson["assetIndex"]["id"].get<std::string>() + ".json"), nullptr, false);

        ui::loadMsg msg{ui::loadMsg::Progress, "Downloading libraries"};
        msg.progressMax = (libSize + assetIndexJson.size());
        loadFunc(msg);

        for (const auto &library : versionJson["libraries"]) {
            exec::getThreadObj().enqueue([=]() {
                downloadLibrary(library);
            });
        }

        downloadClient();

        for (const auto &asset : assetIndexJson["objects"]) {
            exec::getThreadObj().enqueue([=]() {
                nlog::Warn(FI, LI, "%s : Downloading asset: %s", FN, asset["hash"].get<std::string>().c_str());
                downloadAsset(asset);
            });
        }
        auto saveJson = versionJson;
        saveJson["id"] = "NekoServer_" + versionId;
        saveJson["jar"] = saveJson.value("id", "") + ".jar";

        std::ofstream saveFile(installPath + "/versions/NekoServer_" + versionId + "/NekoServer_" + versionId + ".json");
        saveFile << saveJson.dump(4);
        saveFile.close();
        exec::getThreadObj().wait_until_empty();
    }
    // Should not be called from the main thread, as it will block the incoming thread until completion.
    inline void installMinecraft(const std::string &installPath = "./.minecraft", const std::string &targetVersion = "1.16.5", DownloadSource downloadSource = DownloadSource::Official, std::function<void(const ui::hintMsg &)> hintFunc = nullptr, std::function<void(const ui::loadMsg &)> loadFunc = nullptr, std::function<void(unsigned int val, const char *msg)> setLoadInfo = nullptr) {
        std::string EnterMsg = std::string("Enter , downloadSource : ") + std::string(downloadSourceMap.at(downloadSource)) + ", installPath : " + installPath + ", targetVersion : " + targetVersion;
        nlog::autoLog log{FI, LI, FN, EnterMsg};

        network net;
        setLoadInfo(1, "Get version list..");

        auto url = getMinecraftListUrl(downloadSource);
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        auto versionList = net.autoRetryGet(networkBase::Opt::getContent, {args});
        if (versionList.empty()) {
            throw nerr::Error("Failed to get version list!", FI, LI, FN);
        }

        setLoadInfo(2, "parse version list..");

        nlohmann::json versionListJson = nlohmann::json::parse(versionList, nullptr, false);

        auto versions = versionListJson["versions"];
        auto it = std::find_if(versions.begin(), versions.end(), [&](const auto &version) {
            return version["type"] == "release" && version["id"] == targetVersion;
        });

        if (it == versions.end()) {
            throw nerr::Error("Failed to find target version!", FI, LI, FN);
        }

        setLoadInfo(2, "Get target version info..");
        std::string targetVersionUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI((*it).value("url", "")) : (*it).value("url", "");

        args.url = targetVersionUrl.c_str();
        auto targetVersionJson = net.autoRetryGet(networkBase::Opt::getContent, {args});

        if (targetVersionJson.empty()) {
            throw nerr::Error("Failed to download target version json!", FI, LI, FN);
        }

        nlohmann::json versionJson = nlohmann::json::parse(targetVersionJson, nullptr, false);

        installMinecraftDownloads(downloadSource, targetVersion, versionJson, installPath, loadFunc, setLoadInfo);
    }

    inline void checkAndAutoInstall(ClientConfig cfg, std::function<void(const ui::hintMsg &)> hintFunc = nullptr, std::function<void(const ui::loadMsg &)> loadFunc = nullptr, std::function<void(unsigned int val, const char *msg)> setLoadInfo = nullptr) {
        std::string resVer = (cfg.more.resourceVersion) ? std::string(cfg.more.resourceVersion) : std::string();
        if (resVer.empty()) {
            // Customize your installation logic, resource version needs to be stored after installation
            bool stop = false;
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            while (!stop) {
                try {
                    loadFunc({ui::loadMsg::OnlyRaw, info::translations(info::lang.general.installMinecraft)});
                    installMinecraft("./.minecraft", "1.16.5", DownloadSource::Official, hintFunc, loadFunc, setLoadInfo);
                    cfg.more.resourceVersion = "v0.0.1";
                    cfg.save(exec::getConfigObj(), "config.ini", cfg);
                } catch (const nerr::Error &e) {
                    hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.installMinecraft) + e.msg, "", 2, [&mtx, &condVar, &stop](bool check) {
                                  if (!check) {
                                      stop = true;
                                      QApplication::quit();
                                  }
                                  condVar.notify_all();
                              }});
                }
                condVar.wait(lock);
            }
        }
    }
} // namespace neko
