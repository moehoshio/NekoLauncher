#include "neko/app/api.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using namespace neko::api;

class ApiTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Error 結構體測試
TEST_F(ApiTest, ErrorSerialization) {
    Error error{"404", "NotFound", "Resource not found"};
    
    nlohmann::json j;
    to_json(j, error);
    
    EXPECT_EQ(j["error"], "404");
    EXPECT_EQ(j["errorType"], "NotFound");
    EXPECT_EQ(j["errorMessage"], "Resource not found");
    
    Error deserializedError;
    from_json(j, deserializedError);
    
    EXPECT_EQ(deserializedError.error, "404");
    EXPECT_EQ(deserializedError.errorType, "NotFound");
    EXPECT_EQ(deserializedError.errorMessage, "Resource not found");
    EXPECT_FALSE(deserializedError.empty());
}

TEST_F(ApiTest, ErrorEmpty) {
    Error emptyError;
    EXPECT_TRUE(emptyError.empty());
    
    Error nonEmptyError{"", "", "message"};
    EXPECT_FALSE(nonEmptyError.empty());
}

// Meta 結構體測試
TEST_F(ApiTest, MetaSerialization) {
    Meta meta{"1.0", "1.0", "build123", "2024-01-01", "deprecated", 1234567890, true};
    
    nlohmann::json j;
    to_json(j, meta);
    
    EXPECT_EQ(j["apiVersion"], "1.0");
    EXPECT_EQ(j["timestamp"], 1234567890);
    EXPECT_EQ(j["isDeprecated"], true);
    
    Meta deserializedMeta;
    from_json(j, deserializedMeta);
    
    EXPECT_EQ(deserializedMeta.apiVersion, "1.0");
    EXPECT_EQ(deserializedMeta.timestamp, 1234567890);
    EXPECT_EQ(deserializedMeta.isDeprecated, true);
}

// App 結構體測試
TEST_F(ApiTest, AppSerialization) {
    App app{"NekoLauncher", "1.0", "1.0", "build123"};
    
    nlohmann::json j;
    to_json(j, app);
    
    EXPECT_EQ(j["appName"], "NekoLauncher");
    EXPECT_EQ(j["coreVersion"], "1.0");
    
    App deserializedApp;
    from_json(j, deserializedApp);
    
    EXPECT_EQ(deserializedApp.appName, "NekoLauncher");
    EXPECT_EQ(deserializedApp.coreVersion, "1.0");
    EXPECT_FALSE(deserializedApp.empty());
}

// System 結構體測試
TEST_F(ApiTest, SystemSerialization) {
    System system{"Windows", "x64", "10.0.19041"};
    
    nlohmann::json j;
    to_json(j, system);
    
    EXPECT_EQ(j["os"], "Windows");
    EXPECT_EQ(j["arch"], "x64");
    EXPECT_EQ(j["osVersion"], "10.0.19041");
    
    System deserializedSystem;
    from_json(j, deserializedSystem);
    
    EXPECT_EQ(deserializedSystem.os, "Windows");
    EXPECT_EQ(deserializedSystem.arch, "x64");
    EXPECT_EQ(deserializedSystem.osVersion, "10.0.19041");
}

// ClientInfo 結構體測試
TEST_F(ApiTest, ClientInfoSerialization) {
    App app{"NekoLauncher", "1.0", "1.0", "build123"};
    System system{"Windows", "x64", "10.0.19041"};
    Extra extra;
    ClientInfo clientInfo{app, system, extra, "device123"};
    
    nlohmann::json j;
    to_json(j, clientInfo);
    
    EXPECT_EQ(j["deviceId"], "device123");
    EXPECT_TRUE(j.contains("app"));
    EXPECT_TRUE(j.contains("system"));
    
    ClientInfo deserializedClientInfo;
    from_json(j, deserializedClientInfo);
    
    EXPECT_EQ(deserializedClientInfo.deviceId, "device123");
    EXPECT_EQ(deserializedClientInfo.app.appName, "NekoLauncher");
}

// Auth 相關測試
TEST_F(ApiTest, AuthAccountSerialization) {
    Auth::Account account{"user123", "pass123"};
    
    nlohmann::json j;
    to_json(j, account);
    
    EXPECT_EQ(j["username"], "user123");
    EXPECT_EQ(j["password"], "pass123");
    
    Auth::Account deserializedAccount;
    from_json(j, deserializedAccount);
    
    EXPECT_EQ(deserializedAccount.username, "user123");
    EXPECT_EQ(deserializedAccount.password, "pass123");
}

