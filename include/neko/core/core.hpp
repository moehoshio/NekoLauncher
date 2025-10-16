#pragma once

// Neko Module

#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>

#include <neko/core/threadPool.hpp>

#include <neko/log/nlog.hpp>
#include <neko/network/network.hpp>

#include "neko/schema/api.hpp"
#include "neko/app/nekoLc.hpp"

// other libraries

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

namespace neko::core {

    inline void feedbackLog(const std::string &feedbackLog) {
        log::autoLog log;
        network::Network net;
        auto url = network::buildUrl(lc::api::feedbackLog);
        nlohmann::json dataJson = {
            {"feedbacklog", {{"coreVersion", info::app::getVersion()}, {"resourceVersion", info::app::getResourceVersion()}, {"os", info::app::getOsName()}, {"language", info::lang::language()}, {"timestamp", util::time::timeToString()}, {"content", feedbackLog}}}};

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .data = dataJson.dump(),
            .requestId = "feedbackLog-" + util::random::generateRandomString(6),
            .header = network::NetworkBase::HeaderGlobal::jsonContentHeader
        };
        auto res = net.execute(reqConfig);

        if (!res.isSuccess()) {
            throw ex::NetworkError(
                (res.statusCode() == 429) ? "Too Many Request , try again later" : ("Failed to feedbackLog log , code : " + std::to_string(res.statusCode())), ex::ExceptionExtensionInfo{});
        }
    }

} // namespace neko::core
