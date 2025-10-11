#pragma once

// Neko Module

#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>

#include <neko/core/threadPool.hpp>

#include <neko/log/nlog.hpp>
#include <neko/event/event.hpp>

#include <neko/function/utilities.hpp>
#include <neko/system/platform.hpp>

#include <neko/network/network.hpp>

// NekoLc project

#include "neko/schema/api.hpp"
#include "neko/schema/clientConfig.hpp"
#include "neko/schema/eventTypes.hpp"

#include "<neko/core/app.hpp"
#include "<neko/core/appinfo.hpp"
#include "<neko/core/configManager.hpp"
#include "<neko/core/launcherProcess.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/bus/threadBus.hpp"

#include "neko/function/info.hpp"
#include "neko/function/lang.hpp"

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

namespace neko::core {

    namespace internal {

        /**
         * @throws ex::Parse if the response cannot be parsed
         * @throws ex::NetworkError if the network request fails
         */
        inline schema::LauncherConfigResponse getStaticRemoteConfig() {
            log::autoLog log;
            network::Network net;
            network::RequestConfig reqConfig;
            reqConfig.setUrl(app::getStaticRemoteConfigUrl())
                .setMethod(network::RequestType::Get)
                .setRequestId("launcher-config-" + util::random::generateRandomString(6));
            auto result = net.executeWithRetry(reqConfig);
            if (!result.isSuccess() || result.content.empty()) {
                log::error({}, "Failed to get remote launcher config: %s", result.errorMessage.c_str());
                log::debug({}, "Detailed error: %s", result.detailedErrorMessage.c_str());
                throw ex::NetworkError("Failed to get remote launcher config : " + result.errorMessage);
            }
            schema::LauncherConfigResponse response;
            try {
                nlohmann::json config = nlohmann::json::parse(result.content);
                response = config.get<schema::LauncherConfigResponse>();
            } catch (const nlohmann::json::exception &e) {
                log::error({}, "Failed to parse remote launcher config: %s", e.what());
                throw ex::Parse("Failed to parse remote launcher config: " + std::string(e.what()));
            }
            return response;
        }

        /**
         * @throws ex::Parse if the response cannot be parsed
         * @throws ex::NetworkError if the network request fails
         */
        inline schema::LauncherConfigResponse getDynamicRemoteConfig() {
            log::autoLog log;
            network::Network net;
            network::RequestConfig reqConfig;

            nlohmann::json launcherConfigRequest = info::getRequestJson("launcherConfigRequest");

            auto url = net.buildUrl(network::NetworkBase::Api::launcherConfig);

            reqConfig.setUrl(url)
                .setMethod(network::RequestType::Post)
                .setData(launcherConfigRequest.dump())
                .setRequestId("launcher-config-" + util::random::generateRandomString(6))
                .setHeader(network::NetworkBase::HeaderGlobal::jsonContentHeader);
            auto result = net.executeWithRetry(reqConfig);

            if (!result.isSuccess()) {
                log::error({}, "Failed to get remote launcher config: %s", result.errorMessage.c_str());
                log::debug({}, "Detailed error: %s", result.detailedErrorMessage.c_str());
                // If the request fails, throw a NetworkError exception
                throw ex::NetworkError("Failed to get remote launcher config: " + result.errorMessage);
            }

            try {
                // Parse the response content as JSON
                nlohmann::json config = nlohmann::json::parse(result.content);
                return config.get<schema::LauncherConfigResponse>();
            } catch (const nlohmann::json::exception &e) {
                log::error({}, "Failed to parse remote launcher config: %s", e.what());
                throw ex::Parse("Failed to parse remote launcher config: " + std::string(e.what()));
            }
        }
    } // namespace internal