TEST_F(ApiTest, AuthTokenSerialization) {
    Auth::Token token{"access123", "refresh123"};
    
    nlohmann::json j;
    to_json(j, token);
    
    EXPECT_EQ(j["accessToken"], "access123");
    EXPECT_EQ(j["refreshToken"], "refresh123");
    
    Auth::Token deserializedToken;
    from_json(j, deserializedToken);
    
    EXPECT_EQ(deserializedToken.accessToken, "access123");
    EXPECT_EQ(deserializedToken.refreshToken, "refresh123");
}

// LauncherConfigResponse 測試
TEST_F(ApiTest, LauncherConfigResponseSerialization) {
    LauncherConfigResponse config;
    config.host = {"host1.example.com", "host2.example.com"};
    config.retryIntervalSec = 30;
    config.maxRetryCount = 3;
    config.webSocket.enable = true;
    config.webSocket.socketHost = "ws.example.com";
    config.webSocket.heartbeatIntervalSec = 60;
    
    nlohmann::json j;
    to_json(j, config);
    
    EXPECT_EQ(j["retryIntervalSec"], 30);
    EXPECT_EQ(j["maxRetryCount"], 3);
    EXPECT_TRUE(j["webSocket"]["enable"]);
    
    LauncherConfigResponse deserializedConfig;
    from_json(j, deserializedConfig);
    
    EXPECT_EQ(deserializedConfig.retryIntervalSec, 30);
    EXPECT_EQ(deserializedConfig.webSocket.enable, true);
    EXPECT_EQ(deserializedConfig.webSocket.socketHost, "ws.example.com");
}

// MaintenanceResponse 測試
TEST_F(ApiTest, MaintenanceResponseMethods) {
    MaintenanceResponse maintenance;
    
    // 測試進行中的維護
    maintenance.status = "progress";
    EXPECT_TRUE(maintenance.isMaintenance());
    EXPECT_FALSE(maintenance.isScheduled());
    
    // 測試預定的維護
    maintenance.status = "scheduled";
    EXPECT_FALSE(maintenance.isMaintenance());
    EXPECT_TRUE(maintenance.isScheduled());
    
    // 測試完成的維護
    maintenance.status = "completed";
    EXPECT_FALSE(maintenance.isMaintenance());
    EXPECT_FALSE(maintenance.isScheduled());
}

// UpdateResponse 測試
TEST_F(ApiTest, UpdateResponseSerialization) {
    UpdateResponse update;
    update.title = "Update v1.1";
    update.description = "Bug fixes";
    update.isMandatory = true;
    
    UpdateResponse::File file;
    file.url = "http://example.com/file.zip";
    file.fileName = "update.zip";
    file.checksum = "abc123";
    file.hashAlgorithm = "SHA256";
    file.isCoreFile = true;
    
    update.files.push_back(file);
    
    nlohmann::json j;
    to_json(j, update);
    
    EXPECT_EQ(j["title"], "Update v1.1");
    EXPECT_EQ(j["isMandatory"], true);
    EXPECT_EQ(j["files"].size(), 1);
    EXPECT_EQ(j["files"][0]["fileName"], "update.zip");
    
    UpdateResponse deserializedUpdate;
    from_json(j, deserializedUpdate);
    
    EXPECT_EQ(deserializedUpdate.title, "Update v1.1");
    EXPECT_EQ(deserializedUpdate.files.size(), 1);
    EXPECT_EQ(deserializedUpdate.files[0].fileName, "update.zip");
    EXPECT_TRUE(deserializedUpdate.files[0].isCoreFile);
}

// WebSocket 測試
TEST_F(ApiTest, WebSocketServerSideMethods) {
    WebSocketServerSide ws;
    
    // 測試 ping
    ws.action = "ping";
    EXPECT_TRUE(ws.isPing());
    EXPECT_FALSE(ws.isPong());
    EXPECT_FALSE(ws.isNotifyChanged());
    
    // 測試 pong
    ws.action = "pong";
    EXPECT_FALSE(ws.isPing());
    EXPECT_TRUE(ws.isPong());
    
    // 測試 notifyChanged
    ws.action = "notifyChanged";
    ws.notifyChanged.type = "update";
    ws.notifyChanged.message = "New update available";
    EXPECT_TRUE(ws.isNotifyChanged());
    
    // 測試錯誤
    Error error{"500", "ServerError", "Internal error"};
    ws.errors.push_back(error);
    EXPECT_TRUE(ws.hasError());
}

