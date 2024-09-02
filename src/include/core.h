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
        auto fileName = info::getTemp() + "/maintained_" + exec::generateRandomString(12) + ".png";
        
        //download poster
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
        checkMaintenance(hintFunc);
        return State::over;
    }

} // namespace neko

// namespace neko {
//     class core {
//     public:

//         enum class State {
//             over,
//             undone,
//             tryAgainLater,

//         };
//         // over : not maintenance
//     inline State checkMaintenance(ui::MainWindow *w){

//         nlog::Info(FI,LI,"%s : Enter , w ptr : %p",FN,w);
//         network net;
//         auto url = networkBase::buildUrl<std::string>(networkBase::Api::mainenance );
//         int code = 0;
//         decltype(net)::Args args{ url.c_str(),nullptr,&code};
//         auto res = net.get(networkBase::Opt::getContent,args);

//         auto jsonData = nlohmann::json::parse(res,nullptr,false);

//         bool enable = jsonData["enable"];
//         nlog::Info(FI,LI,"%s : this req code %d , maintenance enable : %s , res : %s ",FN,code,exec::boolTo<const char *>(enable),res.c_str());
//         if (!enable) return State::over;

//         std::string msg = jsonData["msg"].get<std::string>(),
//         poster = jsonData["poster"].get<std::string>(),
//         time = jsonData["time"].get<std::string>(),
//         annctLink = jsonData["annctLink"].get<std::string>();

//         ui::maintenanceMsg m{msg,poster,time ,annctLink};
//         w->onMaintenancePage(m);

//         nlog::Info(FI,LI,"%s : Exit",FN);
//         return State::undone;
//     }
//     inline State checkUpdates(ui::MainWindow * w){
//         network net;
//         auto url = networkBase::buildUrl<std::string>(networkBase::Api::checkUpdates);
//         int code = 0;
//         decltype(net)::Args args{ url.c_str() , nullptr , &code };
//         auto res = net.get(networkBase::Opt::getContent,args);

//         switch (code)
//         {
//         case 200:
//             break;
//         case 204:
//             return State::over;
//             break;
//         case 400:
//         default:
//             return State::undone;
//             break;
//         }

//         auto jsonData = nlohmann::json::parse(res,nullptr,false);

//         std::string
//         title = jsonData["title"],
//         msg = jsonData["msg"],
//         poster = jsonData["poster"],
//         time = jsonData["time"];
//         std::vector<std::string> urls;
//         for ( const auto & it : jsonData["url"])
//         {
//             urls.push_back(it);
//         }
//         // ui::updateMsg m{title,msg,time};
//         if (exec::isUrl(poster)){
//             args.url = poster.c_str();
//             std::string fileName = info::getTemp() + exec::generateRandomString(16)+ ".png";
//             net.Do(networkBase::Opt::downloadFile,args);
//             m.poster = fileName;
//         }
//         exec::getThreadObj().enqueue(
//             [](){}
//         );

//         // w->onUpdatePage();

//     }
//     inline State autoUpdate(ui::MainWindow *w){
//         nlog::Info(FI,LI,"%s : Enter , w ptr: %p ",FN,w);
//         checkMaintenance(w);
//         // network net;
//         return State::over;

//     }
//     };
// } // namespace neko
