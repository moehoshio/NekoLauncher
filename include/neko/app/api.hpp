/**
 * @file api.hpp
 * @brief Neko Launcher API data structures and JSON serialization/deserialization.
 * @author moehoshio
 * @see https://github.com/moehoshio/NekoLcApi/wiki
 */

#pragma once

#include <neko/schema/types.hpp>

#include <nlohmann/json.hpp>

#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace neko::api {

    struct Error {
        std::string
            error,
            errorType,
            errorMessage;
        bool empty() const noexcept {
            return error.empty() && errorType.empty() && errorMessage.empty();
        }
    };

    struct Meta {
        std::string
            apiVersion,
            minApiVersion,
            buildVersion,
            releaseDate,
            deprecatedMessage;
        neko::int64 timestamp;
        bool isDeprecated = false;
        bool empty() const noexcept {
            return apiVersion.empty() && minApiVersion.empty() && buildVersion.empty() && releaseDate.empty() && deprecatedMessage.empty();
        }
    };

    struct Preferences {
        std::string
            language;
        bool empty() const noexcept {
            return language.empty();
        }
    };

    struct App {
        std::string
            appName,
            coreVersion,
            resourceVersion,
            buildId;
        bool empty() const noexcept {
            return appName.empty() && coreVersion.empty() && resourceVersion.empty() && buildId.empty();
        }
    };

    struct System {
        std::string
            os,
            arch,
            osVersion;
        bool empty() const noexcept {
            return os.empty() && arch.empty() && osVersion.empty();
        }
    };

    struct Extra {
        bool empty() const noexcept {
            return true;
        }
    };

    struct ClientInfo {
        App app;
        System system;
        Extra extra;
        std::string
            deviceId;

        bool empty() const noexcept {
            return app.empty() && system.empty() && extra.empty() && deviceId.empty();
        }
    };

    struct Auth {

        struct Account {
            std::string
                username,
                password;
            bool empty() const noexcept {
                return username.empty() && password.empty();
            }
        } account;

        struct Signature {
            std::string
                identifier,
                timestamp,
                signature;
            bool empty() const noexcept {
                return identifier.empty() && timestamp.empty() && signature.empty();
            }
        } signature;

        struct Token {
            std::string
                accessToken,
                refreshToken;
            bool empty() const noexcept {
                return accessToken.empty() && refreshToken.empty();
            }
        } token;

        Preferences preferences;

        bool empty() const noexcept {
            return account.empty() && signature.empty() && token.empty() && preferences.empty();
        }
    };

    struct LauncherConfigResponse {
        std::vector<std::string> host;
        neko::int32
            retryIntervalSec,
            maxRetryCount;
        Meta meta;

        struct WebSocket {
            bool enable;
            std::string socketHost;
            neko::int32 heartbeatIntervalSec;
            bool empty() const noexcept {
                return !enable && socketHost.empty() && heartbeatIntervalSec == 0;
            }
        } webSocket;

        struct Security {
            bool enableAuthentication;
            neko::int32
                tokenExpirationSec,
                refreshTokenExpirationDays;
            std::string
                loginUrl,
                logoutUrl,
                refreshTokenUrl;

            bool empty() const noexcept {
                return !enableAuthentication && tokenExpirationSec == 0 && refreshTokenExpirationDays == 0 && loginUrl.empty() && logoutUrl.empty() && refreshTokenUrl.empty();
            }
        } security;

        bool empty() const noexcept {
            return host.empty() && retryIntervalSec == 0 && maxRetryCount == 0 && webSocket.empty() && security.empty() && meta.empty();
        }
    };

    struct MaintenanceResponse {
        std::string
            status,
            message,
            startTime,
            exEndTime,
            posterUrl,
            link;
        Meta meta;
        bool empty() const noexcept {
            return status.empty() && message.empty() && startTime.empty() && exEndTime.empty() && posterUrl.empty() && link.empty() && meta.empty();
        }
        bool isMaintenance() const noexcept {
            return !status.empty() && status == "progress";
        }
        bool isScheduled() const noexcept {
            return !status.empty() && status == "scheduled";
        }
    };

    struct UpdateResponse {
        std::string title,
            description,
            posterUrl,
            publishTime;
        std::string resourceVersion;
        bool isMandatory;
        Meta meta;
        struct File {
            std::string url;
            std::string fileName;
            std::string checksum;
            std::string hashAlgorithm;
            bool suggestMultiThread;
            bool isCoreFile;
            bool isAbsoluteUrl;
            bool empty() const noexcept {
                return url.empty() && fileName.empty() && checksum.empty();
            }
        };

        std::vector<File> files;

        bool empty() const noexcept {
            return title.empty() && description.empty() && posterUrl.empty() && publishTime.empty() && files.empty() && meta.empty();
        }
    };

    struct NewsItem {
        std::string
            id,
            title,
            summary,
            content,
            posterUrl,
            link,
            publishTime,
            category;
        std::vector<std::string> tags;
        neko::int32 priority = 0;

        bool empty() const noexcept {
            return id.empty() && title.empty() && summary.empty();
        }
    };

    struct NewsResponse {
        std::vector<NewsItem> items;
        bool hasMore = false;
        Meta meta;

        bool empty() const noexcept {
            return items.empty() && meta.empty();
        }
    };

    struct NewsRequest {
        ClientInfo clientInfo;
        neko::int64 timestamp;
        neko::int32 limit = 10;
        std::vector<std::string> categories;
        std::string lastId;
        Preferences preferences;

        bool empty() const noexcept {
            return clientInfo.empty() && timestamp == 0;
        }
    };

    struct FeedbackLogRequest {
        ClientInfo clientInfo;
        neko::int64 timestamp;
        std::string logContent;
        Preferences preferences;

        bool empty() const noexcept {
            return clientInfo.empty() && timestamp == 0 && logContent.empty() && preferences.empty();
        }
    };

    struct WebSocketServerSide {
        std::string
            action,
            messageId;
        std::vector<Error> errors;
        Meta meta;
        struct NotifyChanged {
            std::string
                type,
                message;
            ClientInfo clientInfo;
            bool empty() const noexcept {
                return type.empty() && message.empty() && clientInfo.empty();
            }
        } notifyChanged;

        bool empty() const noexcept {
            return action.empty() && messageId.empty() && errors.empty() && meta.empty() && notifyChanged.empty();
        }
        bool isPing() const noexcept {
            return action == "ping";
        }
        bool isPong() const noexcept {
            return action == "pong";
        }
        bool isNotifyChanged() const noexcept {
            return action == "notifyChanged" && !notifyChanged.empty();
        }
        bool hasError() const noexcept {
            return !errors.empty();
        }
    };
    struct WebSocketClientSide {
        std::string
            action,
            accessToken,
            lastMessageId;
        ClientInfo clientInfo;
        Preferences preferences;
        bool empty() const noexcept {
            return action.empty() && accessToken.empty() && lastMessageId.empty() && clientInfo.empty() && preferences.empty();
        }
        bool isPing() const noexcept {
            return action == "ping";
        }
        bool isPong() const noexcept {
            return action == "pong";
        }
        bool isQuery() const noexcept {
            return action == "query";
        }
    };

    struct StaticConfig {

        struct StaticLauncherConfig : public LauncherConfigResponse {

            struct CheckUpdateUrls {
                System system;
                std::string url;
                bool empty() const noexcept {
                    return system.empty() && url.empty();
                }
            };
            std::vector<CheckUpdateUrls> checkUpdateUrls;
            bool empty() const noexcept {
                return LauncherConfigResponse::empty() && checkUpdateUrls.empty();
            }

            std::optional<std::string> getCheckUpdateUrl(const std::string &os, const std::string &arch, const std::string &osVersionRegex) const noexcept {
                if (checkUpdateUrls.empty()) {
                    return std::nullopt;
                }

                for (const auto &it : checkUpdateUrls) {
                    std::regex versionRegex(it.system.osVersion);
                    if (it.system.os == os && it.system.arch == arch && std::regex_search(osVersionRegex, versionRegex)) {
                        return it.url;
                    }
                }
                return std::nullopt;
            }
        } staticLauncherConfig;

        struct StaticMaintenanceInfo : public MaintenanceResponse {
            bool isCompleted() const noexcept {
                return status.empty() || status == "completed";
            }
        } staticMaintenanceInfo;

        bool empty() const noexcept {
            return staticLauncherConfig.empty() && staticMaintenanceInfo.empty();
        }
    };

    struct StaticUpdateInfo {
        std::string
            coreVersion,
            resourceVersion;
        UpdateResponse updateResponse;
        bool empty() const noexcept {
            return coreVersion.empty() && resourceVersion.empty() && updateResponse.empty();
        }
    };

    // Error
    inline void to_json(nlohmann::json &j, const Error &e) {
        j = nlohmann::json{
            {"error", e.error},
            {"errorType", e.errorType},
            {"errorMessage", e.errorMessage}};
    }
    inline void from_json(const nlohmann::json &j, Error &e) {
        e.error = j.value("error", "");
        e.errorType = j.value("errorType", "");
        e.errorMessage = j.value("errorMessage", "");
    }

    // Meta
    inline void to_json(nlohmann::json &j, const Meta &meta) {
        j = nlohmann::json{
            {"apiVersion", meta.apiVersion},
            {"minApiVersion", meta.minApiVersion},
            {"buildVersion", meta.buildVersion},
            {"releaseDate", meta.releaseDate},
            {"deprecatedMessage", meta.deprecatedMessage},
            {"timestamp", meta.timestamp},
            {"isDeprecated", meta.isDeprecated}};
    }
    inline void from_json(const nlohmann::json &j, Meta &meta) {
        meta.apiVersion = j.value("apiVersion", "");
        meta.minApiVersion = j.value("minApiVersion", "");
        meta.buildVersion = j.value("buildVersion", "");
        meta.releaseDate = j.value("releaseDate", "");
        meta.deprecatedMessage = j.value("deprecatedMessage", "");
        meta.timestamp = j.value("timestamp", 0);
        meta.isDeprecated = j.value("isDeprecated", false);
    }

    // Preferences
    inline void to_json(nlohmann::json &j, const Preferences &preferences) {
        j = nlohmann::json{
            {"language", preferences.language}};
    }
    inline void from_json(const nlohmann::json &j, Preferences &preferences) {
        preferences.language = j.value("language", "");
    }

    // App
    inline void to_json(nlohmann::json &j, const App &app) {
        j = nlohmann::json{
            {"appName", app.appName},
            {"coreVersion", app.coreVersion},
            {"resourceVersion", app.resourceVersion},
            {"buildId", app.buildId}};
    }
    inline void from_json(const nlohmann::json &j, App &app) {
        app.appName = j.value("appName", "");
        app.coreVersion = j.value("coreVersion", "");
        app.resourceVersion = j.value("resourceVersion", "");
        app.buildId = j.value("buildId", "");
    }

    // System
    inline void to_json(nlohmann::json &j, const System &system) {
        j = nlohmann::json{
            {"os", system.os},
            {"arch", system.arch},
            {"osVersion", system.osVersion}};
    }
    inline void from_json(const nlohmann::json &j, System &system) {
        system.os = j.value("os", "");
        system.arch = j.value("arch", "");
        system.osVersion = j.value("osVersion", "");
    }

    // Extra
    inline void to_json(nlohmann::json &j, const Extra &extra) {
        j = nlohmann::json::object(); // Empty object for now
    }

    inline void from_json(const nlohmann::json &j, Extra &extra) {
        // No fields to parse, just an empty object
        extra = Extra();
    }

    // ClientInfo
    inline void to_json(nlohmann::json &j, const ClientInfo &clientInfo) {
        j = nlohmann::json{
            {"app", clientInfo.app},
            {"system", clientInfo.system},
            {"extra", clientInfo.extra},
            {"deviceId", clientInfo.deviceId}};
    }
    inline void from_json(const nlohmann::json &j, ClientInfo &clientInfo) {
        from_json(j.at("app"), clientInfo.app);
        from_json(j.at("system"), clientInfo.system);
        from_json(j.at("extra"), clientInfo.extra);
        clientInfo.deviceId = j.value("deviceId", "");
    }

    // Account
    inline void to_json(nlohmann::json &j, const Auth::Account &account) {
        j = nlohmann::json{
            {"username", account.username},
            {"password", account.password}};
    }
    inline void from_json(const nlohmann::json &j, Auth::Account &account) {
        account.username = j.value("username", "");
        account.password = j.value("password", "");
    }

    // Signature
    inline void to_json(nlohmann::json &j, const Auth::Signature &signature) {
        j = nlohmann::json{
            {"identifier", signature.identifier},
            {"timestamp", signature.timestamp},
            {"signature", signature.signature}};
    }
    inline void from_json(const nlohmann::json &j, Auth::Signature &signature) {
        signature.identifier = j.value("identifier", "");
        signature.timestamp = j.value("timestamp", 0);
        signature.signature = j.value("signature", "");
    }

    // Token
    inline void to_json(nlohmann::json &j, const Auth::Token &token) {
        j = nlohmann::json{
            {"accessToken", token.accessToken},
            {"refreshToken", token.refreshToken}};
    }
    inline void from_json(const nlohmann::json &j, Auth::Token &token) {
        token.accessToken = j.value("accessToken", "");
        token.refreshToken = j.value("refreshToken", "");
    }

    // Auth
    inline void to_json(nlohmann::json &j, const Auth &auth) {
        j = nlohmann::json{
            {"account", auth.account},
            {"signature", auth.signature},
            {"token", auth.token},
            {"preferences", auth.preferences}};
    }
    inline void from_json(const nlohmann::json &j, Auth &auth) {
        from_json(j.at("account"), auth.account);
        from_json(j.at("signature"), auth.signature);
        from_json(j.at("token"), auth.token);
        from_json(j.at("preferences"), auth.preferences);
    }

    // WebSocket
    inline void to_json(nlohmann::json &j, const LauncherConfigResponse::WebSocket &webSocket) {
        j = nlohmann::json{
            {"enable", webSocket.enable},
            {"socketHost", webSocket.socketHost},
            {"heartbeatIntervalSec", webSocket.heartbeatIntervalSec}};
    }
    inline void from_json(const nlohmann::json &j, LauncherConfigResponse::WebSocket &webSocket) {
        webSocket.enable = j.value("enable", false);
        webSocket.socketHost = j.value("socketHost", "");
        webSocket.heartbeatIntervalSec = j.value("heartbeatIntervalSec", 0);
    }

    // Security
    inline void to_json(nlohmann::json &j, const LauncherConfigResponse::Security &security) {
        j = nlohmann::json{
            {"enableAuthentication", security.enableAuthentication},
            {"tokenExpirationSec", security.tokenExpirationSec},
            {"refreshTokenExpirationDays", security.refreshTokenExpirationDays},
            {"loginUrl", security.loginUrl},
            {"logoutUrl", security.logoutUrl},
            {"refreshTokenUrl", security.refreshTokenUrl}};
    }
    inline void from_json(const nlohmann::json &j, LauncherConfigResponse::Security &security) {
        security.enableAuthentication = j.value("enableAuthentication", false);
        security.tokenExpirationSec = j.value("tokenExpirationSec", 0);
        security.refreshTokenExpirationDays = j.value("refreshTokenExpirationDays", 0);
        security.loginUrl = j.value("loginUrl", "");
        security.logoutUrl = j.value("logoutUrl", "");
        // Accept both refreshTokenUrl and refreshUrl for compatibility
        security.refreshTokenUrl = j.value("refreshTokenUrl", j.value("refreshUrl", ""));
    }

    // LauncherConfigResponse
    inline void to_json(nlohmann::json &j, const LauncherConfigResponse &config) {
        j = nlohmann::json{
            {"host", config.host},
            {"retryIntervalSec", config.retryIntervalSec},
            {"maxRetryCount", config.maxRetryCount},
            {"webSocket", config.webSocket},
            {"security", config.security},
            {"meta", config.meta}};
    }
    inline void from_json(const nlohmann::json &j, LauncherConfigResponse &config) {
        // Allow wrapped payload: {"launcherConfigResponse": {...}, "meta": {...}}
        const nlohmann::json &payload = j.contains("launcherConfigResponse") ? j.at("launcherConfigResponse") : j;

        config.host = payload.value("host", std::vector<std::string>{});
        config.retryIntervalSec = payload.value("retryIntervalSec", 0);
        config.maxRetryCount = payload.value("maxRetryCount", 0);

        const auto &wsJson = payload.contains("webSocket") ? payload.at("webSocket") : nlohmann::json::object();
        from_json(wsJson, config.webSocket);

        const auto &securityJson = payload.contains("security") ? payload.at("security") : nlohmann::json::object();
        from_json(securityJson, config.security);

        if (payload.contains("meta")) {
            from_json(payload.at("meta"), config.meta);
        } else if (j.contains("meta")) {
            from_json(j.at("meta"), config.meta);
        } else {
            config.meta = {};
        }
    }

    // MaintenanceResponse
    inline void to_json(nlohmann::json &j, const MaintenanceResponse &maintenance) {
        j = nlohmann::json{
            {"status", maintenance.status},
            {"message", maintenance.message},
            {"startTime", maintenance.startTime},
            {"exEndTime", maintenance.exEndTime},
            {"posterUrl", maintenance.posterUrl},
            {"link", maintenance.link},
            {"meta", maintenance.meta}};
    }
    inline void from_json(const nlohmann::json &j, MaintenanceResponse &maintenance) {
        maintenance.status = j.value("status", "");
        maintenance.message = j.value("message", "");
        maintenance.startTime = j.value("startTime", "");
        maintenance.exEndTime = j.value("exEndTime", "");
        maintenance.posterUrl = j.value("posterUrl", "");
        maintenance.link = j.value("link", "");
        from_json(j.at("meta"), maintenance.meta);
    }

    // UpdateResponse::File
    inline void to_json(nlohmann::json &j, const UpdateResponse::File &file) {
        j = nlohmann::json{
            {"url", file.url},
            {"fileName", file.fileName},
            {"checksum", file.checksum},
            {"hashAlgorithm", file.hashAlgorithm},
            {"suggestMultiThread", file.suggestMultiThread},
            {"isCoreFile", file.isCoreFile},
            {"isAbsoluteUrl", file.isAbsoluteUrl}};
    }
    inline void from_json(const nlohmann::json &j, UpdateResponse::File &file) {
        file.url = j.value("url", "");
        file.fileName = j.value("fileName", "");
        file.checksum = j.value("checksum", "");
        file.hashAlgorithm = j.value("hashAlgorithm", "");
        file.suggestMultiThread = j.value("suggestMultiThread", false);
        file.isCoreFile = j.value("isCoreFile", false);
        file.isAbsoluteUrl = j.value("isAbsoluteUrl", false);
    }

    // UpdateResponse
    inline void to_json(nlohmann::json &j, const UpdateResponse &update) {
        j = nlohmann::json{
            {"title", update.title},
            {"description", update.description},
            {"posterUrl", update.posterUrl},
            {"publishTime", update.publishTime},
            {"resourceVersion", update.resourceVersion},
            {"isMandatory", update.isMandatory},
            {"meta", update.meta},
            {"files", update.files}};
    }
    inline void from_json(const nlohmann::json &j, UpdateResponse &update) {
        update.title = j.value("title", "");
        update.description = j.value("description", "");
        update.posterUrl = j.value("posterUrl", "");
        update.publishTime = j.value("publishTime", "");
        update.resourceVersion = j.value("resourceVersion", "");
        update.isMandatory = j.value("isMandatory", false);
        from_json(j.at("meta"), update.meta);

        if (j.contains("files")) {
            for (const auto &fileJson : j.at("files")) {
                UpdateResponse::File file;
                from_json(fileJson, file);
                update.files.push_back(file);
            }
        }
    }

    // NewsItem
    inline void to_json(nlohmann::json &j, const NewsItem &item) {
        j = nlohmann::json{
            {"id", item.id},
            {"title", item.title},
            {"summary", item.summary},
            {"content", item.content},
            {"posterUrl", item.posterUrl},
            {"link", item.link},
            {"publishTime", item.publishTime},
            {"category", item.category},
            {"tags", item.tags},
            {"priority", item.priority}};
    }
    inline void from_json(const nlohmann::json &j, NewsItem &item) {
        item.id = j.value("id", "");
        item.title = j.value("title", "");
        item.summary = j.value("summary", "");
        item.content = j.value("content", "");
        item.posterUrl = j.value("posterUrl", "");
        item.link = j.value("link", "");
        item.publishTime = j.value("publishTime", "");
        item.category = j.value("category", "");
        item.tags = j.value("tags", std::vector<std::string>{});
        item.priority = j.value("priority", 0);
    }

    // NewsResponse
    inline void to_json(nlohmann::json &j, const NewsResponse &response) {
        j = nlohmann::json{
            {"items", response.items},
            {"hasMore", response.hasMore},
            {"meta", response.meta}};
    }
    inline void from_json(const nlohmann::json &j, NewsResponse &response) {
        // Allow wrapped payload: {"newsResponse": {...}, "meta": {...}}
        const nlohmann::json &payload = j.contains("newsResponse") ? j.at("newsResponse") : j;

        if (payload.contains("items")) {
            for (const auto &itemJson : payload.at("items")) {
                NewsItem item;
                from_json(itemJson, item);
                response.items.push_back(item);
            }
        }
        response.hasMore = payload.value("hasMore", false);

        if (payload.contains("meta")) {
            from_json(payload.at("meta"), response.meta);
        } else if (j.contains("meta")) {
            from_json(j.at("meta"), response.meta);
        } else {
            response.meta = {};
        }
    }

    // NewsRequest
    inline void to_json(nlohmann::json &j, const NewsRequest &request) {
        j = nlohmann::json{
            {"clientInfo", request.clientInfo},
            {"timestamp", request.timestamp},
            {"limit", request.limit},
            {"categories", request.categories},
            {"lastId", request.lastId},
            {"preferences", request.preferences}};
    }
    inline void from_json(const nlohmann::json &j, NewsRequest &request) {
        from_json(j.at("clientInfo"), request.clientInfo);
        request.timestamp = j.value("timestamp", 0);
        request.limit = j.value("limit", 10);
        request.categories = j.value("categories", std::vector<std::string>{});
        request.lastId = j.value("lastId", "");
        from_json(j.at("preferences"), request.preferences);
    }

    // FeedbackLogRequest
    inline void to_json(nlohmann::json &j, const FeedbackLogRequest &request) {
        j = nlohmann::json{
            {"clientInfo", request.clientInfo},
            {"timestamp", request.timestamp},
            {"logContent", request.logContent},
            {"preferences", request.preferences}};
    }
    inline void from_json(const nlohmann::json &j, FeedbackLogRequest &request) {
        from_json(j.at("clientInfo"), request.clientInfo);
        request.timestamp = j.value("timestamp", 0);
        request.logContent = j.value("logContent", "");
        from_json(j.at("preferences"), request.preferences);
    }

    // WebSocketServerSide::NotifyChanged
    inline void to_json(nlohmann::json &j, const WebSocketServerSide::NotifyChanged &notifyChanged) {
        j = nlohmann::json{
            {"type", notifyChanged.type},
            {"message", notifyChanged.message},
            {"clientInfo", notifyChanged.clientInfo}};
    }
    inline void from_json(const nlohmann::json &j, WebSocketServerSide::NotifyChanged &notifyChanged) {
        notifyChanged.type = j.value("type", "");
        notifyChanged.message = j.value("message", "");
        from_json(j.at("clientInfo"), notifyChanged.clientInfo);
    }

    // WebSocketServerSide
    inline void to_json(nlohmann::json &j, const WebSocketServerSide &ws) {
        j = nlohmann::json{
            {"action", ws.action},
            {"messageId", ws.messageId},
            {"errors", ws.errors},
            {"meta", ws.meta},
            {"notifyChanged", ws.notifyChanged}};
    }
    inline void from_json(const nlohmann::json &j, WebSocketServerSide &ws) {
        ws.action = j.value("action", "");
        ws.messageId = j.value("messageId", "");
        ws.errors = j.value("errors", std::vector<Error>{});
        from_json(j.at("meta"), ws.meta);
        if (j.contains("notifyChanged")) {
            from_json(j.at("notifyChanged"), ws.notifyChanged);
        } else {
            ws.notifyChanged = WebSocketServerSide::NotifyChanged{};
        }
    }

    // WebSocketClientSide
    inline void to_json(nlohmann::json &j, const WebSocketClientSide &ws) {
        j = nlohmann::json{
            {"action", ws.action},
            {"accessToken", ws.accessToken},
            {"lastMessageId", ws.lastMessageId},
            {"clientInfo", ws.clientInfo},
            {"preferences", ws.preferences}};
    }
    inline void from_json(const nlohmann::json &j, WebSocketClientSide &ws) {
        ws.action = j.value("action", "");
        ws.accessToken = j.value("accessToken", "");
        ws.lastMessageId = j.value("lastMessageId", "");
        from_json(j.at("clientInfo"), ws.clientInfo);
        from_json(j.at("preferences"), ws.preferences);
    }

    // StaticLauncherConfig
    inline void to_json(nlohmann::json &j, const StaticConfig::StaticLauncherConfig &config) {
        nlohmann::json checkUpdateUrlsJson = nlohmann::json::array();
        for (const auto &url : config.checkUpdateUrls) {
            checkUpdateUrlsJson.push_back(
                {{"system", url.system},
                 {"url", url.url}});
        }
        j = nlohmann::json{
            {"host", config.host},
            {"retryIntervalSec", config.retryIntervalSec},
            {"maxRetryCount", config.maxRetryCount},
            {"webSocket", config.webSocket},
            {"security", config.security},
            {"meta", config.meta},
            {"checkUpdateUrls", checkUpdateUrlsJson}};
    }
    inline void from_json(const nlohmann::json &j, StaticConfig::StaticLauncherConfig &config) {
        config.host = j.value("host", std::vector<std::string>{});
        config.retryIntervalSec = j.value("retryIntervalSec", 0);
        config.maxRetryCount = j.value("maxRetryCount", 0);
        from_json(j.at("webSocket"), config.webSocket);
        from_json(j.at("security"), config.security);
        from_json(j.at("meta"), config.meta);

        if (j.contains("checkUpdateUrls")) {
            for (const auto &url : j.at("checkUpdateUrls")) {
                StaticConfig::StaticLauncherConfig::CheckUpdateUrls checkUrl;
                from_json(url.at("system"), checkUrl.system);
                checkUrl.url = url.value("url", "");
                config.checkUpdateUrls.push_back(checkUrl);
            }
        }
    }

    // StaticMaintenanceInfo
    inline void to_json(nlohmann::json &j, const StaticConfig::StaticMaintenanceInfo &info) {
        to_json(j, static_cast<const MaintenanceResponse &>(info));
    }

    inline void from_json(const nlohmann::json &j, StaticConfig::StaticMaintenanceInfo &info) {
        from_json(j, static_cast<MaintenanceResponse &>(info));
    }

    // StaticConfig
    inline void to_json(nlohmann::json &j, const StaticConfig &config) {
        j = nlohmann::json{
            {"staticLauncherConfig", config.staticLauncherConfig},
            {"staticMaintenanceInfo", config.staticMaintenanceInfo}};
    }
    inline void from_json(const nlohmann::json &j, StaticConfig &config) {
        from_json(j.at("staticLauncherConfig"), config.staticLauncherConfig);
        from_json(j.at("staticMaintenanceInfo"), config.staticMaintenanceInfo);
    }

    // StaticUpdateInfo
    inline void to_json(nlohmann::json &j, const StaticUpdateInfo &info) {
        j = nlohmann::json{
            {"coreVersion", info.coreVersion},
            {"resourceVersion", info.resourceVersion},
            {"updateResponse", info.updateResponse}};
    }
    inline void from_json(const nlohmann::json &j, StaticUpdateInfo &info) {
        info.coreVersion = j.value("coreVersion", "");
        info.resourceVersion = j.value("resourceVersion", "");
        from_json(j.at("updateResponse"), info.updateResponse);
    }

} // namespace neko::api