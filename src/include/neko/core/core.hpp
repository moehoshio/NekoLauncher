#pragma once

#include "neko/schema/clientconfig.hpp"
#include "neko/schema/nekodefine.hpp"
#include "neko/schema/wmsg.hpp"

#include "neko/network/network.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

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
#include <string>
#include <string_view>

namespace neko {

    enum class State {
        Completed, // Operation finished successfully, no further action needed
        ActionNeeded, // Action required from user or system
        RetryRequired, // Temporary failure, should retry later
    };

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
            inline bool empty() {
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

        inline bool empty() {
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
    template <typename T = std::string>
    inline T downloadPoster(std::function<void(const ui::hintMsg &)> hintFunc, const std::string &url) {
        if (!url.empty()) {
            network net;
            auto fileName = info::temp() + "/update_" + exec::generateRandomString(12) + ".png";
            int code = 0;

            decltype(net)::Args args{url.c_str(), fileName.c_str(), &code};
            args.writeCallback = networkBase::WriteCallbackFile;
            net.Do(networkBase::Opt::downloadFile, args);
            if (code != 200) {
                nlog::Warn(FI, LI, "%s : failed to poster download", FN);
                hintFunc({info::translations(info::lang.title.warning), info::translations(info::lang.error.downloadPoster), "", 1, [](bool) {}});
                return T();
            }
            return T(fileName.c_str());
        }
        return T();
    }

    // over : not maintenance, undone : in maintenance
    inline State checkMaintenance(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc) {
        nlog::autoLog log{FI, LI, FN};

        loadFunc({ui::loadMsg::Type::OnlyRaw, info::translations(info::lang.loading.maintenanceInfoReq)});

        std::string res;
        std::mutex mtx;
        std::condition_variable condVar;
        bool stop = false;

        for (size_t i = 0; i < 5; ++i) {
            nlog::autoLog log{FI, LI, "Get maintenance req - " + std::to_string(i)};
            std::unique_lock<std::mutex> lock(mtx);

            network net;
            auto url = networkBase::buildUrl(networkBase::Api::mainenance + std::string("?os=") + info::getOsNameS() + "&lang=" + info::language());
            int code = 0;
            decltype(net)::Args args{url.c_str(), nullptr, &code};
            auto temp = net.get(networkBase::Opt::getContent, args);

            if (code == 200) {
                res = temp | exec::move;
                break;
            }
            auto quitHint = std::function<void(bool)>([](bool) {
                nlog::Err(FI, LI, "%s : Retried multiple times but still unable to establish a connection. Exit", FN);
                QApplication::quit();
            });

            auto retryHint = std::function<void(bool)>([&condVar, &stop](bool check) {
                if (!check) {
                    stop = true;
                    condVar.notify_one();
                    QApplication::quit();
                } else {
                    condVar.notify_one();
                }
            });

            std::string msg = info::translations((i == 4) ? info::lang.error.networkConnectionRetryMax : info::lang.error.maintenanceInfoReq) + networkBase::errCodeReason(code) + "\n" + info::translations((i == 4) ? info::lang.error.clickToQuit : info::lang.error.clickToRetry);

            hintFunc({info::translations(info::lang.title.error), msg, "", (i == 4) ? 1 : 2, ((i == 4) ? quitHint : retryHint)});

            if (i == 4)
                return State::ActionNeeded;

            condVar.wait(lock);

            if (stop) {
                return State::ActionNeeded;
            }
        }

        nlog::Info(FI, LI, "%s : res : %s", FN, res.c_str());
        setLoadInfoFunc(0, info::translations(info::lang.loading.maintenanceInfoParse).c_str());

        auto jsonData = nlohmann::json::parse(res, nullptr, false);
        if (jsonData.is_discarded()) {
            nlog::Info(FI, LI, "%s : failed to maintenance parse!", FN);
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.maintenanceInfoParse), "", 1, [](bool) {
                          nlog::Err(FI, LI, "%s : click , quit programs", FN);
                          QApplication::quit();
                      }});
            return State::RetryRequired;
        }

        bool enable = jsonData["enable"].get<bool>();
        nlog::Info(FI, LI, "%s : maintenance enable : %s", FN, exec::boolTo<const char *>(enable));
        if (!enable)
            return State::Completed;

