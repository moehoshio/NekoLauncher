#pragma once

#include "neko/core/core.hpp"
#include "neko/core/resources.hpp"

#include "neko/log/nlog.hpp"
#include "neko/schema/exception.hpp"

#include "neko/function/info.hpp"
#include "neko/function/utilities.hpp"

#include "neko/network/network.hpp"

#include "library/nlohmann/json.hpp"

#include <string>
#include <string_view>

/**
 * @brief Minecraft account-related functions.
 * @namespace neko::minecraft::account
 */
namespace neko::minecraft::account {

    /**
     * @brief Refreshes the Minecraft launcher token.
     * @throws ex::NetworkError if the token refresh fails or the network request encounters an error.
     * @throws ex::Parse if the response from the server cannot be parsed.
     * @details This function checks if the current Minecraft token is valid. If it is not valid, it attempts to refresh the token using the Authlib API. If the refresh is successful,
     * it updates the token and player information in the client configuration. If any errors occur during the process, appropriate exceptions are thrown with detailed error messages.
     */
    inline void launcherMinecraftTokenRefresh() {
        log::autoLog log;

        network::Network net;
        auto url = network::NetworkBase::buildUrl(network::NetworkBase::Api::Authlib::validate, network::NetworkBase::Api::Authlib::host);

        ClientConfig clientConfig(core::getConfigObj());

        nlohmann::json json = {{"accessToken", clientConfig.minecraft.accessToken}};

        network::RequestConfig reqConfig;
        reqConfig.setUrl(url)
            .setMethod(network::RequestType::Post)
            .setHeader(network::NetworkBase::HeaderGlobal::jsonContentTypeHeader)
            .setData(json.dump());

        auto result = net.execute(reqConfig);
        if (!result.hasError && result.statusCode == 204) {
            log::Info("Token is valid");
            return;
        }

        log::Info("token is not validate");

        auto refUrl = network::NetworkBase::buildUrl(network::NetworkBase::Api::Authlib::refresh, network::NetworkBase::Api::Authlib::host);

        nlohmann::json refJson = {
            {"accessToken", clientConfig.minecraft.accessToken}, {"requestUser", false}};

        reqConfig.setUrl(refUrl)
            .setMethod(network::RequestType::Post)
            .setHeader(network::NetworkBase::HeaderGlobal::jsonContentTypeHeader)
            .setData(refJson.dump());

        auto refResult = net.execute(reqConfig);

        if (!refResult.isSuccess() || !refResult.hasContent()) {
            log::Err(log::SrcLoc::current(), "Failed to refresh token: %s", refResult.errorMessage.c_str());
            throw ex::NetworkError("Failed to refresh token", ex::ExceptionExtensionInfo{});
        }

        nlohmann::json refJsonData;
        try {
            refJsonData = nlohmann::json::parse(refResult.content);
        } catch (const nlohmann::json::parse_error &e) {
            log::Err(log::SrcLoc::current(), "Failed to parse refresh token response: %s", e.what());
            throw ex::Parse("Failed to parse refresh token response : " + std::string(e.what()), ex::ExceptionExtensionInfo{});
        }

        auto error = refJsonData.value("error", ""),
             errorMsg = refJsonData.value("errorMessage", "");
        if (!error.empty() || !errorMsg.empty()) {
            log::Err(log::SrcLoc::current(), "Error refreshing token: %s - %s", error.c_str(), errorMsg.c_str());
            throw ex::NetworkError("Error refreshing token: " + error + " - " + errorMsg, ex::ExceptionExtensionInfo{});
        }

        if (!refJsonData.contains("accessToken")) {
            log::Err("Missing accessToken in response");
            throw ex::Parse("Missing accessToken in response", ex::ExceptionExtensionInfo{});
        }

        std::string
            accessToken = refJsonData["accessToken"].get<std::string>(),
            uuid,
            name;

        if (refJsonData.contains("selectedProfile") && refJsonData["selectedProfile"].is_object()) {
            uuid = refJsonData["selectedProfile"].value("id", "");
            name = refJsonData["selectedProfile"].value("name", "");

            clientConfig.minecraft.uuid = uuid.c_str();
            clientConfig.minecraft.playerName = name.c_str();
        }
        clientConfig.minecraft.accessToken = accessToken.c_str();
        clientConfig.save(core::getConfigObj(), info::app::getConfigFileName());
    }

