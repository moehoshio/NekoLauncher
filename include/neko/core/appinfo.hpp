#pragma once

#include "neko/schema/clientconfig.hpp"
#include "neko/schema/nekodefine.hpp"
#include "neko/schema/types.hpp"

#include "neko/bus/configBus.hpp"

#include <string>

/**
 * @namespace neko::core::app
 * @brief Application-specific information and utilities.
 */
namespace neko::core::app {

    /**
     * @brief Gets the application name.
     * @return The application name string.
     */
    constexpr neko::cstr getAppName() {
        return schema::definitions::AppName.data();
    }

    /**
     * @brief Gets the application version.
     * @return The application version string.
     */
    constexpr neko::cstr getVersion() {
        return schema::definitions::NekoLcCoreVersion.data();
    }

    /**
     * @brief Gets the resource version from configuration.
     * @return The resource version string.
     *
     * @details Uses core::getConfigObj from resources.hpp to access configuration.
     */
    inline std::string getResourceVersion() {
        neko::ClientConfig cfg = bus::config::getClientConfig();
        return cfg.other.resourceVersion;
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
        return schema::definitions::clientConfigFileName.data();
    }

    /**
     * @brief Gets the build ID.
     * @return The build ID string. e.g "v0.0.1-20250710184724-githash"
     */
    constexpr neko::cstr getBuildId() {
        return schema::definitions::buildID.data();
    }

    /**
     * @brief Gets the static remote configuration URL.
     * @return The static remote configuration URL string.
     */
    constexpr neko::cstr getStaticRemoteConfigUrl() {
        return schema::definitions::NetworkRemoteConfigUrl.data();
    }

} // namespace neko::core::app