TEST_F(ApiTest, WebSocketClientSideMethods) {
    WebSocketClientSide ws;
    
    ws.action = "ping";
    EXPECT_TRUE(ws.isPing());
    EXPECT_FALSE(ws.isPong());
    EXPECT_FALSE(ws.isQuery());
    
    ws.action = "query";
    EXPECT_TRUE(ws.isQuery());
    EXPECT_FALSE(ws.isPing());
}

// StaticConfig 測試
TEST_F(ApiTest, StaticLauncherConfigGetCheckUpdateUrl) {
    StaticConfig::StaticLauncherConfig config;
    
    StaticConfig::StaticLauncherConfig::CheckUpdateUrls url1;
    url1.system = {"Windows", "x64", "10\\..*"};
    url1.url = "http://windows-x64.example.com/update";
    
    StaticConfig::StaticLauncherConfig::CheckUpdateUrls url2;
    url2.system = {"Linux", "x64", ".*"};
    url2.url = "http://linux-x64.example.com/update";
    
    config.checkUpdateUrls = {url1, url2};
    
    // 測試匹配的情況
    auto result1 = config.getCheckUpdateUrl("Windows", "x64", "10.0.19041");
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "http://windows-x64.example.com/update");
    
    // 測試不匹配的情況
    auto result2 = config.getCheckUpdateUrl("Windows", "x86", "10.0.19041");
    EXPECT_FALSE(result2.has_value());
    
    // 測試 Linux 匹配
    auto result3 = config.getCheckUpdateUrl("Linux", "x64", "Ubuntu 20.04");
    EXPECT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), "http://linux-x64.example.com/update");
}

TEST_F(ApiTest, StaticMaintenanceInfoMethods) {
    StaticConfig::StaticMaintenanceInfo info;
    
    // 測試完成狀態
    info.status = "completed";
    EXPECT_TRUE(info.isCompleted());
    
    // 測試空狀態（也算完成）
    info.status = "";
    EXPECT_TRUE(info.isCompleted());
    
    // 測試進行中狀態
    info.status = "progress";
    EXPECT_FALSE(info.isCompleted());
}

// FeedbackLogRequest 測試
TEST_F(ApiTest, FeedbackLogRequestSerialization) {
    FeedbackLogRequest request;
    request.timestamp = 1234567890;
    request.logContent = "Error occurred in module X";
    
    nlohmann::json j;
    to_json(j, request);
    
    EXPECT_EQ(j["timestamp"], 1234567890);
    EXPECT_EQ(j["logContent"], "Error occurred in module X");
    
    FeedbackLogRequest deserializedRequest;
    from_json(j, deserializedRequest);
    
    EXPECT_EQ(deserializedRequest.timestamp, 1234567890);
    EXPECT_EQ(deserializedRequest.logContent, "Error occurred in module X");
}

// StaticUpdateInfo 測試
TEST_F(ApiTest, StaticUpdateInfoSerialization) {
    StaticUpdateInfo info;
    info.coreVersion = "1.0";
    info.resourceVersion = "1.1";
    info.updateResponse.title = "Update available";
    
    nlohmann::json j;
    to_json(j, info);
    
    EXPECT_EQ(j["coreVersion"], "1.0");
    EXPECT_EQ(j["resourceVersion"], "1.1");
    EXPECT_EQ(j["updateResponse"]["title"], "Update available");
    
    StaticUpdateInfo deserializedInfo;
    from_json(j, deserializedInfo);
    
    EXPECT_EQ(deserializedInfo.coreVersion, "1.0");
    EXPECT_EQ(deserializedInfo.resourceVersion, "1.1");
    EXPECT_EQ(deserializedInfo.updateResponse.title, "Update available");
}

// 邊界情況和錯誤處理測試
TEST_F(ApiTest, JsonDeserializationWithMissingFields) {
    // 測試缺少字段時的反序列化
    nlohmann::json j = {{"error", "404"}};
    
    Error error;
    from_json(j, error);
    
    EXPECT_EQ(error.error, "404");
    EXPECT_EQ(error.errorType, "");
    EXPECT_EQ(error.errorMessage, "");
}

TEST_F(ApiTest, EmptyObjectsSerialization) {
    Extra extra;
    nlohmann::json j;
    to_json(j, extra);
    
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
    
    Extra deserializedExtra;
    from_json(j, deserializedExtra);
    EXPECT_TRUE(deserializedExtra.empty());
}