// /**
//  * @file installMinecraft.hpp
//  * @brief Minecraft download and installation
//  * @author moehoshio
//  * @copyright Copyright (c) 2025 Hoshi
//  * @license MIT OR Apache-2.0
//  */

// #pragma once

// // Neko Module
// #include <neko/schema/types.hpp>
// #include <neko/log/nlog.hpp>
// #include <neko/network/network.hpp>

// // NekoLc project
// #include "neko/schema/clientConfig.hpp"
// #include "neko/ui/uiMsg.hpp"
// #include "neko/minecraft/downloadSource.hpp"

// #include <nlohmann/json.hpp>

// #include <filesystem>
// #include <functional>
// #include <string>
// #include <string_view>
// #include <vector>

// namespace neko::minecraft {

//     // Should not be called from the main thread, as it will block the incoming thread until completion.
//     inline void setupMinecraftDownloads(DownloadSource downloadSource, std::string_view versionId, const nlohmann::json &versionJson, std::string_view installPath = "./.minecraft") {
//         log::autoLog log;

//         std::atomic<neko::uint32> now(0);

//         auto ensureDirectoryExists = [](std::string_view path) {
//             if (!std::filesystem::exists(path)) {
//                 std::filesystem::create_directories(path);
//             }
//         };

//         auto downloadLibrary = [&](const nlohmann::json &library) {
//             std::string libraryUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI(library["downloads"]["artifact"]["url"]) : library["downloads"]["artifact"].value("url", "");
//             std::string libraryPath = exec::sum<std::string>(installPath, "/libraries/", library["downloads"]["artifact"]["path"].get<std::string>());
//             ensureDirectoryExists(libraryPath.substr(0, libraryPath.find_last_of('/')));
//             if (setLoadingNow)
//                 setLoadingNow("Downloading library: " + library["name"].get<std::string>());

//             log::Info(log::SrcLoc::current().function_name(), log::SrcLoc::current().line(), "%s : Downloading library: %s", log::SrcLoc::current().function_name(), libraryUrl.c_str());

//             network::Network net;
//             network::RequestConfig reqConfig;
//             reqConfig.setUrl(libraryUrl)
//                 .setFileName(libraryPath)
//                 .setMethod(network::RequestType::DownloadFile)
//                 .setRequestId("library-" + exec::generateRandomString(6));

//             auto res = net.executeWithRetry(reqConfig);
//             if (!res.isSuccess()) {
//                 throw ex::NetworkError("Failed to download library: " + library["name"].get<std::string>(), ex::ExceptionExtensionInfo{});
//             }
//             if (setLoadingVal)
//                 setLoadingVal(++now);
//         };

//         auto downloadClient = [&]() {
//             ensureDirectoryExists(installPath);
//             ensureDirectoryExists(exec::sum<std::string>(installPath, "/versions/NekoServer_", versionId));

//             std::string clientJarPath = exec::sum<std::string>(installPath, "/versions/NekoServer_", versionId, "/NekoServer_", versionId, ".jar");
//             std::string clientJarUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI(versionJson["downloads"]["client"]["url"]) : versionJson["downloads"]["client"].value("url", "");

//             nlog::Info(log::SrcLoc::current().file_name(), log::SrcLoc::current().line(), log::SrcLoc::current().function_name(), "%s : Downloading client jar: %s", log::SrcLoc::current().function_name(), clientJarUrl.c_str());

//             network::Network net;
//             network::RequestConfig reqConfig;
//             reqConfig.setUrl(clientJarUrl)
//                 .setFileName(clientJarPath)
//                 .setMethod(network::RequestType::DownloadFile)
//                 .setRequestId("client-" + exec::generateRandomString(6));
//             auto res = net.executeWithRetry(reqConfig);
//             if (!res.isSuccess()) {
//                 throw ex::NetworkError("Failed to download client jar!", ex::ExceptionExtensionInfo{});
//             }
//         };

