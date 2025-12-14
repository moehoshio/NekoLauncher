#pragma once

// Neko Module
#include <neko/log/nlog.hpp>

#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>

#include <neko/network/network.hpp>
#include <neko/function/utilities.hpp>
#include <neko/system/platform.hpp>

// NekoLc
#include "neko/app/api.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/nekoLc.hpp"

// other libraries
#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

namespace neko::core {

    /**
     * @brief Send feedback log to neko server
     * @param feedbackLog The feedback log content
     * @throws ex::NetworkError if the network request fails
     */
    inline void feedbackLog(const std::string &feedbackLog) {
        log::autoLog log;
        network::Network net;
        auto url = network::buildUrl(lc::api::feedbackLog);

        auto json = app::getRequestJson("feedbackLogRequest");
        json["content"] = feedbackLog;

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .requestId = "feedbackLog-" + util::random::generateRandomString(6),
            .header = network::header::jsonContentHeader,
            .postData = json.dump()};
        auto res = net.execute(reqConfig);

        if (!res.isSuccess()) {
            throw ex::NetworkError(
                (res.statusCode == 429) ? "Too Many Request , try again later" : ("Failed to feedbackLog log , code : " + std::to_string(res.statusCode)));
        }
    }

} // namespace neko::core
