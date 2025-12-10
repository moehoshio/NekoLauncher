#pragma once

// Neko Module

#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>

#include <neko/log/nlog.hpp>
#include <neko/event/event.hpp>

#include <neko/function/utilities.hpp>
#include <neko/function/hash.hpp>
#include <neko/system/platform.hpp>
#include <neko/network/network.hpp>


// NekoLc project

#include "neko/app/api.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
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

    struct UpdateState {
        neko::types::State state = neko::types::State::Completed;
        std::string result;
        std::string errorMessage;
    };

    inline UpdateState checkUpdate() noexcept {
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
            .maxRetries = 5,
            .retryDelay = std::chrono::milliseconds(150),
            .successCodes = {200, 204}};

        auto result = net.executeWithRetry(retryConfig);

        if (!result.isSuccess()) {
            std::string errMsg = "Failed to check update , code : " + std::to_string(result.statusCode) + " , error : " + result.errorMessage;
            log::error(errMsg);
            std::string dbgMsg = "result : " + result.content + " , detailedErrorMessage : " + result.detailedErrorMessage;
            log::debug(dbgMsg);
            return UpdateState{.state = neko::types::State::Failed, .result = "", .errorMessage = "Failed to check update : " + result.errorMessage};
        }

        if (result.statusCode == 204)
            return UpdateState{};
        if (result.hasContent() && result.statusCode == 200) {
            auto resultContent = result.content;
            std::string infoMsg = "Check update success, has update , result : " + resultContent;
            log::info(infoMsg);
            return UpdateState{.state = neko::types::State::Completed, .result = resultContent, .errorMessage = ""};
        }

        return UpdateState{.state = neko::types::State::Failed, .result = "", .errorMessage = "Unknown error"};
    }

    // If any error occurs, return an empty object
    inline api::UpdateResponse parseUpdate(const std::string &result) noexcept {
        log::autoLog log;

        std::string dbgMsg = "result : " + result;
        log::debug(dbgMsg);
        try {
            auto jsonData = nlohmann::json::parse(result).at("updateResponse");
            api::UpdateResponse updateInfo{
                .title = jsonData.value("title", ""),
                .description = jsonData.value("description", ""),
                .posterUrl = jsonData.value("posterUrl", ""),
                .publishTime = jsonData.value("publishTime", ""),
                .resourceVersion = jsonData.value("resourceVersion", "")};

            updateInfo.isMandatory = jsonData.value("isMandatory", false);

            if (jsonData.contains("meta")) {
                api::from_json(jsonData.at("meta"), updateInfo.meta);
            }

            for (const auto &it : jsonData.at("files")) {
                const auto metaIt = it.contains("downloadMeta") ? it.find("downloadMeta") : it.find("meta");
                const auto &meta = (metaIt != it.end() && metaIt->is_object()) ? *metaIt : nlohmann::json::object();

                updateInfo.files.push_back({
                    it.value("url", ""),
                    it.value("fileName", ""),
                    it.value("checksum", ""),
                    meta.value("hashAlgorithm", ""),
                    meta.value("suggestMultiThread", false),
                    meta.value("isCoreFile", false),
                    meta.value("isAbsoluteUrl", false)});
            }
            if (!updateInfo.files.empty()) {
                return updateInfo;
            }
            log::error("files is empty!");
        } catch (nlohmann::json::parse_error &e) {
            std::string errMsg = "Failed to parse json: " + std::string(e.what());
            log::error(errMsg);
        } catch (nlohmann::json::out_of_range &e) {
            std::string errMsg = "Json key not found: " + std::string(e.what());
            log::error(errMsg);
        } catch (std::exception &e) {
            std::string errMsg = "Exception occurred: " + std::string(e.what());
            log::error(errMsg);
        }
        return {};
    }

    /**
     * @brief Perform the update process with the given update data.
     * @param data The update response data.
     */
    UpdateState update(api::UpdateResponse data) noexcept {
        if (data.empty()) {
            std::string reason = "Update data is empty";
            bus::event::publish(event::UpdateFailedEvent{.reason = reason});
            return {State::Failed, reason};
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
        };

        std::vector<std::future<ResultData>> futures;
        std::atomic<int> progress(0);
        std::atomic<bool> shouldStop(false);

        // Lambda: Download task
        auto downloadTask = [&shouldStop](neko::uint64 id, const api::UpdateResponse::File &info) -> ResultData {
            if (shouldStop.load(std::memory_order_acquire))
                return {neko::types::State::Failed, info};

            network::Network net;
            network::RequestConfig reqConfig{
                .url = info.url,
                .method = network::RequestType::DownloadFile,
                .requestId = "update-" + std::to_string(id) + "-" + util::random::generateRandomString(6),
                .fileName = info.fileName};

            if (info.suggestMultiThread) {
                if (!net.multiThreadedDownload(network::MultiDownloadConfig(reqConfig)))
                    return {neko::types::State::RetryRequired, info};
            } else {
                auto result = net.executeWithRetry({reqConfig});
                if (!result.isSuccess())
                    return {neko::types::State::RetryRequired, info};
            }
            return {neko::types::State::Completed, info};
        };

        // Lambda: Hash verification
        auto verifyHash = [&progress](const api::UpdateResponse::File &info) -> ResultData {
            auto hash = util::hash::digestFile(info.fileName, util::hash::mapAlgorithm(info.hashAlgorithm));

            if (hash == info.checksum) {
                std::string infoMsg = "Hash verification passed: " + info.fileName;
                log::info(infoMsg);

                int currentProgress = ++progress;
                bus::event::publish(event::LoadingValueChangedEvent{.progressValue = static_cast<neko::uint32>(currentProgress)});

                return {neko::types::State::Completed, info};
            }

            log::error("Hash mismatch: file: {}, expected: {}, actual: {}",{}, info.fileName, info.checksum, hash);
            return {neko::types::State::Failed, info};
        };

        // Lambda: Combined task
        auto processFile = [&](neko::uint64 i, const api::UpdateResponse::File &info) -> ResultData {
            if (shouldStop.load(std::memory_order_acquire))
                return {neko::types::State::Failed, info};

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

        // Collect results with early exit
        std::string failureReason;
        for (auto &future : futures) {
            if (shouldStop.load(std::memory_order_acquire))
                break;

            auto result = future.get();
            if (result.state != neko::types::State::Completed) {
                shouldStop.store(true, std::memory_order_release);
                failureReason = "Update failed for file: " + result.fileInfo.fileName + 
                                " (state: " + std::to_string(static_cast<int>(result.state)) + ")";
                log::error(failureReason);
                break;
            }
        }

        if (!failureReason.empty()) {
            bus::event::publish(event::UpdateFailedEvent{.reason = failureReason});
            return UpdateState{.state = neko::types::State::Failed, .result = "", .errorMessage = failureReason};
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
                    return UpdateState{.state = neko::types::State::Failed, .result = "", .errorMessage = "Update executable not found: " + updateSourcePath.string()};
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

                app::quit();
                launcherNewProcess(cmd);

                return UpdateState{.state = neko::types::State::Completed, .result = "", .errorMessage = ""};

            } catch (const std::filesystem::filesystem_error &e) {
                std::string error = std::string("Filesystem error: ") + e.what();
                log::error(error);
                return UpdateState{.state = neko::types::State::Failed, .result = "", .errorMessage = error};
            }
        }

        bus::event::publish(event::UpdateCompleteEvent{});

        return UpdateState{.state = neko::types::State::Completed, .result = "", .errorMessage = ""};
    }

    /**
     * @brief Perform the auto-update process.
     * @return UpdateState indicating the result of the update process.
     * @note This function publishes events to the event bus and may quit the application if maintenance mode is active.
     */
    inline UpdateState autoUpdate() noexcept {
        log::autoLog log;

        // Check maintenance mode
        auto maintenanceState = checkMaintenance();

        if (maintenanceState.isMaintenance) {
            std::string infoMsg = "Maintenance mode active: " + maintenanceState.message;
            log::info(infoMsg);
            
            bool shouldExit = true;
            
            if (shouldExit) {
                app::quit();
            }
            
            return {neko::types::State::Failed, "Maintenance mode active"};
        }

        // Notify progress
        std::string process = lang::tr(lang::keys::update::category, lang::keys::update::checkingForUpdates);

        bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = process});

        // Check for updates
        auto updateState = checkUpdate();
        if (!updateState.errorMessage.empty()) {
            bus::event::publish(event::UpdateFailedEvent{.reason = updateState.errorMessage});
            return UpdateState{.state = neko::types::State::RetryRequired, .result = "", .errorMessage = updateState.errorMessage};
        }

        if (updateState.result.empty()) {
            UpdateState result{.state = neko::types::State::Completed, .result = "", .errorMessage = "No update available"};
            return result;
        }

        // Parse update data
        process = lang::tr(lang::keys::update::category, lang::keys::update::parsingUpdateData);

        auto data = parseUpdate(updateState.result);
        if (data.empty()) {
            std::string error = "Failed to parse update data";
            bus::event::publish(event::UpdateFailedEvent{.reason = error});
            return {neko::types::State::ActionNeeded, error};
        }

        bus::event::publish(event::UpdateAvailableEvent{data});

        // Ask if user wants to update
        
            UpdateState result{neko::types::State::Completed, "Update cancelled by user"};
            return result;

        // Perform update
        auto finalResult = update(data);
        
        return finalResult;
    }

} // namespace neko::core::update