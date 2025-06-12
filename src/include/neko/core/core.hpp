#pragma once
#include "neko/schema/state.hpp"
#include "neko/schema/types.hpp"

#include "neko/schema/clientconfig.hpp"
#include "neko/schema/exception.hpp"

#include "neko/ui/uiMsg.hpp"

#include "neko/network/network.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"
#include "neko/system/platform.hpp"

#include "neko/log/nlog.hpp"

#include "neko/core/launcherProcess.hpp"

#include "library/nlohmann/json.hpp"

#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
// openurl
#include <QtGui/QDesktopServices>

#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <source_location>
#include <string>
#include <string_view>

namespace neko::core {

    struct UpdateInfo {
        std::string Title,
            msg,
            poster,
            time;
        std::string resVersion;
        bool mandatory;
        struct Url {
            std::string url;
            std::string name;
            std::string hash;
            std::string hashAlgorithm;
            bool multis;
            bool temp;
            bool randName;
            bool absoluteUrl;
            bool empty() {
                std::vector<bool> vec{
                    url.empty(), name.empty(), hash.empty(), hashAlgorithm.empty()};

                for (auto it : vec) {
                    if (!it)
                        return false;
                }
                return true;
            }
        };

        std::vector<Url> urls;

        bool empty() {
            std::vector<bool> res{
                Title.empty(), msg.empty(), poster.empty(), time.empty(), urls.empty()};
            for (auto it : res) {
                if (!it) {
                    return false;
                }
            }
            return true;
        }
    };

    // Return file name, if the download fails a null T value.
    inline std::string downloadPoster(const std::string &url) {
        if (!url.empty() && exec::isUrl(url)) {
            network::Network net;
            auto fileName = system::temporaryFolder() + "/poster_" + exec::generateRandomString(12) + ".png";
            network::RequestConfig reqConfig;

            reqConfig.setUrl(url)
                .setOutputFile(fileName)
                .setMethod(network::RequestType::DownloadFile)
                .setRequestId("poster-" + exec::generateRandomString(6));
            auto res = net.execute(reqConfig);

            if (res.isSuccess()) {
                return fileName;
            }
        }
        return std::string();
    }

