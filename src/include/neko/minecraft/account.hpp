#pragma once
#include "neko/log/nlog.hpp"

#include "neko/core/core.hpp"

#include "neko/schema/wmsg.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

#include "neko/network/network.hpp"

#include "library/nlohmann/json.hpp"

#include <string>
#include <string_view>
#include <functional>

namespace neko
{
    inline bool launcherMinecraftTokenValidate(std::function<void(const ui::hintMsg &)> hintFunc) {
        nlog::autoLog log{FI, LI, FN};
        network net;
        auto url = networkBase::buildUrl(neko::networkBase::Api::Authlib::validate, neko::networkBase::Api::Authlib::host);

        nlohmann::json json = {{"accessToken", exec::getConfigObj().GetValue("minecraft", "accessToken", "")}};
        auto data = json.dump();
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.data = data.c_str();
        args.header = "Content-Type: application/json";
        net.Do(networkBase::Opt::postText, args);
        if (code != 204) {
            nlog::Info(FI, LI, "%s : token is not validate", FN);
            auto refUrl = networkBase::buildUrl(neko::networkBase::Api::Authlib::refresh, neko::networkBase::Api::Authlib::host);
            int refCode = 0;
            nlohmann::json refJson = {
                {"accessToken", exec::getConfigObj().GetValue("minecraft", "accessToken", "")}, {"requestUser", false}};
            auto refData = refJson.dump();
            decltype(net)::Args refArgs{refUrl.c_str(), nullptr, &refCode};
            refArgs.data = refData.c_str();
            refArgs.header = "Content-Type: application/json";
            auto res = net.get(networkBase::Opt::postText, refArgs);
            auto jsonData = nlohmann::json::parse(res, nullptr, false);
            if (jsonData.is_discarded()) {
                hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.tokenJsonParse), "", 1});
                nlog::Err(FI, LI, "%s : faild to token json parse", FN);
                return false;
            }
            auto error = jsonData.value("error", ""),
                 errorMsg = jsonData.value("errorMessage", "");
            if (!error.empty() || !errorMsg.empty()) {
                hintFunc({error, errorMsg, "", 1});
                return false;
            }

            auto accessToken = jsonData["accessToken"].get<std::string>();
            std::string uuid;
            std::string name;
            if (!jsonData["selectedProfile"].empty()) {
                uuid = jsonData["selectedProfile"].value("id", "");
                name = jsonData["selectedProfile"].value("name", "");

                exec::getConfigObj().SetValue("minecraft", "uuid", uuid.c_str());
                exec::getConfigObj().SetValue("minecraft", "displayName", name.c_str());
            }
            exec::getConfigObj().SetValue("minecraft", "accessToken", accessToken.c_str());
        }
        return true;
    }

    inline void launcherMinecraftAuthlibAndPrefetchedCheck(std::function<void(const ui::hintMsg &)> hintFunc) {
        nlog::autoLog log{FI, LI, FN};
        std::string authlibPrefetched = exec::getConfigObj().GetValue("minecraft", "authlibPrefetched", "");
        if (!authlibPrefetched.empty())
            return;

        auto url = networkBase::buildUrl(neko::networkBase::Api::Authlib::root, neko::networkBase::Api::Authlib::host);
        network net;
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        auto res = net.autoRetryGet(networkBase::Opt::getContent, {args});

        if (res.empty()) {
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.minecraftAuthlibConnection), "", 1});
            nlog::Err(FI, LI, "%s : faild to connection authlib server!", FN);
            return;
        }

        auto resJson = nlohmann::json::parse(res, nullptr, false);
        if (resJson.is_discarded()) {
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.apiMetaParse), "", 1});
            nlog::Err(FI, LI, "%s : faild to api mete data parse", FN);
            return;
        }
        authlibPrefetched = exec::base64Encode(res);
        exec::getConfigObj().SetValue("minecraft", "authlibPrefetched", authlibPrefetched.c_str());
    }

    
    inline State authLogin(const std::vector<std::string> &inData, std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const std::string &)> callBack) {
        nlog::autoLog log{FI, LI, FN};
        if (inData.size() < 2)
            return State::ActionNeeded;

        nlohmann::json json = {
            {"username", inData[0]},
            {"password", inData[1]},
            {"requestUser", false},
            {"agent", {{"name", "Minecraft"}, {"version", 1}}}};

        auto data = json.dump();
        auto url = neko::networkBase::buildUrl(neko::networkBase::Api::Authlib::authenticate, neko::networkBase::Api::Authlib::host);
        neko::network net;
        int code = 0;
        decltype(net)::Args args{url.c_str(), nullptr, &code};
        args.header = "Content-Type: application/json";
        args.data = data.c_str();
        auto res = net.get(neko::networkBase::Opt::postText, args);
        auto resData = nlohmann::json::parse(res, nullptr, false);

        if (resData.is_discarded()) {
            hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.jsonParse), "", 1});
            return State::ActionNeeded;
        }

        auto error = resData.value("error", ""),
             errorMsg = resData.value("errorMessage", "");
        if (!error.empty() || !errorMsg.empty()) {
            hintFunc({error, errorMsg, "", 1});
            return State::RetryRequired;
        }

        auto accessToken = resData.value("accessToken", "");
        auto uuid = resData["selectedProfile"].value("id", "");
        auto name = resData["selectedProfile"].value("name", "");

        exec::getConfigObj().SetValue("minecraft", "accessToken", accessToken.c_str());
        exec::getConfigObj().SetValue("minecraft", "uuid", uuid.c_str());
        exec::getConfigObj().SetValue("minecraft", "account", inData[0].c_str());
        exec::getConfigObj().SetValue("minecraft", "displayName", name.c_str());

        callBack(name);

        return State::Completed;
    }
    
} // namespace neko
