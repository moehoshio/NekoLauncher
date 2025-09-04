/**
 * @see neko/core/configManager.hpp
 * @file configBus.hpp
 * @brief Provides a bus for managing configuration settings.
 */

#pragma once

#include "neko/core/configManager.hpp"
#include "neko/core/resources.hpp"

namespace neko::bus::config {

    inline bool load(const std::string &filename) {
        return core::getConfigObj().load(filename);
    }

    inline bool save(const std::string &filename) {
        return core::getConfigObj().save(filename);
    }

    inline void updateClientConfig(std::function<void(neko::ClientConfig &)> updaterFunc) {
        core::getConfigObj().updateClientConfig(std::move(updaterFunc));
    }

    inline neko::ClientConfig getClientConfig() {
        return core::getConfigObj().getClientConfig();
    }

} // namespace neko::bus::config