//         auto downloadAssetIndex = [&]() {
//             std::string assetIndexUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI(versionJson["assetIndex"]["url"]) : versionJson["assetIndex"].value("url", "");
//             std::string assetIndexPath = exec::sum<std::string>(installPath, "/assets/indexes/", versionJson["assetIndex"]["id"].get<std::string>(), ".json");
//             ensureDirectoryExists(exec::sum<std::string>(installPath, "/assets/indexes"));

//             if (setLoadingNow)
//                 setLoadingNow("Downloading asset index: " + versionJson["assetIndex"]["id"].get<std::string>());
//             nlog::Debug(log::SrcLoc::current().file_name(), log::SrcLoc::current().line(), log::SrcLoc::current().function_name(), "%s : Downloading asset index: %s", log::SrcLoc::current().function_name(), assetIndexUrl.c_str());

//             network::Network net;
//             network::RequestConfig reqConfig;
//             reqConfig.setUrl(assetIndexUrl)
//                 .setFileName(assetIndexPath)
//                 .setMethod(network::RequestType::DownloadFile)
//                 .setRequestId("assetIndex-" + exec::generateRandomString(6));
//             auto res = net.executeWithRetry(reqConfig);
//             if (!res.isSuccess()) {
//                 throw ex::NetworkError("Failed to download asset index!", ex::ExceptionExtensionInfo{});
//             }
//             if (setLoadingVal)
//                 setLoadingVal(++now);
//         };

//         auto downloadAsset = [&](const nlohmann::json &asset) {
//             std::string assetHash = asset.value("hash", " ");
//             std::string assetUrl = ((downloadSource == DownloadSource::BMCLAPI) ? "https://bmclapi2.bangbang93.com/assets/" : "https://resources.download.minecraft.net/") + assetHash.substr(0, 2) + "/" + assetHash;

//             std::string assetPath = exec::sum<std::string>(installPath, "/assets/objects/", assetHash.substr(0, 2), "/", assetHash);
//             ensureDirectoryExists(assetPath.substr(0, assetPath.find_last_of('/')));

//             network::Network net;
//             network::RequestConfig reqConfig;
//             reqConfig.setUrl(assetUrl)
//                 .setFileName(assetPath)
//                 .setMethod(network::RequestType::DownloadFile)
//                 .setRequestId("asset-" + exec::generateRandomString(6));
//             auto res = net.executeWithRetry(reqConfig);
//             if (!res.isSuccess()) {
//                 throw ex::NetworkError("Failed to download asset!", ex::ExceptionExtensionInfo{});
//             }
//         };

//         neko::uint32 libSize = versionJson["libraries"].size();

//         downloadAssetIndex();

//         auto assetIndexJson = nlohmann::json::parse(std::ifstream(exec::sum<std::string>(installPath, "/assets/indexes/", versionJson["assetIndex"]["id"].get<std::string>(), ".json")), nullptr, false);

//         neko::ui::LoadingMsg msg{neko::ui::LoadingMsg::Progress, "Downloading libraries"};
//         msg.progressMax = (libSize + assetIndexJson.size());
//         if (showLoading)
//             showLoading(msg);

//         for (const auto &library : versionJson["libraries"]) {
//             core::getThreadPool().enqueue([=]() {
//                 downloadLibrary(library);
//             });
//         }

//         downloadClient();

//         for (const auto &asset : assetIndexJson["objects"]) {
//             core::getThreadPool().enqueue([=]() {
//                 nlog::Warn(log::SrcLoc::current().file_name(), log::SrcLoc::current().line(), log::SrcLoc::current().function_name(), "%s : Downloading asset: %s", log::SrcLoc::current().function_name(), asset["hash"].get<std::string>().c_str());
//                 downloadAsset(asset);
//             });
//         }
//         auto saveJson = versionJson;
//         saveJson["id"] = std::string("NekoServer_") + versionId;
//         saveJson["jar"] = saveJson.value("id", "") + ".jar";