    // if successful, return file name
    inline std::optional<std::string> downloadPoster(const std::string &url) noexcept {
        if (!url.empty() && util::check::isUrl(url)) {
            network::Network net;
            auto fileName = system::tempFolder() + "/poster_" + util::random::generateRandomString(12) + ".png";
            network::RequestConfig reqConfig;

            reqConfig.setUrl(url)
                .setFileName(fileName)
                .setMethod(network::RequestType::DownloadFile)
                .setRequestId("poster-" + util::random::generateRandomString(6));
            auto res = net.execute(reqConfig);

            if (res.isSuccess()) {
                return fileName;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Fetches the remote launcher configuration from the launcher config API.
     * @throws ex::NetworkError if the network request fails
     * @throws ex::Parse if the response cannot be parsed
     */
    inline schema::LauncherConfigResponse getRemoteLauncherConfig() {
        log::autoLog log;
        // Use static remote config if enabled
        if constexpr (schema::definitions::EnableStaticDeployment || schema::definitions::EnableStaticRemoteConfig) {
            return internal::getStaticRemoteConfig();
        } else {
            // If static remote config is not enabled, use the launcher config API
            return internal::getDynamicRemoteConfig();
        }
    }

    struct MaintenanceInfo {
        bool isMaintenance = false;
        std::string message;
        std::string poster; // file path
        std::string openLinkCmd; // command to open the link, e.g., "open https://example.com"
    };

    /**
     * @brief Checks if the application is under maintenance.
     * @return MaintenanceInfo containing maintenance status and details
     * @throws ex::NetworkError if the network request fails
     * @throws ex::Parse if the response cannot be parsed
     * @throws ex::OutOfRange if a required key is missing in the response
     */
    inline MaintenanceInfo checkMaintenance() {
        log::autoLog log;
        network::Network net;

        bus::event::publish(event::ShowLoadEvent(ui::LoadMsg{
            .type = ui::LoadMsg::Type::OnlyRaw,
            .process = lang::tr(lang::keys::maintenance::infoRequest)}));

        nlohmann::json maintenanceRequest = info::getRequestJson("maintenanceRequest");
        auto url = net.buildUrl(network::NetworkBase::Api::maintenance);

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .data = maintenanceRequest.dump(),
            .requestId = "maintenance-" + util::random::generateRandomString(6),
            .header = network::NetworkBase::HeaderGlobal::jsonContentHeader};
        auto result = net.executeWithRetry(reqConfig, 5, {150}, {200, 204});

        if (!result.hasError && result.statusCode == 204) {
            return {
                .isMaintenance = false
            };
        }

        if (!result.isSuccess() || !result.hasContent()) {
            throw ex::NetworkError("Failed to check maintenance status: " + result.errorMessage);
        }

        std::string response = result.content;
        bus::event::publish(event::UpdateLoadingNowEvent{
            .process = lang::tr(lang::keys::Loading::maintenanceInfoParse)});
        log::info({}, "maintenance response : {}", response);

        try {

            auto jsonData = nlohmann::json::parse(response).at("maintenanceResponse");
            schema::MaintenanceResponse maintenanceInfo = jsonData.get<schema::MaintenanceResponse>();
            maintenanceInfo.message = lang::withPlaceholdersReplaced(
                lang::tr(lang::keys::Maintenance::message),
                {{"{startTime}", maintenanceInfo.startTime},
                 {"{exEndTime}", maintenanceInfo.endTime},
                 {"{msg}", maintenanceInfo.message}});

            bus::event::publish(event::UpdateLoadingNowEvent{
                .process = lang::tr(lang::keys::Loading::downloadMaintenancePoster)});
            auto filePath = downloadPoster(maintenanceInfo.posterUrl);

            std::string command;
            if constexpr (system::isWindows()) {
                command = "start " + maintenanceInfo.link;
            } else if constexpr (system::isLinux()) {
                command = "xdg-open " + maintenanceInfo.link;
            } else if constexpr (system::isMacOS()) {
                command = "open " + maintenanceInfo.link;
            }

            return {
                .isMaintenance = true,
                .message = maintenanceInfo.message,
                .poster = filePath.value_or(""),
                .openLinkCmd = command
            };
        } catch (nlohmann::json::parse_error &e) {
            log::error({}, "Failed to parse json: %s", e.what());
            throw ex::Parse("Failed to parse json: " + std::string(e.what()));
        } catch (const nlohmann::json::out_of_range &e) {
            log::error({}, "Json key not found: {}", e.what());
            throw ex::OutOfRange("Json key not found: " + std::string(e.what()));
        }
        return {};
    }

    struct CheckUpdateResult {
        bool isUpdateAvailable;
        std::string result;
    };
    // Check for updates, if there are updates available, it will return State::ActionNeeded, otherwise State::Completed.
    inline CheckUpdateResult checkUpdate() noexcept {
        log::autoLog log;
        network::Network net;
        

        nlohmann::json updateRequest = info::getRequestJson("updateRequest");
        auto url = net.buildUrl(network::NetworkBase::Api::checkUpdates);
    
        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .data = updateRequest.dump(),
            .requestId = "checkUpdate-" + util::random::generateRandomString(6),
            .header = network::NetworkBase::HeaderGlobal::jsonContentHeader
        };

        auto result = net.executeWithRetry(reqConfig,5,{150},{200,204,429});

        if (!result.isSuccess()) {
            log::error({}, "Failed to check update , code : %d , error : %s", result.statusCode, result.errorMessage.c_str());
            log::debug({}, "res : %s , detailedErrorMessage : %s ", result.content.c_str(), result.detailedErrorMessage.c_str());
            if (result.statusCode == 429) {
                return CheckUpdateResult{State::RetryRequired, ""};
            } else {
                return CheckUpdateResult{State::ActionNeeded, ""};
            }
        }

        if (res.code == 204)
            return CheckUpdateResult{State::Completed, ""};
        if (res.hasContent() && res.code == 200) {
            result = res.content;
            log::info({}, "Check update success, has update , res : %s", result.c_str());
            return CheckUpdateResult{State::ActionNeeded, result};
        }

        return CheckUpdateResult{State::Completed, ""};
    }

    // If any error occurs, return an empty object (an empty method is provided for checking).
    inline UpdateResponse parseUpdate(const std::string &result) noexcept {
        log::autoLog log;

        log::debug({}, "res : %s ", result.c_str());
        try {
            auto jsonData = nlohmann::json::parse(result).at("updateInformation");
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
            log::error({}, "Failed to parse json: %s", e.what());
        } catch (nlohmann::json::out_of_range &e) {
            log::error({}, "Json key not found: %s", e.what());
        } catch (std::exception &e) {
            log::error({}, "Exception occurred: %s", e.what());
        }
        return {};
    }

    inline State autoUpdate(std::function<void(const neko::ui::HintMsg &)> hintDialog = nullptr, std::function<void(const neko::ui::LoadMsg &)> showLoading = nullptr, std::function<void(neko::uint32)> setLoadingVal = nullptr, std::function<void(const std::string &)> setLoadingNow = nullptr) noexcept {
        log::autoLog log;
        using namespace neko::info;

        auto maintenanceState = checkMaintenance(hintDialog, showLoading, setLoadingVal, setLoadingNow);
        if (maintenanceState != State::Completed)
            return maintenanceState;

        if (setLoadingNow)
            setLoadingNow(lang::tr(lang::keys::Loading::checkUpdate));

        auto updateState = checkUpdate();
        if (updateState.state != State::ActionNeeded)
            return updateState;

        if (setLoadingNow)
            setLoadingNow(lang::tr(lang::keys::Loading::updateInfoParse));
        auto data = parseUpdate(updateState.result);
        if (data.empty())
            return State::ActionNeeded;

        if (setLoadingNow)
            setLoadingNow(lang::tr(lang::keys::Loading::downloadUpdatePoster));
        auto posterPath = downloadPoster(data.posterUrl);

        if (!data.isMandatory) {
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            bool select = true;
            hintDialog(ui::HintMsg{
                .title = data.title,
                .message = (data.publishTime + "\n" + data.description),
                .poster = "",
                .buttonText = {lang::tr(lang::keys::Button::ok), lang::tr(lang::keys::Button::cancel)},
                .callback = [&condVar, &select](neko::uint32 checkId) {
                    if (checkId == 0) {
                        select = true;
                    } else {
                        select = false;
                    }
                    condVar.notify_one();
                }});

            condVar.wait(lock);
            if (!select) {
                return State::Completed;
            }
        }

        if (showLoading) {
            showLoading(ui::LoadMsg{
                .type = ui::LoadMsg::Type::All,
                .process = lang::tr(lang::keys::Loading::settingDownload),
                .h1 = data.title,
                .h2 = data.publishTime,
                .message = data.description,
                .poster = posterPath,
                .icon = "img/loading.gif",
                .speed = 100,
                .progressVal = 0,
                .progressMax = data.files.size()});
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
                it.url = network::NetworkBase::buildUrl(it.url);
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
                log::error({}, "Hash Non-matching : file : %s  expect hash : %s , real hash : %s", info.fileName.c_str(), info.hash.c_str(), hash.c_str());
                return {State::RetryRequired, info};
            }
            log::info({}, "Everything is OK , file : %s  hash is matching", info.fileName.c_str());
            ++progress;
            if (setLoadingVal)
                setLoadingVal(progress.load());
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
                        ui::HintMsg hmsg{
                            .title = lang::tr(lang::keys::Title::error),
                            .message = lang::withPlaceholdersReplaced(
                                lang::tr(lang::keys::Network::downloadUpdateError),
                                {"{details}", lang::tr(lang::keys::Error::clickToQuit)}),
                            .poster = "",
                            .buttonText = {lang::tr(lang::keys::Button::ok)},
                            .callback = exitCallback,
                        };
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
                if (hintDialog) {
                    hintDialog(ui::HintMsg{
                        .title = lang::tr(lang::keys::Title::error),
                        .message = lang::withPlaceholdersReplaced(
                            lang::tr(lang::keys::Network::downloadUpdateError),
                            {"{details}", res.url.fileName}),
                        .poster = "",
                        .buttonText = {lang::tr(lang::keys::Button::retry), lang::tr(lang::keys::Button::cancel)},
                        .callback = callback});
                }
                condVar.wait(lock);
                if (stop.load()) {
                    return State::ActionNeeded;
                }
            }
        }

