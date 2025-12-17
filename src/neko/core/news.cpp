#pragma once

// Neko Module
#include <neko/schema/exception.hpp>
#include <neko/schema/types.hpp>

#include <neko/log/nlog.hpp>

#include <neko/function/utilities.hpp>
#include <neko/network/network.hpp>

// NekoLc project
#include "neko/app/api.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/nekoLc.hpp"

#include "neko/core/news.hpp"

#include <nlohmann/json.hpp>

namespace neko::core {

    std::optional<api::NewsResponse> fetchNews(
        const api::LauncherConfigResponse &config,
        neko::int32 limit,
        const std::vector<std::string> &categories,
        const std::string &lastId) {
        
        log::autoLog log;
        network::Network net;

        // Build request JSON
        nlohmann::json newsRequestBody;
        newsRequestBody["newsRequest"] = {
            {"clientInfo", app::getClientInfo()},
            {"timestamp", util::time::getUtcNow()},
            {"limit", limit}
        };
        
        if (!categories.empty()) {
            newsRequestBody["newsRequest"]["categories"] = categories;
        }
        if (!lastId.empty()) {
            newsRequestBody["newsRequest"]["lastId"] = lastId;
        }
        
        newsRequestBody["preferences"] = app::getPreferences();

        auto url = network::buildUrl(lc::api::news);

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .requestId = "news-" + util::random::generateRandomString(6),
            .header = network::header::jsonContentHeader,
            .postData = newsRequestBody.dump()};

        network::RetryConfig retryConfig{
            .config = reqConfig,
            .maxRetries = config.maxRetryCount,
            .retryDelay = std::chrono::seconds(config.retryIntervalSec),
            .successCodes = {200, 204}};

        auto result = net.executeWithRetry(retryConfig);

        if (!result.hasError && result.statusCode == 204) {
            log::info("No news available (204)");
            return std::nullopt;
        }

        if (!result.isSuccess() || !result.hasContent()) {
            std::string errMsg = "Failed to fetch news: " + result.errorMessage;
            log::error(errMsg);
            throw ex::NetworkError(errMsg);
        }

        log::debug("News response: {}", {}, result.content);

        try {
            auto jsonData = nlohmann::json::parse(result.content);
            api::NewsResponse response;
            api::from_json(jsonData, response);
            
            log::info("Fetched {} news items", {}, std::to_string(response.items.size()));
            return response;

        } catch (const nlohmann::json::parse_error &e) {
            std::string errMsg = "Failed to parse news response: " + std::string(e.what());
            log::error(errMsg);
            throw ex::ParseError(errMsg);
        } catch (const nlohmann::json::exception &e) {
            std::string errMsg = "Failed to parse news response: " + std::string(e.what());
            log::error(errMsg);
            throw ex::ParseError(errMsg);
        }
    }

} // namespace neko::core
