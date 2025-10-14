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