        log::info({}, "update is ready");

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
            log::info({}, "save resource version : %s", data.resourceVersion.c_str());
        }

        if (needExecUpdate) {
            log::info({}, "need exec update");
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);

            auto execUpdate = [=, &condVar](neko::uint32) {
                condVar.notify_all();
                core::app::quit();
            };

            if (hintDialog) {
                hintDialog(ui::HintMsg{
                    .title = lang::tr(lang::keys::Title::reStart),
                    .message = lang::tr(lang::keys::Network::updateOverReStart),
                    .poster = "",
                    .buttonText = {lang::tr(lang::keys::Button::ok)},
                    .callback = execUpdate,
                    .autoClose = 5000,
                });
            }

            auto resState = condVar.wait_for(lock, std::chrono::seconds(6));

            if (resState == std::cv_status::timeout) {
                core::app::quit();
            }
            launcherNewProcess(cmd);
            core::getEventLoop().stopLoop();
        }

        return State::Completed;
    }

    inline void feedbackLog(const std::string &feedbackLog) {
        log::autoLog log;
        network::Network net;
        auto url = net.buildUrl(network::NetworkBase::Api::feedbackLog);
        nlohmann::json dataJson = {
            {"feedbacklog", {{"coreVersion", info::app::getVersion()}, {"resourceVersion", info::app::getResourceVersion()}, {"os", info::app::getOsName()}, {"language", info::lang::language()}, {"timestamp", util::time::timeToString()}, {"content", feedbackLog}}}};

        network::RequestConfig reqConfig;
        reqConfig.setUrl(url)
            .setMethod(network::RequestType::Post)
            .setData(dataJson.dump())
            .setRequestId("feedbackLog-" + util::random::generateRandomString(6))
            .setHeader(network::NetworkBase::HeaderGlobal::jsonContentHeader);
        auto res = net.execute(reqConfig);

        if (!res.isSuccess()) {
            throw ex::NetworkError(
                (res.statusCode() == 429) ? "Too Many Request , try again later" : ("Failed to feedbackLog log , code : " + std::to_string(res.statusCode())), ex::ExceptionExtensionInfo{});
        }
    }

} // namespace neko::core
