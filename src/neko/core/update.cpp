#pragma once

// Neko Module

#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>

#include <neko/log/nlog.hpp>
#include <neko/event/event.hpp>

#include <neko/function/utilities.hpp>
#include <neko/function/hash.hpp>
#include <neko/function/archive.hpp>
#include <neko/system/platform.hpp>
#include <neko/network/network.hpp>


// NekoLc project

#include "neko/app/api.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/core/update.hpp"
#include "neko/core/remoteConfig.hpp"
#include "neko/core/downloadPoster.hpp"
#include "neko/core/launcherProcess.hpp"
#include "neko/core/maintenance.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/bus/threadBus.hpp"

#include "neko/ui/uiMsg.hpp"

// other libraries

#include <nlohmann/json.hpp>

// STL
#include <atomic>
#include <condition_variable>
#include <mutex>

#include <algorithm>
#include <optional>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <source_location>

#include <string>
#include <string_view>

namespace neko::core::update {

    /**
     * @brief Check for updates from the update server.
     * @return Optional JSON payload containing update info when available. nullopt if no update is available.
     * @throws ex::NetworkError if the network request fails.
     * @throws ex::Exception for unexpected responses.
     */
    std::optional<std::string> checkUpdate(api::LauncherConfigResponse config) {
        log::autoLog log;
        network::Network net;

        nlohmann::json updateRequest = app::getRequestJson("updateRequest");
        auto url = network::buildUrl(lc::api::checkUpdates);

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .postData = updateRequest.dump()};

        network::RetryConfig retryConfig{
            .config = reqConfig,
            .maxRetries = config.maxRetryCount,
            .retryDelay = std::chrono::seconds(config.retryIntervalSec),
            .successCodes = {200, 204}};

        auto result = net.executeWithRetry(retryConfig);

        if (!result.isSuccess()) {
            std::string errMsg = "Failed to check update , code : " + std::to_string(result.statusCode) + " , error : " + result.errorMessage;
            log::error(errMsg);
            std::string dbgMsg = "result : " + result.content + " , detailedErrorMessage : " + result.detailedErrorMessage;
            log::debug(dbgMsg);
            throw ex::NetworkError("Failed to check update : " + result.errorMessage);
        }

        if (result.statusCode == 204)
            return std::nullopt;
        if (result.hasContent() && result.statusCode == 200) {
            auto resultContent = result.content;
            std::string infoMsg = "Check update success, has update , result : " + resultContent;
            log::info(infoMsg);
            return resultContent;
        }

