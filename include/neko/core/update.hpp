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

#include "neko/schema/api.hpp"
#include "neko/schema/clientConfig.hpp"
#include "neko/schema/eventTypes.hpp"

#include "neko/core/app.hpp"
#include "neko/core/appinfo.hpp"
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

    struct CheckUpdateResult {
        bool isUpdateAvailable = false;
        std::string result;
        std::string errorMessage;
    };

    inline CheckUpdateResult checkUpdate() noexcept {
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
            return {.isUpdateAvailable = true, .result = result};
        }

        return {false, "", "Unknown error"};
    }

    // If any error occurs, return an empty object
    inline schema::UpdateResponse parseUpdate(const std::string &result) noexcept {
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

    void update() {
        auto result = checkUpdate();

        if (!result.isUpdateAvailable) {
            log::info({}, "No update available.");
            return;
        }

        auto data = parseUpdate(result.result);

        if (data.empty()) {
            log::error({}, "Failed to parse update data.");
            return;
        }

        auto posterPath = downloadPoster(data.posterUrl);
        log::info({}, "Update available: {} - {}", data.title, data.description);
    }

    struct UpdateState {
        State state = State::Completed;
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
        std::function<bool(const schema::UpdateResponse &data)> onUpdateAvailable;
    };

    inline UpdateCallbacks createUiCallbacks() {
        return UpdateCallbacks{
            .onProgress = [](const std::string &process) { bus::event::publish<event::UpdateLoadingNowEvent>(process); },
            .onComplete = [](const UpdateState &result) {
            if (result.state == State::Completed) {
                bus::event::publish<event::UpdateCompleteEvent>();
            } },
            .onError = [](const std::string &error) { bus::event::publish<event::UpdateErrorEvent>(error); },
            .onUpdateAvailable = [](const schema::UpdateResponse &data) {
                bus::event::publish<event::UpdateAvailableEvent>(data);
                if (data.isMandatory) {
                    return true;
                }
                ui::HintMsg hmsg{
                    .title = lang::tr(lang::keys::update::updateAvailable),
                    .message = lang::withPlaceholdersReplaced(
                        lang::tr(lang::keys::update::updateAvailableMessage),
                        {{"{description}", data.description}}),
                    .poster = downloadPoster(data.posterUrl),
                    .buttonText = {lang::tr(lang::keys::button::update), lang::tr(lang::keys::button::cancel)}};
                bus::event::publish<event::ShowHintEvent>(hmsg);
            }};
    }
    inline UpdateCallbacks createSilentCallbacks() {
        return UpdateCallbacks{
            .onProgress = [](const std::string &process) { log::info("Update progress: {}", process); },
            .onComplete = [](const UpdateState &result) { log::info("Update complete: {}", util::boolTo(result.state == State::Completed, "No Error", result.errorMessage)); },
            .onError = [](const std::string &error) { log::error("Update error: {}", error); },
            .onUpdateAvailable = [](const UpdateResponse &data) { return true; }};
    }

    /**
     * @brief Perform the auto-update process.
     * @return UpdateState indicating the result of the update process.
     * @note This function publishes events to the event bus and may quit the application if maintenance mode is active.
     */
    inline UpdateState autoUpdate(UpdateCallbacks callbacks = createDefaultCallbacks()) noexcept {
        log::autoLog log;

        auto maintenanceState = checkMaintenance();

        if (maintenanceState.isMaintenance) {
            bus::event::publish<event::MaintenanceEvent>(ui::HintMsg{
                .title = lang::tr(lang::keys::object::maintenance),
                .message = maintenanceState.message,
                .poster = maintenanceState.poster,
                .buttonText = {lang::tr(lang::keys::button::exit), lang::tr(lang::keys::button::open)},
                .callback = [=](neko::uint32 i) {
                    if (i == 1 && !maintenanceState.openLinkCmd.empty()) {
                        launchProcess(maintenanceState.openLinkCmd);
                    }
                    core::app::quit();
                }});
            return {};
        }
        63 std::string process = lang::withPlaceholdersReplaced(
            lang::tr(lang::keys::action::doingAction),
            {{"{action}", lang::tr(lang::keys::action::networkRequest)},
             {"{object}", lang::tr(lang::keys::object::update)}});

        bus::event::publish<event::UpdateLoadingNowEvent>(process);

        auto updateState = checkUpdate();
        if (!updateState.errorMessage.empty())
            return {State::RetryRequired, updateState.errorMessage};

        if (!updateState.isUpdateAvailable) {
            return {State::Completed, ""};
        }

        process = lang::withPlaceholdersReplaced(
            lang::tr(lang::keys::action::doingAction),
            {{"{action}", lang::tr(lang::keys::action::parseJson)},
             {"{object}", lang::tr(lang::keys::object::update)}});

        bus::event::publish<event::UpdateLoadingNowEvent>(process);

        auto data = parseUpdate(updateState.result);
        if (data.empty())
            return {State::ActionNeeded, "Failed to parse update data"};

        auto posterPath = downloadPoster(data.posterUrl);

        if (!data.isMandatory) {
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            bool select = true;
            condVar.wait(lock);
            if (!select) {
                return {State::Completed, ""};
            }
        }

        struct ResultData {
            neko::State state;
            UpdateResponse::File url;
        };
        std::vector<std::future<ResultData>> result;
        std::atomic<int> progress(0);
        std::atomic<bool> stop(false);

        for (auto &it : data.files) {

            if (it.isCoreFile)
                it.fileName = system::tempFolder() + "/" + it.fileName;
            else
                it.fileName = system::workPath() + "/" + it.fileName;

            if (!it.isAbsoluteUrl)
                it.url = network::buildUrl(it.url);
        }

        auto downloadTask = [=, &stop](int id, UpdateResponse::File info) -> ResultData {
            network::Network net;
            network::RequestConfig reqConfig;
            reqConfig.setUrl(info.url)
                .setMethod(network::RequestType::DownloadFile)
                .setFileName(info.fileName)
                .setRequestId("update-" + std::to_string(id) + "-" + util::random::generateRandomString(6));

            if (stop.load())
                return {State::ActionNeeded, info};

            if (info.multis) {
                if (!net.multiThreadedDownload(network::MultiDownloadConfig(reqConfig)))
                    return {State::RetryRequired, info};
            } else {
                if (!net.executeWithRetry(reqConfig, 3)) {
                    return {State::RetryRequired, info};
                }
            }
            return {State::Completed, info};
        };

        auto checkHash = [=, &progress](const UpdateResponse::File &info) -> ResultData {
            auto hash = util::hash::hashFile(info.fileName, util::hash::mapAlgorithm(info.hashAlgorithm));
            if (hash != info.checksum) {
                log::error({}, "Hash Non-matching : file : {}  expect hash : {} , real hash : {}", info.fileName, info.hash, hash);
                return {State::RetryRequired, info};
            }
            log::info({}, "Everything is OK , file : {}  hash is matching", info.fileName);
            ++progress;

            return {State::Completed, info};
        };

        // push task
        for (size_t i = 0; i < data.files.size(); ++i) {
            result.push_back(core::getThreadPool().enqueue([=, &stop] {
                if (stop.load())
                    return State::ActionNeeded;

                auto state1 = downloadTask(i, data.files[i]);
                if (state1.state != State::Completed)
                    return state1;

                return checkHash(data.files[i]);
            }));
        }

        // check result
        for (size_t i = 0; i < result.size(); ++i) {
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);

            auto exitCallback = [=, &condVar](neko::uint32) {
                stop.store(true);
                condVar.notify_all();
                core::app::quit();
            };
            auto callback = [=, &condVar](neko::uint32 checkId) {
                if (checkId == 0) {
                    core::getThreadPool().enqueue([=, &stop]() {
                        if (stop.load())
                            return;

                        auto retryRes = downloadTask(i, res.url);
                        if (retryRes.state != State::Completed) {
                            hintDialog(hmsg);
                        }
                        auto hashRes = checkHash(res.url);
                        if (hashRes.state != State::Completed) {
                            hintDialog(hmsg);
                        }
                        condVar.notify_all();
                    });
                } else {
                    exitCallback(checkId);
                }
            };

            auto res = result[i].get();
            if (res.state != State::Completed) {

                condVar.wait(lock);
                if (stop.load()) {
                    return State::ActionNeeded;
                }
            }
        }

        log::info("update is ready");

        bool needExecUpdate = false;

        std::string updateTempPath = system::tempFolder() + "/update_" + util::random::generateRandomString(10);
        std::filesystem::copy(system::workPath() + "/update", updateTempPath + "/update");

        std::string cmd = updateTempPath + "/update " + system::workPath();

        for (const auto &it : data.files) {
            if (it.isCoreFile) {
                if (!needExecUpdate)
                    needExecUpdate = true;
                cmd += (" " + it.fileName);
            }
        }
        if (!data.resourceVersion.empty()) {
            ClientConfig cfg(core::getConfigObj());
            cfg.other.resourceVersion = data.resourceVersion.c_str();
            cfg.save(core::getConfigObj(), app::getConfigFileName());
            log::info({}, "save resource version : {}", data.resourceVersion);
        }

        if (needExecUpdate) {
            log::info("need exec update");
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);

            auto execUpdate = [=, &condVar](neko::uint32) {
                condVar.notify_all();
                core::app::quit();
            };

            auto resState = condVar.wait_for(lock, std::chrono::seconds(6));

            if (resState == std::cv_status::timeout) {
                core::app::quit();
            }
            launcherNewProcess(cmd);
            core::getEventLoop().stopLoop();
        }

        return State::Completed;
    }

} // namespace neko::core::update 