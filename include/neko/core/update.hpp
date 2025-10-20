#pragma once

// Neko Module

#include <neko/schema/exception.hpp>
#include <neko/schema/types.hpp>

#include <neko/event/event.hpp>
#include <neko/log/nlog.hpp>

#include <neko/function/utilities.hpp>
#include <neko/network/network.hpp>
#include <neko/system/platform.hpp>

// NekoLc project

#include "neko/app/api.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/app/eventTypes.hpp"

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
        bool isSuccess = false;
        std::string result;
        std::string errorMessage;
    };

    struct UpdateCallbacks {
        std::function<void(const std::string &process)> onProgress;
        std::function<void(const UpdateState &result)> onComplete;
        std::function<void(const std::string &error)> onError;
        /**
         * @brief Callback when an update is available.
         * @param data The update response data.
         * @return true to proceed with the update, false to skip.
         */
        std::function<bool(const api::UpdateResponse &data)> onUpdateAvailable;
        /**
         * @brief Callback when maintenance mode is active.
         * @param state The maintenance state data.
         * @return true to exit the application, false to continue.
         */
        std::function<bool(const MaintenanceState &state)> onMaintenance;
    };

    inline UpdateCallbacks createUiCallbacks() {
        return UpdateCallbacks{
            .onProgress = [](const std::string &process) { bus::event::publish<event::UpdateLoadingNowEvent>(process); },
            .onComplete = [](const UpdateState &result) {
            if (result.state == State::Completed) {
                bus::event::publish<event::UpdateCompleteEvent>();
            } else {
                ui::HintMsg msg{
                    .title = lang::tr(lang::keys::object::error),
                    .message = lang::withPlaceholdersReplaced(
                        lang::tr(lang::keys::error::updateFailed),
                        {{"{error}", result.errorMessage}}),
                    .buttonText = {lang::tr(lang::keys::button::quit), lang::tr(lang::keys::button::retry)}};
                bus::event::publish<event::ShowHintEvent>(msg);
            } },
            .onError = [](const std::string &error) { bus::event::publish<event::ShowHintEvent>(error); },
            .onUpdateAvailable = [](const api::UpdateResponse &data) {
                bus::event::publish<event::UpdateAvailableEvent>(data);
                if (data.isMandatory) {
                    return true;
                }
                // Todo: Show update dialog to user and get confirmation
                return false;
            },
            .onMaintenance = [](const MaintenanceState &state) {
                std::promise<bool> promise;
                auto future = promise.get_future();
                
                ui::HintMsg msg{
                    .title = lang::tr(lang::keys::object::maintenance),
                    .message = state.message,
                    .poster = state.poster,
                    .buttonText = {lang::tr(lang::keys::button::exit), 
                                  lang::tr(lang::keys::button::open)},
                    .callback = [&promise, state](neko::uint32 i) {
                        if (i == 1 && !state.openLinkCmd.empty()) {
                            launchProcess(state.openLinkCmd);
                        }
                        promise.set_value(true);
                    }};
                
                bus::event::publish<event::MaintenanceEvent>(msg);
                return future.get();
            }};
    }
    inline UpdateCallbacks createSilentCallbacks() {
        return UpdateCallbacks{
            .onProgress = [](const std::string &process) { log::info("Update progress: {}", process); },
            .onComplete = [](const UpdateState &result) { log::info("Update complete: {}", util::boolTo(result.state == State::Completed, "Success", result.errorMessage)); },
            .onError = [](const std::string &error) { log::error("Update error: {}", error); },
            .onUpdateAvailable = [](const UpdateResponse &data) { return true; },
            .onMaintenance = [](const MaintenanceState &state) {
                log::warn("Maintenance mode active: {}", state.message);
                if (!state.openLinkCmd.empty()) {
                    log::info("Maintenance link: {}", state.openLinkCmd);
                }
                return false;
            }};
    }

    inline UpdateState checkUpdate() noexcept {
        log::autoLog log;
        network::Network net;

        nlohmann::json updateRequest = app::getRequestJson("updateRequest");
        auto url = network::buildUrl(network::NetworkBase::Api::checkUpdates);

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .data = updateRequest.dump(),
            .requestId = "checkUpdate-" + util::random::generateRandomString(6),
            .header = network::NetworkBase::HeaderGlobal::jsonContentHeader};

        network::RetryConfig retryConfig{
            .config = reqConfig,
            .maxRetries = 5,
            .retryDelay = {150},
            .successCodes = {200, 204}};

        auto result = net.executeWithRetry(retryConfig);

        if (!result.isSuccess()) {
            log::error({}, "Failed to check update , code : {} , error : {}", result.statusCode, result.errorMessage);
            log::debug({}, "result : {} , detailedErrorMessage : {} ", result.content, result.detailedErrorMessage);
            return {false, "", "Failed to check update : " + result.errorMessage};
        }

        if (result.code == 204)
            return {};
        if (result.hasContent() && result.code == 200) {
            result = result.content;
            log::info({}, "Check update success, has update , result : {}", result);
            return {true, result};
        }

        return {false, "", "Unknown error"};
    }

    // If any error occurs, return an empty object
    inline api::UpdateResponse parseUpdate(const std::string &result) noexcept {
        log::autoLog log;

        log::debug({}, "result : {} ", result);
        try {
            auto jsonData = nlohmann::json::parse(result).at("updateResponse");
            UpdateResponse info{
                .title = jsonData.at("title").get<std::string>(),
                .description = jsonData.at("description").get<std::string>(),
                .posterUrl = jsonData.at("posterUrl").get<std::string>(),
                .publishTime = jsonData.at("publishTime").get<std::string>(),
                .resourceVersion = jsonData.at("resourceVersion").get<std::string>()};

            info.isMandatory = jsonData.at("isMandatory").get<bool>();

            for (const auto &it : jsonData.at("files")) {
                info.files.push_back({it.at("url").get<std::string>(),
                                      it.at("fileName").get<std::string>(),
                                      it.at("checksum").get<std::string>(),
                                      it.at("meta").at("hashAlgorithm").get<std::string>(),
                                      it.at("meta").at("suggestMultiThread").get<bool>(),
                                      it.at("meta").at("isCoreFile").get<bool>(),
                                      it.at("meta").at("isAbsoluteUrl").get<bool>()});
            }
            if (!info.files.empty()) {
                return info;
            }
            log::error({}, "files is empty!");
        } catch (nlohmann::json::parse_error &e) {
            log::error({}, "Failed to parse json: {}", e.what());
        } catch (nlohmann::json::out_of_range &e) {
            log::error({}, "Json key not found: {}", e.what());
        } catch (std::exception &e) {
            log::error({}, "Exception occurred: {}", e.what());
        }
        return {};
    }

    /**
     * @brief Perform the update process with the given update data.
     * @param data The update response data.
     * @param callbacks Optional callbacks for progress and state updates.
     */
    UpdateState update(api::UpdateResponse data, const UpdateCallbacks &callbacks = createUiCallbacks()) noexcept {
        if (data.empty()) {
            return {State::Failed, "Update data is empty"};
        }

        // Progress callback helper
        auto notifyProgress = [&](const std::string &msg) {
            if (callbacks.onProgress)
                callbacks.onProgress(msg);
            else
                bus::event::publish<event::UpdateLoadingNowEvent>(msg);
        };

        notifyProgress(lang::withPlaceholdersReplaced(
            lang::tr(lang::keys::action::doingAction),
            {{"{action}", lang::tr(lang::keys::action::downloadFile)},
             {"{object}", lang::tr(lang::keys::object::update)}}));

        log::info({}, "Update available: {} - {} , resource version: {}",
                  data.title, data.description, data.resourceVersion);

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
                return {State::Cancelled, info};

            network::Network net;
            network::RequestConfig reqConfig{
                .url = info.url,
                .method = network::RequestType::DownloadFile,
                .fileName = info.fileName,
                .requestId = "update-" + std::to_string(id) + "-" + util::random::generateRandomString(6)};

            if (info.suggestMultiThread) {
                if (!net.multiThreadedDownload(network::MultiDownloadConfig(reqConfig)))
                    return {State::RetryRequired, info};
            } else {
                auto result = net.executeWithRetry({reqConfig});
                if (!result.isSuccess())
                    return {State::RetryRequired, info};
            }
            return {State::Completed, info};
        };

        // Lambda: Hash verification
        auto verifyHash = [&progress, callbacks](const api::UpdateResponse::File &info) -> ResultData {
            auto hash = util::hash::hashFile(info.fileName, util::hash::mapAlgorithm(info.hashAlgorithm));

            if (hash == info.checksum) {
                log::info({}, "Hash verification passed: {}", info.fileName);

                int currentProgress = ++progress;
                if (callbacks.onProgress) {
                    callbacks.onProgress(std::format("Verified {}/{}", currentProgress, info.fileName));
                } else {
                    bus::event::publish<event::UpdateLoadingValEvent>(currentProgress);
                }

                return {State::Completed, info};
            }

            log::error({}, "Hash mismatch: file={}, expected={}, actual={}",
                       info.fileName, info.checksum, hash);
            return {State::HashMismatch, info};
        };

        // Lambda: Combined task
        auto processFile = [&](neko::uint64 i, const api::UpdateResponse::File &info) -> ResultData {
            if (shouldStop.load(std::memory_order_acquire))
                return {State::Cancelled, info};

            auto downloadResult = downloadTask(i, info);
            if (downloadResult.state != State::Completed)
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
            if (result.state != State::Completed) {
                shouldStop.store(true, std::memory_order_release);
                failureReason = std::format("Update failed for file: {} (state: {})",
                                            result.fileInfo.fileName,
                                            static_cast<int>(result.state));
                log::error({}, failureReason);
                break;
            }
        }

        if (!failureReason.empty()) {
            return {false, "", failureReason};
        }

        log::info({}, "All files downloaded and verified successfully");

        // Prepare update command
        std::vector<std::string> coreFiles;
        for (const auto &file : data.files) {
            if (file.isCoreFile)
                coreFiles.push_back(file.fileName);
        }

        // Save resource version
        if (!data.resourceVersion.empty()) {
            bus::config::updateClientConfig([&data](neko::ClientConfig &cfg) {
                cfg.other.resourceVersion = data.resourceVersion;
            });
            log::info({}, "Saved resource version: {}", data.resourceVersion);
            bus::config::save(app::getConfigFileName());
        }

        // Execute update if core files need replacement
        if (!coreFiles.empty()) {
            try {
                std::string updateExecPath = system::tempFolder() + "/update_" + util::random::generateRandomString(10);

                std::filesystem::path updateSourcePath = system::workPath() + "/update";
                if (!std::filesystem::exists(updateSourcePath)) {
                    return {false, "", "Update executable not found: " + updateSourcePath.string()};
                }

                std::filesystem::create_directories(updateExecPath);
                std::filesystem::copy(updateSourcePath,
                                      updateExecPath + "/update",
                                      std::filesystem::copy_options::overwrite_existing);

                std::string cmd = updateExecPath + "/update " + system::workPath();
                for (const auto &file : coreFiles) {
                    cmd += " " + file;
                }

                log::info({}, "Executing update command: {}", cmd);

                if (callbacks.onComplete) {
                    callbacks.onComplete({State::RestartRequired, "Update ready, restart required"});
                }

                app::quit();
                launcherNewProcess(cmd);

            } catch (const std::filesystem::filesystem_error &e) {
                std::string error = std::format("Filesystem error: {}", e.what());
                log::error({}, error);
                return {false, "", error};
            }
        }

        return {true, "", ""};
    }

    /**
     * @brief Perform the auto-update process.
     * @return UpdateState indicating the result of the update process.
     * @note This function publishes events to the event bus and may quit the application if maintenance mode is active.
     */
    inline UpdateState autoUpdate(const UpdateCallbacks &callbacks = createUiCallbacks()) noexcept {
        log::autoLog log;

        // Check maintenance mode
        auto maintenanceState = checkMaintenance();
        
        if (!maintenanceState.errorMessage.empty()) {
            log::warn({}, "Failed to check maintenance: {}", maintenanceState.errorMessage);
        }

        if (maintenanceState.isMaintenance) {
            log::info({}, "Maintenance mode active: {}", maintenanceState.message);
            
            bool shouldExit = true;
            if (callbacks.onMaintenance) {
                shouldExit = callbacks.onMaintenance(maintenanceState);
            }
            
            if (shouldExit) {
                app::quit();
            }
            
            return {State::Cancelled, "Maintenance mode active"};
        }

        // Notify progress
        std::string process = lang::withPlaceholdersReplaced(
            lang::tr(lang::keys::action::doingAction),
            {{"{action}", lang::tr(lang::keys::action::networkRequest)},
             {"{object}", lang::tr(lang::keys::object::update)}});

        if (callbacks.onProgress)
            callbacks.onProgress(process);

        // Check for updates
        auto updateState = checkUpdate();
        if (!updateState.errorMessage.empty()) {
            if (callbacks.onError)
                callbacks.onError(updateState.errorMessage);
            return {State::RetryRequired, updateState.errorMessage};
        }

        if (!updateState.isUpdateAvailable) {
            UpdateState result{State::Completed, "No update available"};
            if (callbacks.onComplete)
                callbacks.onComplete(result);
            return result;
        }

        // Parse update data
        process = lang::withPlaceholdersReplaced(
            lang::tr(lang::keys::action::doingAction),
            {{"{action}", lang::tr(lang::keys::action::parseJson)},
             {"{object}", lang::tr(lang::keys::object::update)}});

        if (callbacks.onProgress)
            callbacks.onProgress(process);

        auto data = parseUpdate(updateState.result);
        if (data.empty()) {
            std::string error = "Failed to parse update data";
            if (callbacks.onError)
                callbacks.onError(error);
            return {State::ActionNeeded, error};
        }

        // Ask if user wants to update
        if (callbacks.onUpdateAvailable && !callbacks.onUpdateAvailable(data)) {
            UpdateState result{State::Completed, "Update cancelled by user"};
            if (callbacks.onComplete)
                callbacks.onComplete(result);
            return result;
        }

        // Perform update
        auto finalResult = update(data, callbacks);
        
        if (callbacks.onComplete)
            callbacks.onComplete(finalResult);

        return finalResult;
    }

} // namespace neko::core::update