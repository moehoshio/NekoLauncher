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
namespace neko::core::update {


    struct CheckUpdateResult {
        bool isUpdateAvailable;
        std::string result;
    };
    // Check for updates, if there are updates available, it will return State::ActionNeeded, otherwise State::Completed.
    inline CheckUpdateResult checkUpdate() noexcept {
        log::autoLog log;
        network::Network net;

        nlohmann::json updateRequest = info::getRequestJson("updateRequest");
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
            .successCodes = {200, 204, 429}
        };

        auto result = net.executeWithRetry(retryConfig);

        if (!result.isSuccess()) {
            log::error({}, "Failed to check update , code : {} , error : {}", result.statusCode, result.errorMessage);
            log::debug({}, "result : {} , detailedErrorMessage : {} ", result.content, result.detailedErrorMessage);
            if (result.statusCode == 429) {
                return CheckUpdateResult{State::RetryRequired, ""};
            } else {
                return CheckUpdateResult{State::ActionNeeded, ""};
            }
        }

        if (result.code == 204)
            return CheckUpdateResult{State::Completed, ""};
        if (result.hasContent() && result.code == 200) {
            result = result.content;
            log::info({}, "Check update success, has update , result : {}", result);
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
            log::error({}, "Failed to parse json: {}", e.what());
        } catch (nlohmann::json::out_of_range &e) {
            log::error({}, "Json key not found: {}", e.what());
        } catch (std::exception &e) {
            log::error({}, "Exception occurred: {}", e.what());
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

} // namespace neko::core::update