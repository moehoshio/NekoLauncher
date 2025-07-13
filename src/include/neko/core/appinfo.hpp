#pragma once


#include "neko/schema/clientconfig.hpp"
#include "neko/schema/nekodefine.hpp"
#include "neko/schema/types.hpp"

#include "neko/core/resources.hpp"

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
        ClientConfig cfg(core::getConfigObj());
        return cfg.other.resourceVersion;
    }

    /**
     * @brief Gets the device ID from configuration.
     * @return The device ID string.
     */
    inline std::string getDeviceId() {
        ClientConfig cfg(core::getConfigObj());
        return cfg.main.deviceID;
    }

    /**
     * @brief Gets the configuration file name.
     * @return The configuration file name.
     */
    constexpr neko::cstr getConfigFileName() {
        return schema::definitions::clientConfigFileName.data();
    }

    /**
     * @brief Gets the build ID.
     * @return The build ID string.
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
