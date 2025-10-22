/**
 * @see neko/core/configManager.hpp
 * @file configBus.hpp
 * @brief Provides a bus for managing configuration settings.
 */

#pragma once

#include "neko/app/configManager.hpp"
#include "neko/bus/resources.hpp"

namespace neko::bus::config {

    inline bool load(const std::string &filename) {
        return bus::getConfigObj().load(filename);
    }

    inline bool save(const std::string &filename) {
        return bus::getConfigObj().save(filename);
    }

    inline void updateClientConfig(std::function<void(neko::ClientConfig &)> updaterFunc) {
        bus::getConfigObj().updateClientConfig(std::move(updaterFunc));
    }

    inline neko::ClientConfig getClientConfig() {
        return bus::getConfigObj().getClientConfig();
    }

} // namespace neko::bus::config