        throw ex::Exception("Unexpected response while checking updates: status = " + std::to_string(result.statusCode));
    }


    /**
     * @brief Parse the update response from JSON string.
     * @param result The JSON string containing the update response.
     * @throws ex::Parse if JSON parsing fails.
     * @throws ex::OutOfRange if expected keys are missing in the JSON.
     * @throws ex::Exception for other exceptions during parsing.
     * @return The parsed UpdateResponse object.
     */
    api::UpdateResponse parseUpdate(const std::string &result) {
        log::autoLog log;

        std::string dbgMsg = "result : " + result;
        log::debug(dbgMsg);
        try {
            auto jsonData = nlohmann::json::parse(result).at("updateResponse");

            auto requiredString = [](const nlohmann::json &j, std::string_view key) -> std::string {
                return j.at(key).get<std::string>();
            };

            api::UpdateResponse updateInfo{
                .title = requiredString(jsonData, "title"),
                .description = requiredString(jsonData, "description"),
                .posterUrl = requiredString(jsonData, "posterUrl"),
                .publishTime = requiredString(jsonData, "publishTime"),
                .resourceVersion = requiredString(jsonData, "resourceVersion")};

            updateInfo.isMandatory = jsonData.at("isMandatory").get<bool>();

            if (jsonData.contains("meta")) {
                api::from_json(jsonData.at("meta"), updateInfo.meta);
            }

            const auto &filesJson = jsonData.at("files");
            for (const auto &it : filesJson) {
                const auto metaIt = it.find("downloadMeta");
                const auto &meta = (metaIt != it.end() && metaIt->is_object()) ? *metaIt : nlohmann::json::object();

                api::UpdateResponse::File file{
                    .url = it.at("url").get<std::string>(),
                    .fileName = it.at("fileName").get<std::string>(),
                    .checksum = it.at("checksum").get<std::string>(),
                    .hashAlgorithm = meta.value("hashAlgorithm", "sha256"),
                    .suggestMultiThread = meta.value("suggestMultiThread", true),
                    .isCoreFile = meta.value("isCoreFile", false),
                    .isAbsoluteUrl = meta.value("isAbsoluteUrl", true)};

                updateInfo.files.push_back(std::move(file));
            }
            if (!updateInfo.files.empty()) {
                return updateInfo;
            }
            log::error("files is empty!");
        } catch (nlohmann::json::parse_error &e) {
            std::string errMsg = "Failed to parse json: " + std::string(e.what());
            log::error(errMsg);
            throw ex::Parse(errMsg);
        } catch (nlohmann::json::out_of_range &e) {
            std::string errMsg = "Json key not found: " + std::string(e.what());
            log::error(errMsg);
            throw ex::OutOfRange(errMsg);
        } catch (std::exception &e) {
            std::string errMsg = "Exception occurred: " + std::string(e.what());
            log::error(errMsg);
            throw ex::Exception(errMsg);
        }
        return {};
    }

    /**
     * @brief Perform the update process with the given update data.
     * @param data The update response data.
     * @throws ex::NetworkError if any network operation fails.
     * @throws ex::FileError if file operations fail.
     * @throws ex::InvalidArgument if the update data is invalid.
     * @throws ex::Exception for other errors during the update process.
     */
    void update(api::UpdateResponse data) {
        if (data.empty()) {
            std::string reason = "Update data is empty";
            bus::event::publish(event::UpdateFailedEvent{.reason = reason});
            throw ex::InvalidArgument(reason);
        }

        auto notifyProgress = [&](const std::string &msg) {
            bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = msg});
        };

        notifyProgress(lang::tr(lang::keys::update::category, lang::keys::update::startingUpdate));

        std::string infoMsg = "Update available: " + data.title + " - " + data.description + " , resource version: " + data.resourceVersion;
        log::info(infoMsg);

        // Prepare file URLs and paths
        for (auto &it : data.files) {
            it.fileName = (it.isCoreFile ? system::tempFolder() : system::workPath()) + "/" + it.fileName;

            if (!it.isAbsoluteUrl) {
                it.url = network::buildUrl(it.url);
            }
        }

        struct ResultData {
            neko::State state;
            api::UpdateResponse::File fileInfo;
            std::string failureReason;
        };

        std::vector<std::future<ResultData>> futures;
        std::atomic<int> progress(0);
        std::atomic<bool> shouldStop(false);

        auto extractArchive = [](const std::string &filePath) -> std::optional<std::string> {
            archive::ExtractConfig cfg{
                .inputArchivePath = filePath,
                .destDir = system::workPath(),
                .overwrite = true};
            try {
                archive::zip::extract(cfg);
                return std::nullopt;
            } catch (const ex::Exception &e) {
                return std::string{"Extract failed for "} + filePath + ": " + e.what();
            } catch (const std::exception &e) {
                return std::string{"Extract failed for "} + filePath + ": " + e.what();
            }
        };

        // Lambda: Download task
        auto downloadTask = [&shouldStop](neko::uint64 id, const api::UpdateResponse::File &info) -> ResultData {
            if (shouldStop.load(std::memory_order_acquire))
                return {neko::types::State::Failed, info, "Update aborted"};

            network::Network net;
            network::RequestConfig reqConfig{
                .url = info.url,
                .method = network::RequestType::DownloadFile,
                .requestId = "update-" + std::to_string(id) + "-" + util::random::generateRandomString(6),
                .fileName = info.fileName};

            if (info.suggestMultiThread) {
                if (!net.multiThreadedDownload(network::MultiDownloadConfig(reqConfig)))
                    return {neko::types::State::RetryRequired, info, "Multi-threaded download failed"};
            } else {
                auto result = net.executeWithRetry({reqConfig});
                if (!result.isSuccess()) {
                    std::string err = "Download failed for file: " + info.fileName +
                                      ", status code: " + std::to_string(result.statusCode) +
                                      ", error: " + result.errorMessage;
                    return {neko::types::State::RetryRequired, info, err};
                }
            }
            return {neko::types::State::Completed, info, ""};
        };

        // Lambda: Hash verification
        auto verifyHash = [&progress](const api::UpdateResponse::File &info) -> ResultData {
            auto hash = util::hash::digestFile(info.fileName, util::hash::mapAlgorithm(info.hashAlgorithm));

            if (hash == info.checksum) {
                std::string infoMsg = "Hash verification passed: " + info.fileName;
                log::info(infoMsg);

                int currentProgress = ++progress;
                bus::event::publish(event::LoadingValueChangedEvent{.progressValue = static_cast<neko::uint32>(currentProgress)});

                return {neko::types::State::Completed, info, ""};
            }

            std::string err = "Hash mismatch for file: " + info.fileName +
                              ", expected: " + info.checksum +
                              ", actual: " + hash;
            log::error(err);
            return {neko::types::State::Failed, info, err};
        };

        // Lambda: Combined task
        auto processFile = [&](neko::uint64 i, const api::UpdateResponse::File &info) -> ResultData {
            if (shouldStop.load(std::memory_order_acquire))
                return {neko::types::State::Failed, info, "Update aborted"};

            auto downloadResult = downloadTask(i, info);
            if (downloadResult.state != neko::types::State::Completed)
                return downloadResult;

            return verifyHash(info);
        };

        // Submit all tasks
        futures.reserve(data.files.size());
        for (size_t i = 0; i < data.files.size(); ++i) {
            futures.push_back(bus::thread::submit(processFile, i, data.files[i]));
        }

        // Collect results with early exit; drain remaining futures to avoid stray work
        neko::types::State failureState = neko::types::State::Completed;
        std::string failureReason;
        for (auto &future : futures) {
            auto result = future.get();
            if (result.state != neko::types::State::Completed) {
                shouldStop.store(true, std::memory_order_release);
                failureState = result.state;
                failureReason = !result.failureReason.empty()
                                   ? result.failureReason
                                   : "Update failed for file: " + result.fileInfo.fileName +
                                         " (state: " + std::to_string(static_cast<int>(result.state)) + ")";
                log::error(failureReason);
                break;
            }
        }

        if (failureState != neko::types::State::Completed) {
            for (auto &future : futures) {
                if (future.valid()) {
                    future.wait();
                }
            }

            bus::event::publish(event::UpdateFailedEvent{.reason = failureReason});
            if (failureState == neko::types::State::RetryRequired) {
                throw ex::NetworkError(failureReason);
            }
            throw ex::Exception(failureReason);
        }

        // Extract compressed archives into work path (zip supported)
        for (const auto &file : data.files) {
            if (!util::string::matchExtensionNames(file.fileName, {".zip"})) {
                continue;
            }
            if (auto err = extractArchive(file.fileName)) {
                bus::event::publish(event::UpdateFailedEvent{.reason = *err});
                throw ex::FileError(*err);
            }
            log::info("Extracted archive during update: " + file.fileName + " -> " + system::workPath());
        }

        log::info("All files downloaded and verified successfully");

        // Prepare update command
        std::vector<std::string> coreFiles;
        for (const auto &file : data.files) {
            if (file.isCoreFile)
                coreFiles.push_back(file.fileName);
        }

        // Save resource version
        if (!data.resourceVersion.empty()) {
            bus::config::updateClientConfig([&data](neko::ClientConfig &cfg) {
                cfg.main.resourceVersion = data.resourceVersion.c_str();
            });
            std::string infoMsg = "Saved resource version: " + data.resourceVersion;
            log::info(infoMsg);
            bus::config::save(app::getConfigFileName());
        }

        // Execute update if core files need replacement
        if (!coreFiles.empty()) {
            try {
                std::string updateExecPath = system::tempFolder() + "/update_" + util::random::generateRandomString(10);

                std::filesystem::path updateSourcePath = system::workPath() + "/update";
                if (!std::filesystem::exists(updateSourcePath)) {
                    std::string err = "Update executable not found: " + updateSourcePath.string();
                    bus::event::publish(event::UpdateFailedEvent{.reason = err});
                    throw ex::FileError(err);
                }

                std::filesystem::create_directories(updateExecPath);
                std::filesystem::copy(updateSourcePath,
                                      updateExecPath + "/update",
                                      std::filesystem::copy_options::overwrite_existing);

                std::string cmd = updateExecPath + "/update " + system::workPath();
                for (const auto &file : coreFiles) {
                    cmd += " " + file;
                }

                std::string infoMsg = "Executing update command: " + cmd;
                log::info(infoMsg);

                bus::event::publish(event::UpdateCompleteEvent{});
                bus::event::publish(event::RestartRequestEvent{.reason = "Update applied", .command = cmd});
                return;
            } catch (const std::filesystem::filesystem_error &e) {
                std::string error = std::string("Filesystem error: ") + e.what();
                log::error(error);
                bus::event::publish(event::UpdateFailedEvent{.reason = error});
                throw ex::FileError(error);
            }
        }

        bus::event::publish(event::UpdateCompleteEvent{});
    }

    /**
     * @brief Perform the auto-update process.
     * @note This function publishes events to the event bus and may quit the application if maintenance mode is active.
     */
    void autoUpdate() {
        log::autoLog log;

        try {
            MaintenanceInfo maintenanceState = checkMaintenance();

            if (maintenanceState.isMaintenance) {
                std::string infoMsg = "Maintenance mode active: " + maintenanceState.message;
                log::info(infoMsg);

                // Maintenance notice is already shown; halt update without forcing exit
                return;
            }

            // Notify progress
            std::string process = lang::tr(lang::keys::update::category, lang::keys::update::checkingForUpdates);
            bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = process});

            // Check for updates
            auto updatePayload = checkUpdate();
            if (!updatePayload.has_value()) {
                return;
            }

            // Parse update data
            process = lang::tr(lang::keys::update::category, lang::keys::update::parsingUpdateData);

            auto data = parseUpdate(*updatePayload);
            if (data.empty()) {
                std::string error = "Failed to parse update data";
                bus::event::publish(event::UpdateFailedEvent{.reason = error});
                throw ex::Exception(error);
            }

            bus::event::publish(event::UpdateAvailableEvent{data});

            // Perform update
            update(std::move(data));
        } catch (const ex::Exception &e) {
            std::string reason = std::string("Auto-update failed: ") + e.what();
            log::error(reason);
            bus::event::publish(event::UpdateFailedEvent{.reason = reason});
            throw;
        } catch (const std::exception &e) {
            std::string reason = std::string("Unexpected error during auto-update: ") + e.what();
            log::error(reason);
            bus::event::publish(event::UpdateFailedEvent{.reason = reason});
            throw ex::Exception(reason);
        }
    }

} // namespace neko::core::update