    // Check maintenance information, if the server is in maintenance mode, it will return State::ActionNeeded, otherwise State::Completed.
    inline State checkMaintenance(std::function<void(const neko::ui::HintMsg &)> showHint = nullptr, std::function<void(const neko::ui::LoadMsg &)> showLoading = nullptr, std::function<void(neko::uint32)> setLoadingVal = nullptr, std::function<void(neko::cstr)> setLoadingNow = nullptr) {
        log::autoLog log;

        if (showLoading) {
            showLoading({neko::ui::LoadMsg::Type::OnlyRaw, info::translations(info::lang::LanguageKey::Loading::maintenanceInfoReq)});
        }

        std::string response;
        std::mutex mtx;
        std::condition_variable condVar;
        bool stop = false;

        for (neko::uint32 i = 0; i < 5; ++i) {
            log::autoLog log{"Maintenance req - " + std::to_string(i)};
            std::unique_lock<std::mutex> lock(mtx);

            network::Network net;

            nlohmann::json dataJson = {
                {"queryMaintenance", {{"os", system::getOsName()}, {"language", info::lang::language()}}}};
            auto url = net.buildUrl(network::NetworkBase::Api::mainenance);
            auto data = dataJson.dump();
            network::RequestConfig reqConfig;
            reqConfig.setUrl(url)
                .setMethod(network::RequestType::Post)
                .setData(data)
                .setRequestId("maintenance-" + exec::generateRandomString(6))
                .setHeader("Content-Type: application/json");
            auto result = net.execute(reqConfig);

            if (result.isSuccess()) {
                response = result.content;
                break;
            }
            auto quitHint = std::function<void(neko::uint32)>([](neko::uint32) {
                log::Err(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : Retried multiple times but still unable to establish a connection. Exit", log::srcLoc::current().function_name());
                QApplication::quit();
            });

            auto retryHint = std::function<void(neko::uint32)>([&condVar, &stop](neko::uint32 checkId) {
                if (checkId == 0) {
                    stop = true;
                    condVar.notify_one();
                    QApplication::quit();
                } else {
                    condVar.notify_one();
                }
            });
            res.errorMessage;
            res.statusCode;

            std::string msg = info::lang::translations((i == 4) ? info::lang::LanguageKey::Error::networkConnectionRetryMax : info::lang::LanguageKey::Error::maintenanceInfoReq) + res.errorMessage + "\n" + info::lang::translations((i == 4) ? info::lang::LanguageKey::Error::clickToQuit : info::lang::LanguageKey::Error::clickToRetry);
            std::vector<std::string> buttons = (i == 4) ? std::vector<std::string>{
                                                              info::lang::translations(info::lang::LanguageKey::General::ok)}
                                                        : std::vector<std::string>{info::lang::translations(info::lang::LanguageKey::General::retry), info::lang::translations(info::lang::LanguageKey::General::cancel)};

            neko::uint32 autoClose = (i == 4) ? 0 : 5000;
            if (showHint)
                showHint({info::lang::translations(info::lang::LanguageKey::Title::error), msg, "", buttons, ((i == 4) ? quitHint : retryHint), autoClose, 0});

            if (i == 4 || stop)
                return State::ActionNeeded;
        }

        nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : res : %s", log::srcLoc::current().function_name(), res.c_str());
        if (setLoadingNow)
            setLoadingNow(info::translations(info::lang::LanguageKey::Loading::maintenanceInfoParse).c_str());

        auto rawJsonData = nlohmann::json::parse(response, nullptr, false);
        if (rawJsonData.is_discarded() || !rawJsonData.contains("maintenanceInformation")) {
            nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : failed to maintenance parse!", log::srcLoc::current().function_name());
            if (showHint)
                showHint({info::translations(info::lang::LanguageKey::Title::error), info::translations(info::lang::LanguageKey::Error::maintenanceInfoParse), "", {info::lang::translations(info::lang::LanguageKey::General::ok)}, [](neko::uint32) {
                              nlog::Err(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : click , quit programs", log::srcLoc::current().function_name());
                              QApplication::quit();
                          }});
            return State::RetryRequired;
        }

        auto jsonData = rawJsonData["maintenanceInformation"];

        bool enable = jsonData["enable"].get<bool>();
        nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : maintenance enable : %s", log::srcLoc::current().function_name(), exec::boolTo<neko::cstr>(enable));
        if (!enable)
            return State::Completed;

        std::string msg = jsonData["message"].get<std::string>(),
                    poster = jsonData["poster"].get<std::string>(),
                    time = jsonData["time"].get<std::string>(),
                    link = jsonData["link"].get<std::string>();
        msg = time + "\n" + msg;

        if (setLoadingNow)
            setLoadingNow(info::translations(info::lang::LanguageKey::Loading::downloadMaintenancePoster).c_str());
        auto fileName = downloadPoster(poster);

        neko::ui::HintMsg hmsg{info::translations(info::lang::LanguageKey::Title::maintenance), msg, fileName, {info::lang::translations(info::lang::LanguageKey::General::ok)}, [link](neko::uint32) {
                                   QDesktopServices::openUrl(QUrl(QString::fromStdString(link)));
                                   QApplication::quit();
                               }};
        if (showHint)
            showHint(hmsg);
        return State::ActionNeeded;
    }

    // Check for updates, if there are updates available, it will return State::ActionNeeded, otherwise State::Completed.
    inline State checkUpdate(std::string &result) {
        nlog::autoLog log;
        network::Network net;
        auto url = net.buildUrl(network::NetworkBase::Api::checkUpdates);
        nlohmann::json dataJson = {
            {"checkUpdate",
             {{"coreVersion", info::app::getVersion()},
              {"resourceVersion", info::app::getResVersion()},
              {"os", system::getOsName()},
              {"language", info::lang::language()}}}};
        auto data = dataJson.dump();
        network::RequestConfig reqConfig;
        reqConfig.setUrl(url)
            .setMethod(network::RequestType::Post)
            .setData(data)
            .setRequestId("checkUpdate-" + exec::generateRandomString(6))
            .setHeader("Content-Type: application/json");
        auto res = net.execute(reqConfig);

        if (!res.isSuccess()) {
            nlog::Err(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : failed to check update , code : %d , error : %s", log::srcLoc::current().function_name(), res.statusCode, res.errorMessage.c_str());
            nlog::Debug(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : res : %s , detailedErrorMessage : %s ", log::srcLoc::current().function_name(), res.content.c_str(), res.detailedErrorMessage.c_str());
            if (res.statusCode == 429) {
                return State::RetryRequired;
            } else {
                return State::ActionNeeded;
            }
        }

        if (res.code == 204)
            return State::Completed;
        if (res.hasContent() && res.code == 200) {
            result = res.content;
            nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : check update success, res : %s", log::srcLoc::current().function_name(), result.c_str());
            return State::ActionNeeded;
        }

        return State::Completed;
    }

    // If any error occurs, return an empty object (an empty method is provided for checking).
    inline UpdateInfo parseUpdate(const std::string &result) {
        nlog::autoLog log;

        nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : res : %s ", log::srcLoc::current().function_name(), result.c_str());
        auto rawJsonData = nlohmann::json::parse(result, nullptr, false);
        if (rawJsonData.is_discarded() || !rawJsonData.contains("updateInformation")) {
            nlog::Err(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : failed to update parse!", log::srcLoc::current().function_name());
            return {};
        }
        auto jsonData = rawJsonData["updateInformation"];
        UpdateInfo info{
            jsonData["title"].get<std::string>(),
            jsonData["message"].get<std::string>(),
            jsonData["poster"].get<std::string>(),
            jsonData["time"].get<std::string>(),
            jsonData["resourceVersion"].get<std::string>()};

        info.mandatory = jsonData["mandatory"].get<bool>();

        for (const auto &it : jsonData["download"]) {
            info.urls.push_back({it["url"].get<std::string>(),
                                 it["name"].get<std::string>(),
                                 it["hash"].get<std::string>(),
                                 it["meta"]["hashAlgorithm"].get<std::string>(),
                                 it["meta"]["multis"].get<bool>(),
                                 it["meta"]["temp"].get<bool>(),
                                 it["meta"]["randName"].get<bool>(),
                                 it["meta"]["absoluteUrl"].get<bool>()});
        }

        if (info.urls.empty()) {
            nlog::Err(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : urls is empty!", log::srcLoc::current().function_name());
            return {};
        }
        return info;
    }

    inline State autoUpdate(std::function<void(const neko::ui::HintMsg &)> showHint = nullptr, std::function<void(const neko::ui::LoadMsg &)> showLoading = nullptr, std::function<void(neko::uint32)> setLoadingVal = nullptr, std::function<void(neko::cstr)> setLoadingNow = nullptr) {
        nlog::autoLog log;
        std::string checkUpdateResult;

        auto maintenanceState = checkMaintenance(showHint, showLoading, setLoadingVal, setLoadingNow);
        if (maintenanceState != State::Completed)
            return maintenanceState;

        if (setLoadingNow)
            setLoadingNow(info::lang::translations(info::lang::LanguageKey::Loading.checkUpdate));

        auto updateState = checkUpdate(checkUpdateResult);
        if (updateState != State::ActionNeeded)
            return updateState;

        if (setLoadingNow)
            setLoadingNow(info::lang::translations(info::lang::LanguageKey::Loading.updateInfoParse));
        auto data = parseUpdate(checkUpdateResult);
        if (data.empty())
            return State::ActionNeeded;

        if (setLoadingNow)
            setLoadingNow(info::lang::translations(info::lang::LanguageKey::Loading.downloadUpdatePoster));
        auto posterPath = downloadPoster(data.poster);

        if (!data.mandatory) {
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            bool select = true;
            showHint({data.Title, (data.time + "\n" + data.msg), "", {info::lang::translations(info::lang::LanguageKey::General::ok), info::lang::translations(info::lang::LanguageKey::General::cancel)}, [&condVar, &select](neko::uint32 checkId) {
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

        neko::ui::LoadMsg lmsg{neko::ui::LoadMsg::All, info::lang::translations(info::lang::LanguageKey::Loading.settingDownload), data.Title, data.time, data.msg, posterPath, "img/loading.gif", 100, 0, data.urls.size()};
        if (showLoading)
            showLoading(lmsg);

        std::vector<std::future<neko::State>> result;
        std::atomic<int> progress(0);
        std::atomic<bool> stop(false);

        for (auto &it : data.urls) {
            if (it.randName)
                it.name = exec::generateRandomString(16);

            if (it.temp)
                it.name = system::temporaryFolder() + "/" + it.name;
            else
                it.name = system::workPath() + "/" + it.name;

            if (!it.absoluteUrl)
                it.url = network::NetworkBase::buildUrl(it.url);
        }

        auto downloadTask = [=, &progress, &stop](int id, UpdateInfo::Url info) {
            network::Network net;
            network::RequestConfig reqConfig;
            reqConfig.setUrl(info.url)
                .setMethod(network::RequestType::DownloadFile)
                .setOutputFile(info.name)
                .setRequestId("update-" + std::to_string(id) + "-" + exec::generateRandomString(6));

            if (stop.load())
                return State::ActionNeeded;

            if (info.multis) {
                if (!net.multiThreadedDownload(network::MultiDownloadConfig(reqConfig)))
                    return State::RetryRequired;
            } else {
                if (!net.executeWithRetry(reqConfig, 3)) {
                    return State::RetryRequired;
                }
                ++progress;
                if (setLoadingVal)
                    setLoadingVal(progress.load());
                return State::Completed;
            }
        };

        auto checkHash = [=, &progress](const std::string &file, const std::string &exHash, const std::string hashAlgortihm) {
            auto hash = exec::hashFile(file, exec::mapAlgorithm(hashAlgortihm));
            if (hash == exHash) {
                nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : Everything is OK , file : %s  hash is matching", log::srcLoc::current().function_name(), file.c_str());
                ++progress;
                if (setLoadingVal)
                    setLoadingVal(progress.load());
                return State::Completed;
            } else {
                nlog::Err(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : Hash Non-matching : file : %s  expect hash : %s , real hash : %s", log::srcLoc::current().function_name(), file.c_str(), exHash.c_str(), hash.c_str());
                return State::RetryRequired;
            }
        };

        // push task
        for (size_t i = 0; i < data.urls.size(); ++i) {
            result.push_back(exec::getThreadObj().enqueue([=, &stop] {
                if (stop.load())
                    return State::ActionNeeded;

                auto state1 = downloadTask(i, data.urls[i]);
                if (state1 != State::Completed)
                    return state1;

                return checkHash(data.urls[i].name, data.urls[i].hash, data.urls[i].hashAlgorithm);
            }));
        }

        // check result
        for (auto &it : result) {

            if (it.get() != State::Completed) {
                stop.store(true);
                if (showHint)
                    showHint({info::lang::translations(info::lang::LanguageKey::Title::error), info::lang::translations(info::lang::LanguageKey::Error.downloadUpdate), "", {info::lang::translations(info::lang::LanguageKey::General::ok), info::lang::translations(info::lang::LanguageKey::General::cancel)}, [=](neko::uint32 checkId) {
                                  if (checkId == 0) {
                                      core::getThreadPool().enqueue([=] {
                                          autoUpdate(showHint, showLoading, setLoadingVal, setLoadingNow);
                                      });
                                  } else {
                                      QApplication::quit();
                                  }
                              }});
                return State::ActionNeeded;
            }
        }

        nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : update is ok", log::srcLoc::current().function_name());

        bool needExecUpdate = false;

        std::string updateTempPath = system::temporaryFolder() + "/update_" + exec::generateRandomString(10);
        std::filesystem::copy(system::workPath() + "/update", updateTempPath + "/update");

        std::string cmd = updateTempPath + "/update " + system::workPath();

        for (const auto &it : data.urls) {
            if (it.temp) {
                if (!needExecUpdate)
                    needExecUpdate = true;
                cmd += (" " + it.name);
            }
        }
        if (!data.resVersion.empty()) {
            ClientConfig cfg(core::getConfigObj());
            cfg.more.resourceVersion = data.resVersion.c_str();
            cfg.save(core::getConfigObj(), info::app::getConfigFileName());
            nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : save resource version : %s", log::srcLoc::current().function_name(), data.resVersion.c_str());
        }

        if (needExecUpdate) {
            nlog::Info(log::srcLoc::current().file_name(), log::srcLoc::current().line(), "%s : need exec update", log::srcLoc::current().function_name());
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);

            auto execUpdate = [=, &condVar](bool) {
                condVar.notify_all();
                QApplication::quit();
            };

            if (showHint)
                showHint({info::lang::translations(info::lang::LanguageKey::Title::reStart), info::lang::translations(info::lang::LanguageKey::General::updateOverReStart), "", {info::lang::translations(info::lang::LanguageKey::General::ok), info::lang::translations(info::lang::LanguageKey::General::ok)}, execUpdate});
            auto resState = condVar.wait_for(lock, std::chrono::seconds(6));

            if (resState == std::cv_status::timeout) {
                QApplication::quit();
            }
            launcherNewProcess(cmd);
        }

        return State::Completed;
    }

    inline void feedbackLog(const std::string &feedback) {
        nlog::autoLog log;
        network::Network net;
        auto url = net.buildUrl(network::NetworkBase::Api::feedback);
        nlohmann::json dataJson = {
            {"feedbacklog", {{"coreVersion", info::app::getVersion()}, {"resourceVersion", info::app::getResVersion()}, {"os", info::app::getOsName()}, {"language", info::lang::language()}, {"timestamp", exec::getTimestamp()}, {"content", feedback}}}};

        network::RequestConfig reqConfig;
        reqConfig.setUrl(url)
            .setMethod(network::RequestType::Post)
            .setData(dataJson.dump())
            .setRequestId("feedback-" + exec::generateRandomString(6))
            .setHeader("Content-Type: application/json");
        auto res = net.execute(reqConfig);

        if (!res.isSuccess()) {
            throw ex::NetworkError(
                (res.statusCode() == 429) ? "Too Many Request , try again later" : ("Failed to feedback log , code : " + std::to_string(res.statusCode())), ex::ExceptionExtensionInfo{});
        }
    }

} // namespace neko::core
