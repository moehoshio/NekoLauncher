#pragma once
#include "exec.h"
#include "info.h"
#include "mainwindow.h"
#include "msgtypes.h"
#include "network.h"

#include "nlohmann/json.hpp"

// openurl
#include <QtGui/QDesktopServices>
namespace neko {

    enum class State {
        over,
        undone,
        tryAgainLater,
    };

    State checkMaintenance(std::function<void(const ui::hintMsg &)> hintFunc) {
        nlog::autoLog log{FI, LI, FN};

        network net;
        auto url = networkBase::buildUrl<std::string>(networkBase::Api::mainenance);
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        auto res = net.get(networkBase::Opt::getContent, args);
        if (code != 200) {
            nlog::Err(FI, LI, "%s : this req undone(not http code 200) tryAgainLater , code : %d", FN, code);
            return State::tryAgainLater;
        }

        auto jsonData = nlohmann::json::parse(res, nullptr, false);
        bool enable = jsonData["enable"];
        nlog::Info(FI, LI, "%s : this req code %d , maintenance enable : %s , res : %s ", FN, code, exec::boolTo<const char *>(enable), res.c_str());
        if (!enable)
            return State::over;

        std::string msg = jsonData["msg"].get<std::string>(),
                    poster = jsonData["poster"].get<std::string>(),
                    time = jsonData["time"].get<std::string>(),
                    annctLink = jsonData["annctLink"].get<std::string>();
        msg = time + "\n" + msg;
        auto fileName = info::getTemp() + "maintained_" + exec::generateRandomString(12) + ".png";

        // download poster
        if (!poster.empty()) {

            decltype(net)::Args args2{poster.c_str(), fileName.c_str(), &code};
            args2.writeCallback = networkBase::WriteCallbackFile;
            net.Do(networkBase::Opt::downloadFile, args2);
        }

        auto callBack = [annctLink](auto &&) {
            QDesktopServices::openUrl(QUrl(annctLink.c_str()));
            QApplication::quit();
        };

        ui::hintMsg m{"Being maintained", msg, ((poster.empty()) ? "" : fileName), 1, callBack};
        hintFunc(m);
        return State::undone;
    }

    State autoUpdate(std::function<void(const ui::hintMsg &)> hintFunc) {
        nlog::autoLog log{FI, LI, FN};
        auto maintenanceState = checkMaintenance(hintFunc);
        if (maintenanceState != State::over)
            return maintenanceState;

        network net;
        auto url = net.buildUrl<std::string>(networkBase::Api::checkUpdates);

        auto data = std::string("{\n") + "\"core\":\"" + info::getVersion() + "\",\n\"res\":\"" + info::getResVersion() + "\"\n}";

        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.data = data.c_str();

        auto res = net.get(networkBase::Opt::postText, args);
        nlog::Info(FI, LI, "%s : res : %s ", FN, res.c_str());
        if (code == 204)
            return State::over;

        if (!res.empty() && code == 200) {
            nlog::autoLog log{FI, LI, " res not empty , code == 200"};
            auto jsonData = nlohmann::json::parse(res);
            std::string title = jsonData["title"].get<std::string>(),
                        msg = jsonData["msg"].get<std::string>(),
                        poster = jsonData["poster"].get<std::string>(),
                        time = jsonData["time"].get<std::string>();
            auto rawUrls = jsonData["url"]["urls"];
            auto rawNames = jsonData["url"]["names"];
            auto rawHashs = jsonData["url"]["hashs"];
            auto rawMultis = jsonData["url"]["multis"];
            std::vector<std::string> urls = rawUrls.get<std::vector<std::string>>();
            std::vector<std::string> names = rawNames.get<std::vector<std::string>>();
            std::vector<std::string> hashs = rawHashs.get<std::vector<std::string>>();
            std::vector<int> multis = rawMultis.get<std::vector<int>>();
            std::vector<std::future<neko::State>> result;

            if (urls.size() == 0) {
                nlog::Err(FI, LI, "%s : urls is 0 !", FN);
                return State::undone;
            }

            if (exec::anyTrue(urls.size() != names.size(), names.size() != hashs.size(), hashs.size() != multis.size())) {
                nlog::Err(FI, LI, "%s : Resources Unexpected : urls : %zu , names : %zu , hashs : %zu ", FN, urls.size(), names.size(), hashs.size());
                return State::undone;
            } else {
                nlog::Info(FI, LI, "%s : vector size not 0 and match", FN);
            }

            for (size_t i = 0; i < urls.size(); ++i) {
                nlog::autoLog log{FI, LI, "urls-" + std::to_string(i)};
                result.push_back(exec::getThreadObj().enqueue([=] {
                    nlog::autoLog log{FI, LI, "dg-urls-" + std::to_string(i)};
                    network net;
                    int code = 0;
                    decltype(net)::Args args{
                        urls[i].c_str(),
                        names[i].c_str(),
                        &code};
                    std::string id = std::to_string(i);
                    args.id = id.c_str();
                    args.writeCallback = networkBase::WriteCallbackFile;
                    if (multis[i]) {
                        if (!net.Multi(networkBase::Opt::downloadFile, {args}))
                            return State::tryAgainLater;
                    } else {
                        if (!net.autoRetry(networkBase::Opt::downloadFile, {args}))
                            return State::tryAgainLater;
                    }

                    auto hash = exec::hashFile(names[i]);

                    if (hash != hashs[i]) {
                        nlog::Err(FI, LI, "%s : Hash Non-matching :  expect hash : %s , real hash : %s", FN, hashs[i].c_str(), hash.c_str());
                        return State::tryAgainLater;
                    } else {
                        nlog::Info(FI, LI, "%s : Everything is OK , hash is matching");
                        return State::over;
                    }
                }));
            }
            for (auto &it : result) {
                if (it.get() != State::over)
                    return State::undone;
            }
        } else {
            nlog::Warn(FI, LI, "%s : code : %d , res : %s", FN, code, res.c_str());
            return State::tryAgainLater;
        }

        return State::over;
    }

} // namespace neko
