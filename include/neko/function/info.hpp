#pragma once

#include "neko/schema/api.hpp"
#include "neko/app/appinfo.hpp"

#include <neko/system/platform.hpp>
#include <neko/function/utilities.hpp>

#include <nlohmann/json.hpp>

namespace neko::info {

    /**
     * @brief Gets the client information.
     * @return The client information struct.
     */
    inline schema::ClientInfo getClientInfo() {
        return schema::ClientInfo{
            .app = {
                .appName = getAppName(),
                .coreVersion = getVersion(),
                .resourceVersion = getResourceVersion(),
                .buildId = getBuildId()},
            .system = {.os = system::getOsName(), .arch = system::getArchName(), .osVersion = system::getOsVersion()},
            .extra = {},
            .deviceId = getDeviceId()};
    }

    inline schema::Preferences getPreferences() {
        return schema::Preferences{
            .language = lang::language()};
    }

    inline nlohmann::json getRequestJson(const std::string &requestName) {
        return nlohmann::json{
            {requestName, {
                {"clientInfo", getClientInfo()},
                {"timestamp", util::time::getUtcNow()}
            }},
            {"preferences", getPreferences()}
        };
    }
} // namespace neko::info