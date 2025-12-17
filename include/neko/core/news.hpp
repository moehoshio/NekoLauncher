#pragma once

#include "neko/app/api.hpp"

#include <optional>
#include <string>
#include <vector>

namespace neko::core {

    /**
     * @brief Fetches news from the news API.
     * @param config The launcher configuration containing retry settings.
     * @param limit Maximum number of news items to fetch (default 10).
     * @param categories Optional category filter.
     * @param lastId Optional ID for pagination.
     * @return Optional NewsResponse if news is available, nullopt if no news (204).
     * @throws ex::NetworkError if the network request fails.
     * @throws ex::Parse if the response cannot be parsed.
     */
    std::optional<api::NewsResponse> fetchNews(
        const api::LauncherConfigResponse &config,
        neko::int32 limit = 10,
        const std::vector<std::string> &categories = {},
        const std::string &lastId = "");

} // namespace neko::core
