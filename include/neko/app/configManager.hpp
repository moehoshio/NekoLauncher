#pragma once

#include <SimpleIni.h>

#include "neko/app/clientConfig.hpp"

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>

namespace neko::app {

    class ConfigManager {
    private:
        mutable std::shared_mutex mutex;
        CSimpleIniA ini;

    public:
        bool load(const std::string &filename) {
            std::unique_lock lock(mutex);
            return ini.LoadFile(filename.c_str()) == SI_OK;
        }

        bool save(const std::string &filename) {
            std::shared_lock lock(mutex);
            return ini.SaveFile(filename.c_str()) == SI_OK;
        }

        /**
         * @brief Atomic update of the client configuration
         * @param updaterFunc Function to update the client configuration. After assignment is completed, the value will be written back to ini object
         */
        void updateClientConfig(std::function<void(neko::ClientConfig &)> updaterFunc) {
            std::unique_lock lock(mutex);
            neko::ClientConfig cfg(ini);
            updaterFunc(cfg);
            cfg.setToConfig(ini);
        }

        /**
         * @brief Get the current client configuration
         * @return Current client configuration
         * @note This function returns a copy of the configuration, which may not be consistent with the latest state
         */
        neko::ClientConfig getClientConfig() const {
            std::shared_lock lock(mutex);
            return neko::ClientConfig(ini);
        }
    };

} // namespace neko::app