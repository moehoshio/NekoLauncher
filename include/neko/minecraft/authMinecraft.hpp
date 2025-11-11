/**
 * @file authMinecraft.hpp
 * @brief A Minecraft authentication module
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

// Neko Module

#include <neko/log/nlog.hpp>
#include <neko/function/utilities.hpp>
#include <neko/network/network.hpp>
#include <neko/schema/exception.hpp>

// NekoLc project
#include "neko/app/appinfo.hpp"
#include "neko/app/nekoLc.hpp"
#include "neko/app/lang.hpp"

#include "neko/bus/resources.hpp"
#include "neko/bus/configBus.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

namespace neko::minecraft::auth {

    enum class AuthMode {
        // Microsoft, // To Be implemented
        AuthlibInjector,
        Offline
    };

    /**
     * @brief This function checks if the Minecraft authlib prefetch data is already stored in the client configuration. If it is not, it attempts to fetch the data from the network.
     * @throws ex::NetworkError if the token refresh fails or the network request encounters an error.
     * @throws ex::Parse if the response from the server cannot be parsed.
     */
    inline void authMinecraftAuthlibAndPrefetchedCheck() {
        log::autoLog log;
        auto clientConfig = bus::config::getClientConfig();
        std::string authlibPrefetched = clientConfig.minecraft.authlibPrefetched;
        if (!authlibPrefetched.empty())
            return;

        auto url = network::buildUrl(lc::api::authlib::root, lc::api::authlib::host);
        network::Network net;
        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Get,
        };
        auto result = net.execute(reqConfig);
        if (!result.isSuccess() || !result.hasContent()) {
            log::error("Failed to fetch authlib data: {}", {} ,result.errorMessage);
            throw ex::NetworkError("Failed to fetch authlib data");
        }

        nlohmann::json resJson;
        try {
            resJson = nlohmann::json::parse(result.content);
        } catch (const nlohmann::json::parse_error &e) {
            log::error("Failed to parse authlib response: {}", {} , e.what());
            throw ex::Parse("Failed to parse authlib response: " + std::string(e.what()));
        }
        authlibPrefetched = util::base64::base64Encode(resJson.dump());

        bus::config::updateClientConfig([&authlibPrefetched](neko::ClientConfig &clientConfig) {
            clientConfig.minecraft.authlibPrefetched = authlibPrefetched.c_str();
        });
        bus::config::save(app::getConfigFileName());
    }

    /**
     * @brief Refreshes the Minecraft Auth token.
     * @throws ex::NetworkError if the token refresh fails or the network request encounters an error.
     * @throws ex::Parse if the response from the server cannot be parsed.
     * @details This function checks if the current Minecraft token is valid. If it is not valid, it attempts to refresh the token using the Authlib API. If the refresh is successful,
     * it updates the token and player information in the client configuration. If any errors occur during the process, appropriate exceptions are thrown with detailed error messages.
     */
    inline void authMinecraftTokenRefresh(AuthMode authMode = AuthMode::AuthlibInjector) {
        log::autoLog log;

        if (authMode == AuthMode::Offline) {
            log::info("Offline mode, skipping token refresh");
            return;
        }

        network::Network net;
        auto cfg = bus::config::getClientConfig();

        // Authlib Injector
        if (authMode == AuthMode::AuthlibInjector) {
            auto url = network::buildUrl(lc::api::authlib::validate, lc::api::authlib::host);
            nlohmann::json json = {{"accessToken", cfg.minecraft.accessToken}, {"requestUser", false}};
            network::RequestConfig reqConfig{
                .url = url,
                .method = network::RequestType::Post,
                .header = network::header::jsonContentHeader,
                .postData = json.dump()};
            auto result = net.execute(reqConfig);
            if (!result.hasError && result.statusCode == 204) {
                log::info("Token is valid");
                return;
            }

            // If token is not valid, refresh it
            log::info("token not validate and ready refresh");
            auto refUrl = network::buildUrl(lc::api::authlib::refresh, lc::api::authlib::host);
            reqConfig.url = refUrl;

            auto refResult = net.execute(reqConfig);

            if (!refResult.isSuccess() || !refResult.hasContent()) {
                log::error("Failed to refresh token, {}", {} , refResult.errorMessage);
                throw ex::NetworkError("Failed to refresh token , " + refResult.errorMessage);
            }

            // Check response content

            nlohmann::json refJsonData;
            try {
                refJsonData = nlohmann::json::parse(refResult.content);
            } catch (const nlohmann::json::parse_error &e) {
                log::error("Failed to parse refresh token response: {}", {} , e.what());
                throw ex::Parse("Failed to parse refresh token response : " + std::string(e.what()));
            }

            auto error = refJsonData.value("error", ""),
                 errorMsg = refJsonData.value("errorMessage", "");
            if (!error.empty() || !errorMsg.empty()) {
                log::error("Error refreshing token: {} - {}", {} ,error , errorMsg);
                throw ex::NetworkError("Error refreshing token: " + error + " - " + errorMsg);
            }

            if (!refJsonData.contains("accessToken")) {
                log::error("Missing accessToken in response");
                throw ex::Parse("Missing accessToken in response");
            }

            // Update token and player info in config

            std::string
                accessToken = refJsonData["accessToken"].get<std::string>(),
                uuid,
                name;

            // If selectedProfile is present, update uuid and name
            if (refJsonData.contains("selectedProfile") && refJsonData["selectedProfile"].is_object()) {
                uuid = refJsonData["selectedProfile"].value("id", "");
                name = refJsonData["selectedProfile"].value("name", "");

                bus::config::updateClientConfig([uuid, name](neko::ClientConfig &clientConfig) {
                    clientConfig.minecraft.uuid = uuid.c_str();
                    clientConfig.minecraft.playerName = name.c_str();
                });
            }

            // Always update accessToken
            bus::config::updateClientConfig([accessToken](neko::ClientConfig &clientConfig) {
                clientConfig.minecraft.accessToken = accessToken.c_str();
            });
            bus::config::save(app::getConfigFileName());

        } else {
            log::error("Unsupported auth mode for token refresh");
            throw ex::InvalidArgument("Unsupported auth mode for token refresh");
        }
    }

    struct LoginResult {
        std::string error;
        std::string name;
    };
    /**
     * @brief Authenticates a user with the API.
     * @param inData The input data containing username and password.
     * @return The result of the authentication attempt.
     * @note If the returned .error is not empty, it indicates failure and contains the error message.
     */
    inline LoginResult authLogin(const std::vector<std::string> &inData, AuthMode authMode = AuthMode::AuthlibInjector) noexcept {
        log::autoLog log;
        LoginResult result;

        // Offline
        if (authMode == AuthMode::Offline) {
            std::string uuid = util::uuid::uuidV3(inData[0], "OfflinePlayer:");
            bus::config::updateClientConfig([&](ClientConfig &clientConfig) {
                clientConfig.minecraft.account = inData[0].c_str();
                clientConfig.minecraft.playerName = inData[0].c_str();
                clientConfig.minecraft.uuid = uuid.c_str();
                clientConfig.minecraft.accessToken = "OfflineToken";
            });
            bus::config::save(app::getConfigFileName());
            result.name = inData[0];
            return result;
        }

        // Need at least username and password
        if (inData.size() < 2) {
            result.error = lang::tr(std::string(lang::keys::error::invalidInput));
            return result;
        }

        // Authlib Injector
        if (authMode == AuthMode::AuthlibInjector) {
            nlohmann::json json = {
                {"username", inData[0]},
                {"password", inData[1]},
                {"requestUser", false},
                {"agent", {{"name", "Minecraft"}, {"version", 1}}}};

            auto url = network::buildUrl(lc::api::authlib::authenticate, lc::api::authlib::host);
            network::Network net;
            network::RequestConfig reqConfig{
                .url = url,
                .method = network::RequestType::Post,
                .header = network::header::jsonContentHeader,
                .postData = json.dump()};

            auto netResult = net.execute(reqConfig);

            if (!netResult.isSuccess() || !netResult.hasContent()) {
                log::error("Failed to authenticate: {}", {} ,netResult.errorMessage);
                result.error = lang::tr(std::string(lang::keys::error::networkError));
                return result;
            }

            nlohmann::json resData;
            try {
                resData = nlohmann::json::parse(netResult.content);
            } catch (const nlohmann::json::parse_error &e) {
                log::error("Failed to parse authentication response: {}", {} , e.what());
                result.error = lang::tr(std::string(lang::keys::error::parseError)) + e.what();
                return result;
            }

            auto error = resData.value("error", ""),
                 errorMsg = resData.value("errorMessage", "");
            if (!error.empty() || !errorMsg.empty()) {
                result.error = error + ": " + errorMsg;
                return result;
            }

            if (!resData.contains("accessToken") || !resData.contains("selectedProfile") || !resData["selectedProfile"].is_object()) {
                log::error("Missing accessToken in response");
                result.error = lang::tr(std::string(lang::keys::minecraft::missingAccessToken)) + netResult.errorMessage;
                return result;
            }

            auto accessToken = resData.value("accessToken", "");
            auto uuid = resData["selectedProfile"].value("id", "");
            auto name = resData["selectedProfile"].value("name", "");

            bus::config::updateClientConfig([accessToken, uuid, name, inData](ClientConfig &clientConfig) {
                clientConfig.minecraft.accessToken = accessToken.c_str();
                clientConfig.minecraft.uuid = uuid.c_str();
                clientConfig.minecraft.playerName = name.c_str();
                clientConfig.minecraft.account = inData[0].c_str();
            });
            bus::config::save(app::getConfigFileName());

            result.name = name;

        } else {
            result.error = "Unsupported auth mode for login";
        }

        return result;
    }

    inline void authLogout(AuthMode authMode = AuthMode::AuthlibInjector) noexcept {
        log::autoLog log;
        neko::ClientConfig cfg = bus::config::getClientConfig();

        auto clear = []() {
            bus::config::updateClientConfig([](ClientConfig &clientConfig) {
                clientConfig.minecraft.account = "";
                clientConfig.minecraft.playerName = "";
                clientConfig.minecraft.uuid = "";
                clientConfig.minecraft.accessToken = "";
            });
            bus::config::save(app::getConfigFileName());
        };

        if (authMode == AuthMode::Offline) {
            clear();
            return;
        }

        if (authMode == AuthMode::AuthlibInjector) {
            auto url = network::buildUrl(lc::api::authlib::invalidate, lc::api::authlib::host);
            nlohmann::json json = {
                {"accessToken", cfg.minecraft.accessToken}};

            network::Network net;
            network::RequestConfig reqConfig{
                .url = url,
                .method = network::RequestType::Post,
                .requestId = "logout-" + util::random::generateRandomString(10),
                .header = network::header::jsonContentHeader,
                .postData = json.dump()
            };

            (void)net.execute(reqConfig);
            clear();
            return;
        }
        
    }

} // namespace neko::minecraft::auth