
// Neko Module
#include <neko/schema/types.hpp>
#include <neko/log/nlog.hpp>
#include <neko/network/network.hpp>
#include <neko/function/utilities.hpp>
#include <neko/schema/exception.hpp>

// NekoLc project
#include "neko/app/clientConfig.hpp"
#include "neko/app/lang.hpp"
#include "neko/ui/uiMsg.hpp"
#include "neko/minecraft/downloadSource.hpp"
#include "neko/minecraft/installMinecraft.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/bus/threadBus.hpp"
#include "neko/event/eventTypes.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <atomic>
#include <future>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace neko::minecraft {

    // Should not be called from the main thread, as it will block the incoming thread until completion.
    void setupMinecraftDownloads(
        DownloadSource downloadSource,
        neko::strview versionId,
        const nlohmann::json &versionJson,
        neko::strview installPath) {

        log::autoLog log;

        const std::filesystem::path basePath(installPath);

        // Ensure we have enough workers for thousands of download tasks; keep original to restore.
        const auto originalThreads = bus::thread::getThreadCount();
        if (originalThreads < 128) {
            bus::thread::setThreadCount(128);
            log::info("MC install: bump thread pool from {} to 128 for download parallelism", {}, originalThreads);
        }

        auto sendStatus = [](const std::string &msg) {
            bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = msg});
        };

        auto ensureDirectoryExists = [](const std::filesystem::path &path) {
            if (!std::filesystem::exists(path)) {
                std::filesystem::create_directories(path);
            }
        };

        auto downloadFile = [&](const std::string &url, const std::filesystem::path &dest, const std::string &prefix) {
            network::Network net;
            network::RequestConfig reqConfig{
                .url = url,
                .method = network::RequestType::DownloadFile,
                .requestId = prefix + util::random::generateRandomString(6),
                .fileName = dest.string()};

            auto res = net.executeWithRetry({reqConfig});
            if (!res.isSuccess()) {
                throw ex::NetworkError("Download failed: " + url);
            }
        };

        const auto &libraries = versionJson.at("libraries");
        const auto &assetIndex = versionJson.at("assetIndex");

        // Download asset index first
        const std::string assetIndexUrl = (downloadSource == DownloadSource::BMCLAPI)
                                              ? replaceWithBMCLAPI(assetIndex.at("url").get<std::string>())
                                              : assetIndex.at("url").get<std::string>();
        const std::filesystem::path assetIndexPath = basePath / "assets" / "indexes" / (assetIndex.at("id").get<std::string>() + ".json");
        ensureDirectoryExists(assetIndexPath.parent_path());

        sendStatus(lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::downloadingAssetIndex, "Downloading asset index..."));
        downloadFile(assetIndexUrl, assetIndexPath, "assetIndex-");

        nlohmann::json assetIndexJson;
        {
            std::ifstream ifs(assetIndexPath);
            if (!ifs.is_open()) {
                throw ex::FileError("Failed to open asset index: " + assetIndexPath.string());
            }
            assetIndexJson = nlohmann::json::parse(ifs, nullptr, true, true);
        }

        // Prepare progress display
        const auto &assetsObj = assetIndexJson.at("objects");
        const neko::uint32 progressMax = static_cast<neko::uint32>(libraries.size() + assetsObj.size() + 1); // +1 for client jar

        // Notify UI to show progress bar
        bus::event::publish(event::ShowLoadingEvent(neko::ui::LoadingMsg{
            .type = neko::ui::LoadingMsg::Type::Progress,
            .process = lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::installing, "Installing Minecraft"),
            .progressVal = 0,
            .progressMax = progressMax}));

        std::atomic<neko::uint32> progress{0};
        auto bumpProgress = [&]() {
            auto val = ++progress;
            log::info("MC install progress: {}/{}", {}, val, progressMax);
            bus::event::publish(event::LoadingValueChangedEvent{.progressValue = val});
        };

        std::mutex errorMutex;
        std::string firstError;
        std::atomic<bool> failed{false};

        auto recordFailure = [&](const std::string &msg) {
            failed.store(true, std::memory_order_release);
            std::scoped_lock lock(errorMutex);
            if (firstError.empty()) {
                firstError = msg;
            }
        };

        std::vector<std::future<void>> tasks;
        tasks.reserve(libraries.size() + assetsObj.size());

        auto scheduleDownload = [&](const std::string &url, const std::filesystem::path &dest, const std::string &prefix, const std::string &statusMsg) {
            tasks.push_back(bus::thread::submit([=, &recordFailure, &bumpProgress, &failed, &sendStatus, &downloadFile]() {
                if (failed.load(std::memory_order_acquire)) {
                    return;
                }
                try {
                    log::info("MC install downloading: {} -> {}", {}, url, dest.string());
                    sendStatus(statusMsg);
                    downloadFile(url, dest, prefix);
                    log::info("MC install downloaded: {}", {}, dest.string());
                    bumpProgress();
                } catch (const std::exception &e) {
                    log::error("MC install download failed: {} -> {} : {}", {}, url, dest.string(), e.what());
                    recordFailure(e.what());
                }
            }));
        };

        // Download libraries
        for (const auto &library : libraries) {
            const auto &artifact = library.at("downloads").at("artifact");
            std::string libraryUrl = (downloadSource == DownloadSource::BMCLAPI)
                                         ? replaceWithBMCLAPI(artifact.at("url").get<std::string>())
                                         : artifact.at("url").get<std::string>();
            std::filesystem::path libraryPath = basePath / "libraries" / artifact.at("path").get<std::string>();
            ensureDirectoryExists(libraryPath.parent_path());

            std::string status = lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::downloadingLibrary, "Downloading library...") + " " + library.value("name", "");
            scheduleDownload(libraryUrl, libraryPath, "library-", status);
        }

        // Download client jar
        const auto &client = versionJson.at("downloads").at("client");
        std::string clientUrl = (downloadSource == DownloadSource::BMCLAPI)
                                    ? replaceWithBMCLAPI(client.at("url").get<std::string>())
                                    : client.at("url").get<std::string>();
        std::filesystem::path versionDir = basePath / "versions" / (std::string("NekoServer_") + std::string(versionId));
        ensureDirectoryExists(versionDir);
        std::filesystem::path clientJarPath = versionDir / (std::string("NekoServer_") + std::string(versionId) + ".jar");

        sendStatus(lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::downloadingClient, "Downloading client jar..."));
        downloadFile(clientUrl, clientJarPath, "client-");
        bumpProgress();

        // Download assets
        for (const auto &[_, asset] : assetsObj.items()) {
            std::string assetHash = asset.at("hash").get<std::string>();
            std::string assetUrl = ((downloadSource == DownloadSource::BMCLAPI)
                                        ? "https://bmclapi2.bangbang93.com/assets/"
                                        : "https://resources.download.minecraft.net/") +
                                    assetHash.substr(0, 2) + "/" + assetHash;

            std::filesystem::path assetPath = basePath / "assets" / "objects" / assetHash.substr(0, 2) / assetHash;
            ensureDirectoryExists(assetPath.parent_path());

            std::string status = lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::downloadingAssets, "Downloading assets...");
            scheduleDownload(assetUrl, assetPath, "asset-", status);
        }

        for (auto &task : tasks) {
            task.wait();
        }
        if (originalThreads < 128) {
            bus::thread::setThreadCount(originalThreads);
            log::info("MC install: restored thread pool size to {}", {}, originalThreads);
        }
        if (failed.load(std::memory_order_acquire)) {
            throw ex::Exception("Minecraft install failed: " + firstError);
        }

        // Save version manifest
        auto saveJson = versionJson;
        saveJson["id"] = std::string("NekoServer_") + std::string(versionId);
        saveJson["jar"] = saveJson.value("id", "") + std::string(".jar");

        std::filesystem::path versionJsonPath = versionDir / (std::string("NekoServer_") + std::string(versionId) + ".json");
        sendStatus(lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::savingVersion, "Saving version manifest..."));

        std::ofstream saveFile(versionJsonPath);
        if (!saveFile.is_open()) {
            throw ex::FileError("Failed to save version json: " + versionJsonPath.string());
        }
        saveFile << saveJson.dump(4);
        bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::completed, "Minecraft install completed")});
    }

    // Should not be called from the main thread, as it will block the incoming thread until completion.
    void installMinecraft(
        neko::strview installPath,
        neko::strview targetVersion,
        DownloadSource downloadSource) {

        std::string enterMsg = "Install Minecraft: source=" + std::string(downloadSourceMap.at(downloadSource)) +
                               ", installPath=" + std::string(installPath) +
                               ", targetVersion=" + std::string(targetVersion);
        log::autoLog log{enterMsg};

        bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::fetchVersionList, "Getting version list...")});

        network::Network net;
        auto res = net.executeWithRetry({
            network::RequestConfig{
                .url = getMinecraftListUrl(downloadSource),
                .method = network::RequestType::Get,
                .requestId = "version-list-" + util::random::generateRandomString(6)}});

        if (!res.isSuccess()) {
            throw ex::NetworkError("Failed to get version list: " + res.errorMessage);
        }
        if (!res.hasContent()) {
            throw ex::NetworkError("Version list is empty!");
        }

        nlohmann::json versionListJson;
        try {
            versionListJson = nlohmann::json::parse(res.content);
        } catch (const std::exception &e) {
            throw ex::Parse(std::string("Failed to parse version list json: ") + e.what());
        }

        const auto &versions = versionListJson.at("versions");
        auto it = std::find_if(versions.begin(), versions.end(), [&](const auto &version) {
            return version.value("type", "") == "release" && version.value("id", "") == targetVersion;
        });
        if (it == versions.end()) {
            throw ex::Runtime("Failed to find target version: " + std::string(targetVersion));
        }

        bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = lang::tr(lang::keys::minecraft::category, lang::keys::minecraft::fetchVersionInfo, "Getting target version info...")});

        std::string targetVersionUrl = (downloadSource == DownloadSource::BMCLAPI)
                                           ? replaceWithBMCLAPI(it->value("url", ""))
                                           : it->value("url", "");

        auto targetVersionRes = net.executeWithRetry({
            network::RequestConfig{
                .url = targetVersionUrl,
                .method = network::RequestType::Get,
                .requestId = "target-version-" + util::random::generateRandomString(6)}});

        if (!targetVersionRes.isSuccess()) {
            throw ex::NetworkError("Failed to get target version info: " + targetVersionRes.errorMessage);
        }

        nlohmann::json versionJson;
        try {
            versionJson = nlohmann::json::parse(targetVersionRes.content);
        } catch (const std::exception &e) {
            throw ex::Parse(std::string("Failed to parse target version json: ") + e.what());
        }

        setupMinecraftDownloads(downloadSource, targetVersion, versionJson, installPath);
    }

} // namespace neko::minecraft
