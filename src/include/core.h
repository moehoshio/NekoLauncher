#pragma once
#include "exec.h"
#include "info.h"
#include "mainwindow.h"
#include "msgtypes.h"
#include "network.h"

#include "nlohmann/json.hpp"

// openurl
#include <QtGui/QDesktopServices>

#include <condition_variable>
#include <mutex>
namespace neko {

    enum class State {
        over,
        undone,
        tryAgainLater,
    };

    struct updateInfo {
        std::string title,
            msg,
            poster,
            time;
        std::vector<std::string> urls,
            names,
            hashs;
        std::vector<int> multis;

        bool empty() {
            std::vector<bool> res{
                title.empty(), msg.empty(), poster.empty(), time.empty(), urls.empty(), names.empty(), hashs.empty(), multis.empty()};
            for (auto it : res) {
                if (!it) {
                    return false;
                }
            }
            return true;
        }
    };
    //Return a null T value if the download fails.
    template <typename T = std::string>
    T downloadPoster(std::function<void(const ui::hintMsg &)> hintFunc, const std::string &url) {
        if (!url.empty()) {
            network net;
            auto fileName = info::getTemp() + "update_" + exec::generateRandomString(10) + ".png";
            int code = 0;

            network<std::string>::Args args{url.c_str(), fileName.c_str(), &code};
            args.writeCallback = networkBase::WriteCallbackFile;
            net.Do(networkBase::Opt::downloadFile, args);
            if (code != 200) {
                nlog::Warn(FI, LI, "%s : failed to poster download", FN);
                hintFunc({"Error", "failed to poster download!", "", 1, [](bool) {}});
                return T();
            }
            return fileName.c_str();
        }
        return T();
    }

    // over : not maintenance, undone : in maintenance
    State checkMaintenance(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc) {
        nlog::autoLog log{FI, LI, FN};
        loadFunc({ui::loadMsg::Type::OnlyRaw, "maintenance info req.."});

        std::string res;
        std::mutex mtx;
        std::condition_variable condVar;
        bool stop = false;

        for (size_t i = 0; i < 5; ++i) {
            nlog::autoLog log{FI, LI, "Get maintenance req - " + std::to_string(i)};
            std::unique_lock<std::mutex> lock(mtx);

            network net;
            auto url = networkBase::buildUrl<std::string>(networkBase::Api::mainenance);
            int code = 0;
            decltype(net)::Args args{url.c_str(), nullptr, &code};
            auto temp = net.get(networkBase::Opt::getContent, args);

            if (code == 200) {
                res = temp | exec::move;
                break;
            }
            if (i == 4) {
                hintFunc({"Error", "Retried multiple times but still unable to establish a connection.", "", 1, [](bool) {
                              nlog::Err(FI, LI, "%s : Retried multiple times but still unable to establish a connection. Exit", FN);
                              QApplication::quit();
                          }});
                return State::undone;
            } else {
                hintFunc({"Error", "An issue occurred while retrieving maintenance information: " + networkBase::errCodeReason(code), "", 2, [=, &condVar, &stop](bool check) {
                              if (!check) {
                                  stop = true;
                                  condVar.notify_one();
                                  QApplication::quit();
                              } else {
                                  condVar.notify_one();
                              }
                          }});
            }

            condVar.wait(lock);
            if (stop) {
                return State::undone;
            }
        }

        nlog::Info(FI, LI, "%s : res : %s", FN, res.c_str());
        setLoadInfoFunc(0, "maintenance info paese...");

        auto jsonData = nlohmann::json::parse(res, nullptr, false);
        if (jsonData.is_discarded()) {
            nlog::Info(FI, LI, "%s : failed to maintenance parse!", FN);
            hintFunc({"Error", "failed to maintenance parse!", "", 1, [](bool) {
                          nlog::Err(FI, LI, "%s : click , quit programs", FN);
                          QApplication::quit();
                      }});
            return State::tryAgainLater;
        }

        bool enable = jsonData["enable"].get<bool>();
        nlog::Info(FI, LI, "%s : maintenance enable : %s", FN, exec::boolTo<const char *>(enable));
        if (!enable)
            return State::over;

        std::string msg = jsonData["msg"].get<std::string>(),
                    poster = jsonData["poster"].get<std::string>(),
                    time = jsonData["time"].get<std::string>(),
                    annctLink = jsonData["annctLink"].get<std::string>();
        msg = time + "\n" + msg;

        setLoadInfoFunc(0, "download maintained poster...");
        auto fileName = downloadPoster(hintFunc,poster);

        ui::hintMsg hmsg{"Being maintained", msg, fileName, 1, [annctLink](bool) {
                             QDesktopServices::openUrl(QUrl(annctLink.c_str()));
                             QApplication::quit();
                         }};
        hintFunc(hmsg);
        return State::undone;
    }
    // over : none update , undone : update
    State checkUpdate(std::string &res) {
        nlog::autoLog log{FI, LI, FN};
        network net;
        auto url = net.buildUrl<std::string>(networkBase::Api::checkUpdates);
        auto data = std::string("{\n") + "\"core\":\"" + info::getVersion() + "\",\n\"res\":\"" + info::getResVersion() + "\"\n}";
        auto id = std::string(FN) + "-" + exec::generateRandomString(6);
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.data = data.c_str();
        args.id = id.c_str();
        res = net.get(networkBase::Opt::postText, args);
        if (code == 204)
            return State::over;
        if (!res.empty() && code == 200) {
            return State::undone;
        } else {
            nlog::Warn(FI, LI, "%s : code : %d , res : %s", FN, code, res.c_str());
            res.clear();
            return State::tryAgainLater;
        }
    }
    // If any error occurs, return an empty object (an empty method is provided for checking).
    updateInfo parseUpdate(const std::string &res) {
        nlog::autoLog log{FI, LI, FN};
        nlog::Info(FI, LI, "%s : res : %s ", FN, res.c_str());
        auto jsonData = nlohmann::json::parse(res, nullptr, false);
        if (jsonData.is_discarded()) {
            nlog::Err(FI, LI, "%s : failed to update parse!", FN);
            return {};
        }
        updateInfo info{
            jsonData["title"].get<std::string>(),
            jsonData["msg"].get<std::string>(),
            jsonData["poster"].get<std::string>(),
            jsonData["time"].get<std::string>(),
            jsonData["url"]["urls"].get<std::vector<std::string>>(),
            jsonData["url"]["names"].get<std::vector<std::string>>(),
            jsonData["url"]["hashs"].get<std::vector<std::string>>(),
            jsonData["url"]["multis"].get<std::vector<int>>()};

        if (info.urls.size() == 0) {
            nlog::Err(FI, LI, "%s : urls size is 0 !", FN);
            return {};
        }

        if (exec::anyTrue(info.urls.size() != info.names.size(), info.names.size() != info.hashs.size(), info.hashs.size() != info.multis.size())) {
            nlog::Err(FI, LI, "%s : Resources Unexpected : urls size : %zu , names size : %zu , hashs size : %zu ", FN, info.urls.size(), info.names.size(), info.hashs.size(), info.multis.size());
            return {};
        } else {
            nlog::Info(FI, LI, "%s : vector size not 0 and match", FN);
        }
        return info;
    }

