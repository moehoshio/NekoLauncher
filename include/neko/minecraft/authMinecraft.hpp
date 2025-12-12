/**
 * @file authMinecraft.hpp
 * @brief A Minecraft authentication module
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include "neko/bus/configBus.hpp"

#include <string>
#include <vector>

namespace neko::minecraft::auth {

    enum class AuthMode {
        // Microsoft, // To Be implemented
        AuthlibInjector,
        Offline
    };

    inline bool isLoggedIn() noexcept {
        auto cfg = bus::config::getClientConfig();
        return !cfg.minecraft.accessToken.empty() && !cfg.minecraft.uuid.empty() && !cfg.minecraft.playerName.empty();
    }

    inline std::string getPlayerName() noexcept {
        auto cfg = bus::config::getClientConfig();
        return cfg.minecraft.playerName;
    }

    /**
     * @brief This function checks if the Minecraft authlib prefetch data is already stored in the client configuration. If it is not, it attempts to fetch the data from the network.
     * @throws ex::NetworkError if the token refresh fails or the network request encounters an error.
     * @throws ex::Parse if the response from the server cannot be parsed.
     */
    void authMinecraftAuthlibAndPrefetchedCheck();

    /**
     * @brief Refreshes the Minecraft Auth token.
     * @throws ex::NetworkError if the token refresh fails or the network request encounters an error.
     * @throws ex::Parse if the response from the server cannot be parsed.
     * @details This function checks if the current Minecraft token is valid. If it is not valid, it attempts to refresh the token using the Authlib API. If the refresh is successful,
     * it updates the token and player information in the client configuration. If any errors occur during the process, appropriate exceptions are thrown with detailed error messages.
     */
    void authMinecraftTokenRefresh(AuthMode authMode = AuthMode::AuthlibInjector);

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
    LoginResult authLogin(const std::vector<std::string> &inData, AuthMode authMode = AuthMode::AuthlibInjector) noexcept;

    void authLogout(AuthMode authMode = AuthMode::AuthlibInjector) noexcept;

} // namespace neko::minecraft::auth