//         std::ofstream saveFile(exec::sum<std::string>(installPath, "/versions/NekoServer_", versionId, "/NekoServer_", versionId, ".json"));
//         saveFile << saveJson.dump(4);
//         saveFile.close();
//         core::getThreadPool().wait_until_empty();
//     }

//     // Should not be called from the main thread, as it will block the incoming thread until completion.
//     inline void installMinecraft(std::string_view installPath = "./.minecraft", std::string_view targetVersion = "1.16.5", DownloadSource downloadSource = DownloadSource::Official, std::function<void(const ui::NoticeMsg &)> showHint = nullptr, std::function<void(const neko::ui::LoadingMsg &)> showLoading = nullptr, std::function<void(neko::uint32)> setLoadingVal = nullptr, std::function<void(neko::cstr)> setLoadingNow = nullptr) {
//         std::string EnterMsg = exec::sum<std::string>("Enter , downloadSource : ", downloadSourceMap.at(downloadSource), ", installPath : ", installPath, ", targetVersion : ", targetVersion);
//         nlog::autoLog log{EnterMsg};

//         setLoadingNow("Get version list..");

//         auto url = getMinecraftListUrl(downloadSource);
//         network::Network net;
//         network::RequestConfig reqConfig;
//         reqConfig.setUrl(url)
//             .setMethod(network::RequestType::Get)
//             .setUserAgent(network::NetworkBase::globalConfig.userAgent)
//             .setRequestId("version-list-" + exec::generateRandomString(6));
//         auto res = net.executeWithRetry(reqConfig);
//         if (!res.isSuccess()) {
//             throw ex::NetworkError("Failed to get version list!", ex::ExceptionExtensionInfo{});
//         }

//         if (!res.hasContent()) {
//             throw ex::NetworkError("Version list is empty!", ex::ExceptionExtensionInfo{});
//         }

//         std::string versionList = res.content;

//         setLoadingNow("parse version list..");

//         nlohmann::json versionListJson;
//         try {
//             versionListJson = nlohmann::json::parse(versionList);
//         } catch (const std::exception& e) {
//             std::throw_with_nested(ex::Parse("Failed to parse version list json!", ex::ExceptionExtensionInfo{}));
//         }

//         auto versions = versionListJson["versions"];
//         auto it = std::find_if(versions.begin(), versions.end(), [&](const auto &version) {
//             return version["type"] == "release" && version["id"] == targetVersion;
//         });

//         if (it == versions.end()) {
//             throw ex::Runtime("Failed to find target version!", ex::ExceptionExtensionInfo{});
//         }

//         setLoadingNow("Get target version info..");
//         std::string targetVersionUrl = (downloadSource == DownloadSource::BMCLAPI) ? replaceWithBMCLAPI((*it).value("url", "")) : (*it).value("url", "");

//         network::RequestConfig reqConfigGetVersion;
//         reqConfigGetVersion.setUrl(targetVersionUrl)
//             .setMethod(network::RequestType::Get)
//             .setUserAgent(network::NetworkBase::globalConfig.userAgent)
//             .setRequestId("target-version-" + exec::generateRandomString(6));
//         auto targetVersionRes = net.executeWithRetry(reqConfigGetVersion);
//         if (!targetVersionRes.isSuccess()) {
//             throw ex::NetworkError("Failed to get target version info!", ex::ExceptionExtensionInfo{});
//         }

//         nlohmann::json versionJson;
//         try {
//             versionJson = nlohmann::json::parse(targetVersionRes.content);
//         } catch (const std::exception& e) {
//             std::throw_with_nested(ex::Parse("Failed to parse target version json!", ex::ExceptionExtensionInfo{}));
//         }

//         setupMinecraftDownloads(downloadSource, targetVersion, versionJson, installPath, showLoading, setLoadingVal, setLoadingNow);
//     }

// } // namespace neko::minecraft
