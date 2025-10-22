#pragma once

#include <neko/schema/types.hpp>
#include <neko/system/platform.hpp>
#include <neko/system/memoryinfo.hpp>

#include "neko/app/nekoLc.hpp"

#include "neko/app/api.hpp" // for ClientInfo
#include "neko/app/lang.hpp" // for Preferences

#include "neko/app/clientConfig.hpp"

#include "neko/bus/configBus.hpp" // for getClientConfig

#include <string>

/**
 * @namespace neko::core::app
 * @brief Application-specific information and utilities.
 */
namespace neko::app {

    /**
     * @brief Gets the application name.
     * @return The application name string.
     */
    constexpr neko::cstr getAppName() {
        return lc::AppName.data();
    }

    /**
     * @brief Gets the application version.
     * @return The application version string.
     */
    constexpr neko::cstr getVersion() {
        return lc::AppVersion.data();
    }

    /**
     * @brief Gets the resource version from configuration.
     * @return The resource version string.
     *
     * @details Uses core::getConfigObj from resources.hpp to access configuration.
     */
    inline std::string getResourceVersion() {
        neko::ClientConfig cfg = bus::config::getClientConfig();
        return cfg.main.resourceVersion;
    }

    /**
     * @brief Gets the device ID from configuration.
     * @return The device ID string. e.g "123e4567-e89b-12d3-a456-426614174000"
     */
    inline std::string getDeviceId() {
        neko::ClientConfig cfg = bus::config::getClientConfig();
        return cfg.main.deviceID;
    }

    /**
     * @brief Gets the configuration file name.
     * @return The configuration file name. e.g "config.ini"
     */
    constexpr neko::cstr getConfigFileName() {
        return lc::ClientConfigFileName.data();
    }

    /**
     * @brief Gets the build ID.
     * @return The build ID string. e.g "v0.0.1-20250710184724-githash"
     */
    constexpr neko::cstr getBuildId() {
        return lc::buildID.data();
    }

    /**
     * @brief Gets the static remote configuration URL.
     * @return The static remote configuration URL string.
     */
    constexpr neko::cstr getStaticRemoteConfigUrl() {
        return lc::NetworkStaticRemoteConfigUrl.data();
    }

    /**
     * @brief Gets the client information.
     * @return The client information struct.
     */
    inline api::ClientInfo getClientInfo() {
        return api::ClientInfo{
            .app = {
                .appName = getAppName(),
                .coreVersion = getVersion(),
                .resourceVersion = getResourceVersion(),
                .buildId = getBuildId()},
            .system = {.os = system::getOsName(), .arch = system::getOsArch(), .osVersion = system::getOsVersion()},
            .extra = {},
            .deviceId = getDeviceId()};
    }

    inline api::Preferences getPreferences() {
        return api::Preferences{
            .language = lang::language()};
    }

    inline nlohmann::json getRequestJson(const std::string &requestName) {
        return nlohmann::json{
            {requestName, {{"clientInfo", getClientInfo()}, {"timestamp", util::time::getUtcNow()}}},
            {"preferences", getPreferences()}};
    }

} // namespace neko::app