        std::string msg = jsonData["msg"].get<std::string>(),
                    poster = jsonData["poster"].get<std::string>(),
                    time = jsonData["time"].get<std::string>(),
                    link = jsonData["link"].get<std::string>();
        msg = time + "\n" + msg;

        setLoadInfoFunc(0, info::translations(info::lang.loading.downloadMaintenancePoster).c_str());
        auto fileName = downloadPoster(hintFunc, poster);

        ui::hintMsg hmsg{info::translations(info::lang.title.maintenance), msg, fileName, 1, [link](bool) {
                             QDesktopServices::openUrl(QUrl(link.c_str()));
                             QApplication::quit();
                         }};
        hintFunc(hmsg);
        return State::ActionNeeded;
    }
    // over : none update , undone : need update
    inline State checkUpdate(std::string &res) {
        nlog::autoLog log{FI, LI, FN};
        network net;
        auto url = net.buildUrl(networkBase::Api::checkUpdates);
        nlohmann::json dataJson = {
            {"core", info::getVersion()},
            {"res", info::getResVersion()},
            {"os", info::getOsName()},
            {"lang", info::language()}};
        auto data = dataJson.dump();
        auto id = std::string(FN) + "-" + exec::generateRandomString(6);
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.data = data.c_str();
        args.id = id.c_str();
        res = net.get(networkBase::Opt::postText, args);
        if (code == 204)
            return State::Completed;
        if (!res.empty() && code == 200) {
            return State::ActionNeeded;
        } else {
            nlog::Warn(FI, LI, "%s : code : %d , res : %s", FN, code, res.c_str());
            res.clear();
            return State::RetryRequired;
        }
    }
    // If any error occurs, return an empty object (an empty method is provided for checking).
    inline UpdateInfo parseUpdate(const std::string &res) {
        nlog::autoLog log{FI, LI, FN};

        nlog::Info(FI, LI, "%s : res : %s ", FN, res.c_str());
        auto jsonData = nlohmann::json::parse(res, nullptr, false);
        if (jsonData.is_discarded()) {
            nlog::Err(FI, LI, "%s : failed to update parse!", FN);
            return {};
        }
        UpdateInfo info{
            jsonData["title"].get<std::string>(),
            jsonData["msg"].get<std::string>(),
            jsonData["poster"].get<std::string>(),
            jsonData["time"].get<std::string>(),
            jsonData["resVersion"].get<std::string>()};

        info.mandatory = jsonData["mandatory"].get<bool>();

        for (const auto &it : jsonData["update"]) {
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
            nlog::Err(FI, LI, "%s : urls is empty!", FN);
            return {};
        }
        return info;
    }

    inline State autoUpdate(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc) {
        nlog::autoLog log{FI, LI, FN};
        std::string res;

        auto maintenanceState = checkMaintenance(hintFunc, loadFunc, setLoadInfoFunc);
        if (maintenanceState != State::Completed)
            return maintenanceState;

        setLoadInfoFunc(0, info::translations(info::lang.loading.checkUpdate).c_str());

        auto updateState = checkUpdate(res);
        if (updateState != State::ActionNeeded)
            return updateState;

        setLoadInfoFunc(0, info::translations(info::lang.loading.updateInfoParse).c_str());
        auto data = parseUpdate(res);
        if (data.empty())
            return State::ActionNeeded;

        setLoadInfoFunc(0, info::translations(info::lang.loading.downloadUpdatePoster).c_str());
        auto fileName = downloadPoster(hintFunc, data.poster);

        if (!data.mandatory) {
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            bool select = true;
            hintFunc({data.Title, (data.time + "\n" + data.msg), "", 2, [&condVar, &select](bool check) {
                          if (check) {
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

        ui::loadMsg lmsg{ui::loadMsg::All, info::translations(info::lang.loading.settingDownload), data.Title, data.time, data.msg, fileName, 100, 0, static_cast<int>(data.urls.size() * 2)};
        loadFunc(lmsg);

        std::vector<std::future<neko::State>> result;
        int progress = 0;
        bool stop = false;

        for (auto &it : data.urls) {
            if (it.randName)
                it.name = exec::generateRandomString(16);

            if (it.temp)
                it.name = info::temp() + it.name;

            if (!it.absoluteUrl)
                it.url = networkBase::buildUrl(it.url);
        }

        auto downloadTask = [=, &progress, &stop](int id, UpdateInfo::Url info) {
            network net;
            int code = 0;
            decltype(net)::Args args{
                info.url.c_str(),
                info.name.c_str(),
                &code};
            std::string ids = "update-" + std::to_string(id);
            args.id = ids.c_str();
            args.writeCallback = networkBase::WriteCallbackFile;
            if (stop)
                return State::ActionNeeded;

            if (info.multis) {
                if (!net.Multi(networkBase::Opt::downloadFile, {args}))
                    return State::RetryRequired;
            } else {
                if (!net.autoRetry(networkBase::Opt::downloadFile, {args}))
                    return State::RetryRequired;
            }
            ++progress;
            setLoadInfoFunc(progress, info::translations(info::lang.loading.downloadUpdate).c_str());
            return State::Completed;
        };

        auto checkHash = [=, &progress](const std::string &file, const std::string &exHash, const std::string hashAlgortihm) {
            auto hash = exec::hashFile(file, exec::mapAlgorithm(hashAlgortihm));
            if (hash == exHash) {
                nlog::Info(FI, LI, "%s : Everything is OK , file : %s  hash is matching", FN, file.c_str());
                ++progress;
                setLoadInfoFunc(progress, info::translations(info::lang.loading.downloadUpdate).c_str());
                return State::Completed;
            } else {
                nlog::Err(FI, LI, "%s : Hash Non-matching : file : %s  expect hash : %s , real hash : %s", FN, file.c_str(), exHash.c_str(), hash.c_str());
                return State::RetryRequired;
            }
        };

        // push task
        for (size_t i = 0; i < data.urls.size(); ++i) {
            result.push_back(exec::getThreadObj().enqueue([=, &stop] {
                if (stop)
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
                stop = true;
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.downloadUpdate), "", 2, [=](bool check) {
                              if (check) {
                                  exec::getThreadObj().enqueue([=] {
                                      autoUpdate(hintFunc, loadFunc, setLoadInfoFunc);
                                  });
                              } else {
                                  QApplication::quit();
                              }
                          }});
                return State::ActionNeeded;
            }
        }

        nlog::Info(FI, LI, "%s : update is ok", FN);

        bool needExecUpdate = false;
        std::string cmd = info::workPath() + "/update " + info::workPath();

        for (const auto &it : data.urls) {
            if (it.temp) {
                needExecUpdate = true;
                cmd += (" " + it.name);
            }
        }
        if (!data.resVersion.empty()) {
            exec::getConfigObj().SetValue("more", "resourceVersion", data.resVersion.c_str());
        }

        if (needExecUpdate) {
            nlog::Info(FI, LI, "%s : need exec update", FN);
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);

            auto execUpdate = [=, &condVar](bool) {
                condVar.notify_all();
                QApplication::quit();
            };

            hintFunc({info::translations(info::lang.title.reStart), info::translations(info::lang.general.updateOverReStart), "", 1, execUpdate});
            auto resState = condVar.wait_for(lock, std::chrono::seconds(6));

            if (resState == std::cv_status::timeout) {
                QApplication::quit();
            }
            launcherProcess(cmd,[]{},[](int){});
        }

        return State::Completed;
    }

    inline void feedbackLog(const std::string &feedback) {
        nlog::autoLog log{FI, LI, FN};
        network net;
        auto url = net.buildUrl(networkBase::Api::feedback);
        nlohmann::json dataJson = {
            {"core", info::getVersion()},
            {"res", info::getResVersion()},
            {"os", info::getOsName()},
            {"lang", info::language()},
            {"time", exec::getTimestamp()},
            {"log", feedback}};
        auto data = dataJson.dump();
        auto id = std::string(FN) + "-" + exec::generateRandomString(6);
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.data = data.c_str();
        args.id = id.c_str();
        net.Do(networkBase::Opt::postText, args);
        if (code != 204) {
            throw nerr::Error((code == 429) ? "Too Many Request , try again later" : "Failed to feedback log , code : " + std::to_string(code), FI, LI, FN);
        }
    }


} // namespace neko
