#pragma once

// Neko Module
#include <neko/schema/exception.hpp>
#include <neko/schema/types.hpp>

#include "neko/app/api.hpp"
#include "neko/core/remoteConfig.hpp"

#include <string>

namespace neko::core {

    struct MaintenanceInfo {
        bool isMaintenance = false;
        std::string message;
        std::string posterPath;
        std::string openLinkCmd; // command to open the link, e.g., "open https://example.com"
    };

    /**
     * @brief Checks if the application is under maintenance.
     * @return MaintenanceInfo containing maintenance status and details
     * @throws ex::NetworkError if the network request fails
     * @throws ex::Parse if the response cannot be parsed
     * @throws ex::OutOfRange if a required key is missing in the response
     * @note This function publishes events to update the UI about the maintenance check process.
     */
    MaintenanceInfo checkMaintenance(api::LauncherConfigResponse config = getRemoteLauncherConfig());
} // namespace neko::core
