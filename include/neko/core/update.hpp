#pragma once

#include "neko/app/api.hpp"
#include <neko/schema/exception.hpp>

#include "neko/core/remoteConfig.hpp"

#include <optional>
#include <string>

namespace neko::core::update {

    /**
     * @brief Check for updates from the update server.
     * @return Optional JSON payload containing update info when available. nullopt if no update is available.
     * @throws ex::NetworkError if the network request fails.
     * @throws ex::Exception for unexpected responses.
     */
    std::optional<std::string> checkUpdate(api::LauncherConfigResponse config = getRemoteLauncherConfig());

    /**
     * @brief Parse the update response from JSON string.
     * @param result The JSON string containing the update response.
    * @throws ex::ParseError if JSON parsing fails.
    * @throws ex::RangeError if expected keys are missing in the JSON.
     * @throws ex::Exception for other exceptions during parsing.
     * @return The parsed UpdateResponse object.
     */
    api::UpdateResponse parseUpdate(const std::string &result);

    /**
     * @brief Perform the update process with the given update data.
     * @param data The update response data.
     * @throws ex::NetworkError if any network operation fails.
     * @throws ex::FileError if file operations fail.
    * @throws ex::ArgumentError if the update data is invalid.
     * @throws ex::Exception for other errors during the update process.
     */
    void update(api::UpdateResponse data);

    /**
     * @brief Perform the auto-update process.
     * @note This function publishes events to the event bus and may quit the application if maintenance mode is active.
     */
    void autoUpdate();

} // namespace neko::core::update