    /**
     * @brief This function checks if the Minecraft authlib prefetch data is already stored in the client configuration. If it is not, it attempts to fetch the data from the network.
     * @throws ex::NetworkError if the token refresh fails or the network request encounters an error.
     * @throws ex::Parse if the response from the server cannot be parsed.
     */
    inline void launcherMinecraftAuthlibAndPrefetchedCheck() {
        log::autoLog log;
        ClientConfig clientConfig(core::getConfigObj());
        std::string authlibPrefetched = clientConfig.minecraft.authlibPrefetched;
        if (!authlibPrefetched.empty())
            return;

        auto url = network::NetworkBase::buildUrl(network::NetworkBase::Api::Authlib::root, network::NetworkBase::Api::Authlib::host);
        network::Network net;

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Get,
        };
        auto result = net.execute(reqConfig);
        if (!result.isSuccess() || !result.hasContent()) {
            log::Err(log::SrcLoc::current(), "Failed to fetch authlib data: %s", result.errorMessage.c_str());
            throw ex::NetworkError("Failed to fetch authlib data", ex::ExceptionExtensionInfo{});
        }

        nlohmann::json resJson;
        try {
            resJson = nlohmann::json::parse(result.content);
        } catch (const nlohmann::json::parse_error &e) {
            log::Err(log::SrcLoc::current(), "Failed to parse authlib response: %s", e.what());
            throw ex::Parse("Failed to parse authlib response: " + std::string(e.what()), ex::ExceptionExtensionInfo{});
        }
        authlibPrefetched = util::base64::base64Encode(resJson.dump());
        clientConfig.minecraft.authlibPrefetched = authlibPrefetched.c_str();
        clientConfig.save(core::getConfigObj(), info::app::getConfigFileName());
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
    inline LoginResult authLogin(const std::vector<std::string> &inData) noexcept {
        log::autoLog log;
        LoginResult result;
        if (inData.size() < 2) {
            result.error = info::lang::tr(info::lang::Keys::Error::invalidInput);
            return result;
        }

        nlohmann::json json = {
            {"username", inData[0]},
            {"password", inData[1]},
            {"requestUser", false},
            {"agent", {{"name", "Minecraft"}, {"version", 1}}}};

        auto data = json.dump();
        auto url = network::NetworkBase::buildUrl(network::NetworkBase::Api::Authlib::authenticate, network::NetworkBase::Api::Authlib::host);
        network::Network net;
        network::NetworkResult reqConfig;
        reqConfig.setUrl(url)
            .setMethod(network::RequestType::Post)
            .setHeader(network::NetworkBase::HeaderGlobal::jsonContentTypeHeader)
            .setData(data);

        auto result = net.execute(reqConfig);

        if (!result.isSuccess() || !result.hasContent()) {
            log::Err(log::SrcLoc::current(), "Failed to authenticate: %s", result.errorMessage.c_str());
            result.error = info::lang::tr(info::lang::Keys::Error::networkError);
            return result;
        }

        nlohmann::json resData;
        try {
            resData = nlohmann::json::parse(result.content);
        } catch (const nlohmann::json::parse_error &e) {
            log::Err(log::SrcLoc::current(), "Failed to parse authentication response: %s", e.what());
            result.error = info::lang::tr(info::lang::Keys::Error::jsonParse) + e.what();
            return result;
        }

        auto error = resData.value("error", ""),
             errorMsg = resData.value("errorMessage", "");
        if (!error.empty() || !errorMsg.empty()) {
            result.error = error + errorMsg;
            return result;
        }

        if (!resData.contains("accessToken") || !resData.contains("selectedProfile") || !resData["selectedProfile"].is_object()) {
            log::Err(log::SrcLoc::current(), "Missing accessToken in response");
            result.error = info::lang::tr(info::lang::Keys::Error::missingAccessToken) + result.error;
            return result;
        }

        auto accessToken = resData.value("accessToken", "");
        auto uuid = resData["selectedProfile"].value("id", "");
        auto name = resData["selectedProfile"].value("name", "");

        ClientConfig clientConfig(core::getConfigObj());
        clientConfig.minecraft.accessToken = accessToken.c_str();
        clientConfig.minecraft.uuid = uuid.c_str();
        clientConfig.minecraft.playerName = name.c_str();
        clientConfig.minecraft.account = inData[0].c_str();
        clientConfig.save(core::getConfigObj(), info::app::getConfigFileName());

        result.name = name;
        return result;
    }

    inline void authLogout() noexcept {
        log::autoLog log;
        neko::ClientConfig cfg(core::getConfigObj());

        auto url = network::NetworkBase::buildUrl(network::NetworkBase::Api::Authlib::invalidate, network::NetworkBase::Api::Authlib::host);
        nlohmann::json json = {
            {"accessToken", cfg.minecraft.accessToken}};
        auto data = json.dump();

        network::Network net;
        network::RequestConfig reqConfig;
        reqConfig.setUrl(url)
            .setMethod(network::RequestType::Post)
            .setData(data)
            .setHeader("Content-Type: application/json")
            .setRequestId("logout-" + util::random::generateRandomString(10));
        (void)net.execute(reqConfig);

        cfg.minecraft.account = "";
        cfg.minecraft.playerName = "";
        cfg.minecraft.accessToken = "";
        cfg.minecraft.uuid = "";

        cfg.save(core::getConfigObj(), app::getConfigFileName());
    }

} // namespace neko::minecraft::account