    State autoUpdate(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc) {
        nlog::autoLog log{FI, LI, FN};
        std::string res;

        auto maintenanceState = checkMaintenance(hintFunc, loadFunc, setLoadInfoFunc);
        if (maintenanceState != State::over)
            return maintenanceState;

        setLoadInfoFunc(0, "check update..");

        auto updateState = checkUpdate(res);
        if (updateState != State::undone)
            return updateState;

        setLoadInfoFunc(0, "update info parse...");
        auto data = parseUpdate(res);
        if (data.empty())
            return State::undone;

        setLoadInfoFunc(0, "download update poster...");
        auto fileName = downloadPoster(hintFunc, data.poster);

        ui::loadMsg lmsg{ui::loadMsg::All, "update download...", data.title, data.time, data.msg, fileName, 100, 0, static_cast<int>(data.urls.size() * 2)};
        loadFunc(lmsg);

        std::vector<std::future<neko::State>> result;
        int progress = 0;
        bool stop = false;

        // push task
        for (size_t i = 0; i < data.urls.size(); ++i) {
            result.push_back(exec::getThreadObj().enqueue([=, &progress, &stop] {
                network net;
                int code = 0;
                decltype(net)::Args args{
                    data.urls[i].c_str(),
                    data.names[i].c_str(),
                    &code};
                std::string id = std::to_string(i);
                args.id = id.c_str();
                args.writeCallback = networkBase::WriteCallbackFile;
                if (stop)
                    return State::undone;
                if (data.multis[i]) {
                    if (!net.Multi(networkBase::Opt::downloadFile, {args}))
                        return State::tryAgainLater;
                } else {
                    if (!net.autoRetry(networkBase::Opt::downloadFile, {args}))
                        return State::tryAgainLater;
                }
                ++progress;
                setLoadInfoFunc(progress, "verify file...");

                auto hash = exec::hashFile(data.names[i]);

                if (stop)
                    return State::undone;
                if (hash != data.hashs[i]) {
                    nlog::Err(FI, LI, "%s : Hash Non-matching :  expect hash : %s , real hash : %s", FN, data.hashs[i].c_str(), hash.c_str());
                    return State::tryAgainLater;
                } else {
                    nlog::Info(FI, LI, "%s : Everything is OK , hash is matching", FN);
                    ++progress;
                    setLoadInfoFunc(progress, "update download..");
                    return State::over;
                }
            }));
        }
        // check result
        for (auto &it : result) {

            if (it.get() != State::over) {
                stop = true;
                hintFunc({"Error", "failed to update download !", "", 1, [](bool check) {
                              nlog::Err(FI, LI, "%s : click , quit programs", FN);
                              QApplication::quit();
                          }});
                return State::undone;
            }
        }

        return State::over;
    }

} // namespace neko
