#pragma once

#include <neko/log/nlog.hpp>
#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>

#include <neko/function/utilities.hpp>
#include <neko/system/platform.hpp>

#include <neko/network/network.hpp>

// NekoLc project
#include "neko/app/nekoLc.hpp"
#include "neko/schema/api.hpp"

namespace neko::core {

    namespace internal {

        /**
         * @throws ex::Parse if the response cannot be parsed
         * @throws ex::NetworkError if the network request fails
         */
        inline schema::LauncherConfigResponse getStaticRemoteConfig() {
            log::autoLog log;
            network::Network net;
            network::RequestConfig reqConfig{
                .url = app::getStaticRemoteConfigUrl(),
                .method = network::RequestType::Get,
                .requestId = "launcher-config-" + util::random::generateRandomString(6)
            };
            auto result = net.executeWithRetry({reqConfig});
            if (!result.isSuccess() || result.content.empty()) {
                log::error({}, "Failed to get remote launcher config: {}", result.errorMessage);
                log::debug({}, "Detailed error: {}", result.detailedErrorMessage);
                throw ex::NetworkError("Failed to get remote launcher config : " + result.errorMessage);
            }
            schema::LauncherConfigResponse response;
            try {
                nlohmann::json config = nlohmann::json::parse(result.content);
                response = config.get<schema::LauncherConfigResponse>();
            } catch (const nlohmann::json::exception &e) {
                log::error({}, "Failed to parse remote launcher config: %s", e.what());
                throw ex::Parse("Failed to parse remote launcher config: " + std::string(e.what()));
            }
            return response;
        }

        /**
         * @throws ex::Parse if the response cannot be parsed
         * @throws ex::NetworkError if the network request fails
         */
        inline schema::LauncherConfigResponse getDynamicRemoteConfig() {
            log::autoLog log;
            network::Network net;
            network::RequestConfig reqConfig{
                .url = network::buildUrl(lc::api::launcherConfig),
                .method = network::RequestType::Post,
                .data = info::getRequestJson("launcherConfigRequest").dump(),
                .requestId = "launcher-config-" + util::random::generateRandomString(6),
                .header = network::NetworkBase::HeaderGlobal::jsonContentHeader
            };

            auto result = net.executeWithRetry({reqConfig});

            if (!result.isSuccess()) {
                log::error({}, "Failed to get remote launcher config: {}", result.errorMessage);
                log::debug({}, "Detailed error: {}", result.detailedErrorMessage);
                throw ex::NetworkError("Failed to get remote launcher config: " + result.errorMessage);
            }

            try {
                // Parse the response content as JSON
                nlohmann::json config = nlohmann::json::parse(result.content);
                return config.get<schema::LauncherConfigResponse>();
            } catch (const nlohmann::json::exception &e) {
                log::error({}, "Failed to parse remote launcher config: {}", e.what());
                throw ex::Parse("Failed to parse remote launcher config: " + std::string(e.what()));
            }
        }
    } // namespace internal

    /**
     * @brief Fetches the remote launcher configuration from the launcher config API.
     * @throws ex::NetworkError if the network request fails
     * @throws ex::Parse if the response cannot be parsed
     */
    inline schema::LauncherConfigResponse getRemoteLauncherConfig() {
        log::autoLog log;
        // Use static remote config if enabled
        if constexpr (lc::EnableStaticDeployment || lc::EnableStaticRemoteConfig) {
            return internal::getStaticRemoteConfig();
        } else {
            // If static remote config is not enabled, use the launcher config API
            return internal::getDynamicRemoteConfig();
        }
    }
} // namespace neko::core
