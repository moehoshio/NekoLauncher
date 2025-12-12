#pragma once

#include "neko/app/api.hpp"

namespace neko::core {

    /**
     * @brief Fetches the remote launcher configuration from the launcher config API.
     * @throws ex::NetworkError if the network request fails
     * @throws ex::Parse if the response cannot be parsed
     */
    api::LauncherConfigResponse getRemoteLauncherConfig();
} // namespace neko::core
