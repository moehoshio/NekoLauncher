/**
 * @file neko_lc_schema_test.cpp
 * @brief Unit tests for neko api modules (API types, client config, event types)
 */

#include <gtest/gtest.h>
#include <neko/app/api.hpp>
#include <neko/app/clientConfig.hpp>
#include <neko/app/eventTypes.hpp>
#include <nlohmann/json.hpp>
#include <SimpleIni.h>

#include <filesystem>
#include <fstream>
#include <sstream>

// ============================================================================
// Error Schema Tests
// ============================================================================

class ErrorSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ErrorSchemaTest, ErrorToJson) {
    neko::api::Error error;
    error.error = "ERR001";
    error.errorType = "NetworkError";
    error.errorMessage = "Connection timeout";

    nlohmann::json j = error;

    EXPECT_EQ(j["error"], "ERR001");
    EXPECT_EQ(j["errorType"], "NetworkError");
    EXPECT_EQ(j["errorMessage"], "Connection timeout");
}

TEST_F(ErrorSchemaTest, ErrorFromJson) {
    nlohmann::json j = {
        {"error", "ERR002"},
        {"errorType", "ValidationError"},
        {"errorMessage", "Invalid input"}
    };

    neko::api::Error error = j.get<neko::api::Error>();

    EXPECT_EQ(error.error, "ERR002");
    EXPECT_EQ(error.errorType, "ValidationError");
    EXPECT_EQ(error.errorMessage, "Invalid input");
}

TEST_F(ErrorSchemaTest, ErrorEmpty) {
    neko::api::Error error;
    EXPECT_TRUE(error.empty());

    error.error = "ERR001";
    EXPECT_FALSE(error.empty());
}

// ============================================================================
// Meta Schema Tests
// ============================================================================

class MetaSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(MetaSchemaTest, MetaToJson) {
    neko::api::Meta meta;
    meta.apiVersion = "1.0.0";
    meta.minApiVersion = "0.9.0";
    meta.buildVersion = "build-123";
    meta.releaseDate = "2025-10-15";
    meta.timestamp = 1697356800;
    meta.isDeprecated = false;

    nlohmann::json j = meta;

    EXPECT_EQ(j["apiVersion"], "1.0.0");
    EXPECT_EQ(j["minApiVersion"], "0.9.0");
    EXPECT_EQ(j["buildVersion"], "build-123");
    EXPECT_EQ(j["releaseDate"], "2025-10-15");
    EXPECT_EQ(j["timestamp"], 1697356800);
    EXPECT_EQ(j["isDeprecated"], false);
}

TEST_F(MetaSchemaTest, MetaFromJson) {
    nlohmann::json j = {
        {"apiVersion", "2.0.0"},
        {"minApiVersion", "1.5.0"},
        {"buildVersion", "build-456"},
        {"releaseDate", "2025-11-01"},
        {"timestamp", 1698566400},
        {"isDeprecated", true},
        {"deprecatedMessage", "Please upgrade"}
    };

    neko::api::Meta meta = j.get<neko::api::Meta>();

    EXPECT_EQ(meta.apiVersion, "2.0.0");
    EXPECT_EQ(meta.minApiVersion, "1.5.0");
    EXPECT_EQ(meta.buildVersion, "build-456");
    EXPECT_EQ(meta.releaseDate, "2025-11-01");
    EXPECT_EQ(meta.timestamp, 1698566400);
    EXPECT_EQ(meta.isDeprecated, true);
    EXPECT_EQ(meta.deprecatedMessage, "Please upgrade");
}

// ============================================================================
// ClientInfo Schema Tests
// ============================================================================

class ClientInfoSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ClientInfoSchemaTest, AppToJson) {
    neko::api::App app;
    app.appName = "NekoLauncher";
    app.coreVersion = "1.0.0";
    app.resourceVersion = "1.0.0";
    app.buildId = "build-123";

    nlohmann::json j = app;

    EXPECT_EQ(j["appName"], "NekoLauncher");
    EXPECT_EQ(j["coreVersion"], "1.0.0");
    EXPECT_EQ(j["resourceVersion"], "1.0.0");
    EXPECT_EQ(j["buildId"], "build-123");
}

TEST_F(ClientInfoSchemaTest, SystemToJson) {
    neko::api::System system;
    system.os = "Windows";
    system.arch = "x64";
    system.osVersion = "10.0.19045";

    nlohmann::json j = system;

    EXPECT_EQ(j["os"], "Windows");
    EXPECT_EQ(j["arch"], "x64");
    EXPECT_EQ(j["osVersion"], "10.0.19045");
}

TEST_F(ClientInfoSchemaTest, ClientInfoToJson) {
    neko::api::ClientInfo clientInfo;
    clientInfo.app.appName = "NekoLauncher";
    clientInfo.app.coreVersion = "1.0.0";
    clientInfo.system.os = "Windows";
    clientInfo.system.arch = "x64";
    clientInfo.deviceId = "device-123";

    nlohmann::json j = clientInfo;

    EXPECT_EQ(j["app"]["appName"], "NekoLauncher");
    EXPECT_EQ(j["system"]["os"], "Windows");
    EXPECT_EQ(j["deviceId"], "device-123");
}

TEST_F(ClientInfoSchemaTest, ClientInfoFromJson) {
    nlohmann::json j = {
        {"app", {
            {"appName", "TestApp"},
            {"coreVersion", "2.0.0"},
            {"resourceVersion", "2.0.0"},
            {"buildId", "test-build"}
        }},
        {"system", {
            {"os", "Linux"},
            {"arch", "arm64"},
            {"osVersion", "5.15.0"}
        }},
        {"extra", nlohmann::json::object()},
        {"deviceId", "device-456"}
    };

    neko::api::ClientInfo clientInfo = j.get<neko::api::ClientInfo>();

    EXPECT_EQ(clientInfo.app.appName, "TestApp");
    EXPECT_EQ(clientInfo.app.coreVersion, "2.0.0");
    EXPECT_EQ(clientInfo.system.os, "Linux");
    EXPECT_EQ(clientInfo.system.arch, "arm64");
    EXPECT_EQ(clientInfo.deviceId, "device-456");
}

// ============================================================================
// Auth Schema Tests
// ============================================================================

class AuthSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(AuthSchemaTest, AccountToJson) {
    neko::api::Auth::Account account;
    account.username = "testuser";
    account.password = "testpass";

    nlohmann::json j = account;

    EXPECT_EQ(j["username"], "testuser");
    EXPECT_EQ(j["password"], "testpass");
}

TEST_F(AuthSchemaTest, TokenToJson) {
    neko::api::Auth::Token token;
    token.accessToken = "access123";
    token.refreshToken = "refresh456";

    nlohmann::json j = token;

    EXPECT_EQ(j["accessToken"], "access123");
    EXPECT_EQ(j["refreshToken"], "refresh456");
}

TEST_F(AuthSchemaTest, AuthFromJson) {
    nlohmann::json j = {
        {"account", {
            {"username", "user1"},
            {"password", "pass1"}
        }},
        {"signature", {
            {"identifier", "id123"},
            {"timestamp", 1697356800},
            {"signature", "sig123"}
        }},
        {"token", {
            {"accessToken", "token1"},
            {"refreshToken", "refresh1"}
        }},
        {"preferences", {
            {"language", "en"}
        }}
    };

    neko::api::Auth auth = j.get<neko::api::Auth>();

    EXPECT_EQ(auth.account.username, "user1");
    EXPECT_EQ(auth.account.password, "pass1");
    EXPECT_EQ(auth.token.accessToken, "token1");
    EXPECT_EQ(auth.token.refreshToken, "refresh1");
    EXPECT_EQ(auth.preferences.language, "en");
}

// ============================================================================
// LauncherConfigResponse Schema Tests
// ============================================================================

class LauncherConfigSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(LauncherConfigSchemaTest, WebSocketToJson) {
    neko::api::LauncherConfigResponse::WebSocket ws;
    ws.enable = true;
    ws.socketHost = "wss://example.com";
    ws.heartbeatIntervalSec = 30;

    nlohmann::json j = ws;

    EXPECT_EQ(j["enable"], true);
    EXPECT_EQ(j["socketHost"], "wss://example.com");
    EXPECT_EQ(j["heartbeatIntervalSec"], 30);
}

TEST_F(LauncherConfigSchemaTest, SecurityToJson) {
    neko::api::LauncherConfigResponse::Security security;
    security.enableAuthentication = true;
    security.tokenExpirationSec = 3600;
    security.refreshTokenExpirationDays = 30;
    security.loginUrl = "https://example.com/login";
    security.logoutUrl = "https://example.com/logout";
    security.refreshTokenUrl = "https://example.com/refresh";

    nlohmann::json j = security;

    EXPECT_EQ(j["enableAuthentication"], true);
    EXPECT_EQ(j["tokenExpirationSec"], 3600);
    EXPECT_EQ(j["refreshTokenExpirationDays"], 30);
    EXPECT_EQ(j["loginUrl"], "https://example.com/login");
}

TEST_F(LauncherConfigSchemaTest, LauncherConfigFromJson) {
    nlohmann::json j = {
        {"host", {"https://server1.com", "https://server2.com"}},
        {"retryIntervalSec", 5},
        {"maxRetryCount", 3},
        {"webSocket", {
            {"enable", true},
            {"socketHost", "wss://ws.example.com"},
            {"heartbeatIntervalSec", 60}
        }},
        {"security", {
            {"enableAuthentication", true},
            {"tokenExpirationSec", 7200},
            {"refreshTokenExpirationDays", 7},
            {"loginUrl", "https://api.example.com/login"},
            {"logoutUrl", "https://api.example.com/logout"},
            {"refreshTokenUrl", "https://api.example.com/refresh"}
        }},
        {"meta", {
            {"apiVersion", "1.0.0"},
            {"minApiVersion", "1.0.0"},
            {"buildVersion", "build-1"},
            {"releaseDate", "2025-10-15"},
            {"timestamp", 1697356800},
            {"isDeprecated", false},
            {"deprecatedMessage", ""}
        }}
    };

    neko::api::LauncherConfigResponse config = j.get<neko::api::LauncherConfigResponse>();

    EXPECT_EQ(config.host.size(), 2);
    EXPECT_EQ(config.host[0], "https://server1.com");
    EXPECT_EQ(config.retryIntervalSec, 5);
    EXPECT_EQ(config.maxRetryCount, 3);
    EXPECT_EQ(config.webSocket.enable, true);
    EXPECT_EQ(config.webSocket.socketHost, "wss://ws.example.com");
    EXPECT_EQ(config.security.enableAuthentication, true);
    EXPECT_EQ(config.security.tokenExpirationSec, 7200);
}

// ============================================================================
// MaintenanceResponse Schema Tests
// ============================================================================

class MaintenanceSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(MaintenanceSchemaTest, MaintenanceFromJson) {
    nlohmann::json j = {
        {"status", "progress"},
        {"message", "Server is under maintenance"},
        {"startTime", "2025-10-15T00:00:00Z"},
        {"exEndTime", "2025-10-15T04:00:00Z"},
        {"posterUrl", "https://example.com/poster.jpg"},
        {"link", "https://example.com/status"},
        {"meta", {
            {"apiVersion", "1.0.0"},
            {"minApiVersion", "1.0.0"},
            {"buildVersion", "build-1"},
            {"releaseDate", "2025-10-15"},
            {"timestamp", 1697356800},
            {"isDeprecated", false},
            {"deprecatedMessage", ""}
        }}
    };

    neko::api::MaintenanceResponse maintenance = j.get<neko::api::MaintenanceResponse>();

    EXPECT_EQ(maintenance.status, "progress");
    EXPECT_EQ(maintenance.message, "Server is under maintenance");
    EXPECT_EQ(maintenance.startTime, "2025-10-15T00:00:00Z");
    EXPECT_EQ(maintenance.exEndTime, "2025-10-15T04:00:00Z");
    EXPECT_TRUE(maintenance.isMaintenance());
}

TEST_F(MaintenanceSchemaTest, MaintenanceStatus) {
    neko::api::MaintenanceResponse maintenance;

    maintenance.status = "progress";
    EXPECT_TRUE(maintenance.isMaintenance());

    maintenance.status = "scheduled";
    EXPECT_TRUE(maintenance.isScheduled());

    maintenance.status = "normal";
    EXPECT_FALSE(maintenance.isMaintenance());
    EXPECT_FALSE(maintenance.isScheduled());
}

// ============================================================================
// UpdateResponse Schema Tests
// ============================================================================

class UpdateSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UpdateSchemaTest, UpdateFileToJson) {
    neko::api::UpdateResponse::File file;
    file.url = "https://example.com/file.zip";
    file.fileName = "update.zip";
    file.checksum = "abc123";
    file.hashAlgorithm = "SHA256";
    file.suggestMultiThread = true;
    file.isCoreFile = true;
    file.isAbsoluteUrl = true;

    nlohmann::json j = file;

    EXPECT_EQ(j["url"], "https://example.com/file.zip");
    EXPECT_EQ(j["fileName"], "update.zip");
    EXPECT_EQ(j["checksum"], "abc123");
    EXPECT_EQ(j["hashAlgorithm"], "SHA256");
    EXPECT_EQ(j["suggestMultiThread"], true);
    EXPECT_EQ(j["isCoreFile"], true);
    EXPECT_EQ(j["isAbsoluteUrl"], true);
}

TEST_F(UpdateSchemaTest, UpdateResponseFromJson) {
    nlohmann::json j = {
        {"title", "New Update Available"},
        {"description", "Bug fixes and improvements"},
        {"posterUrl", "https://example.com/poster.jpg"},
        {"publishTime", "2025-10-15T12:00:00Z"},
        {"resourceVersion", "1.1.0"},
        {"isMandatory", true},
        {"meta", {
            {"apiVersion", "1.0.0"},
            {"minApiVersion", "1.0.0"},
            {"buildVersion", "build-1"},
            {"releaseDate", "2025-10-15"},
            {"timestamp", 1697356800},
            {"isDeprecated", false},
            {"deprecatedMessage", ""}
        }},
        {"files", {
            {
                {"url", "file1.zip"},
                {"fileName", "core.zip"},
                {"checksum", "hash1"},
                {"hashAlgorithm", "SHA256"},
                {"suggestMultiThread", true},
                {"isCoreFile", true},
                {"isAbsoluteUrl", false}
            },
            {
                {"url", "file2.zip"},
                {"fileName", "resource.zip"},
                {"checksum", "hash2"},
                {"hashAlgorithm", "SHA256"},
                {"suggestMultiThread", false},
                {"isCoreFile", false},
                {"isAbsoluteUrl", false}
            }
        }}
    };

    neko::api::UpdateResponse update = j.get<neko::api::UpdateResponse>();

    EXPECT_EQ(update.title, "New Update Available");
    EXPECT_EQ(update.description, "Bug fixes and improvements");
    EXPECT_EQ(update.resourceVersion, "1.1.0");
    EXPECT_EQ(update.isMandatory, true);
    EXPECT_EQ(update.files.size(), 2);
    EXPECT_EQ(update.files[0].fileName, "core.zip");
    EXPECT_EQ(update.files[0].isCoreFile, true);
    EXPECT_EQ(update.files[1].fileName, "resource.zip");
    EXPECT_EQ(update.files[1].isCoreFile, false);
}

// ============================================================================
// WebSocket Schema Tests
// ============================================================================

class WebSocketSchemaTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(WebSocketSchemaTest, WebSocketClientSideToJson) {
    neko::api::WebSocketClientSide ws;
    ws.action = "ping";
    ws.accessToken = "token123";
    ws.lastMessageId = "msg123";

    nlohmann::json j = ws;

    EXPECT_EQ(j["action"], "ping");
    EXPECT_EQ(j["accessToken"], "token123");
    EXPECT_EQ(j["lastMessageId"], "msg123");
}

TEST_F(WebSocketSchemaTest, WebSocketClientSideActions) {
    neko::api::WebSocketClientSide ws;

    ws.action = "ping";
    EXPECT_TRUE(ws.isPing());
    EXPECT_FALSE(ws.isPong());
    EXPECT_FALSE(ws.isQuery());

    ws.action = "pong";
    EXPECT_FALSE(ws.isPing());
    EXPECT_TRUE(ws.isPong());
    EXPECT_FALSE(ws.isQuery());

    ws.action = "query";
    EXPECT_FALSE(ws.isPing());
    EXPECT_FALSE(ws.isPong());
    EXPECT_TRUE(ws.isQuery());
}

TEST_F(WebSocketSchemaTest, WebSocketServerSideActions) {
    neko::api::WebSocketServerSide ws;

    ws.action = "ping";
    EXPECT_TRUE(ws.isPing());
    EXPECT_FALSE(ws.isPong());

    ws.action = "pong";
    EXPECT_FALSE(ws.isPing());
    EXPECT_TRUE(ws.isPong());

    ws.action = "notifyChanged";
    ws.notifyChanged.type = "update";
    ws.notifyChanged.message = "New update available";
    EXPECT_TRUE(ws.isNotifyChanged());
}

TEST_F(WebSocketSchemaTest, WebSocketServerSideHasError) {
    neko::api::WebSocketServerSide ws;

    EXPECT_FALSE(ws.hasError());

    neko::api::Error error;
    error.error = "ERR001";
    ws.errors.push_back(error);

    EXPECT_TRUE(ws.hasError());
}

// ============================================================================
// ClientConfig Tests
// ============================================================================

class ClientConfigTest : public ::testing::Test {
protected:
    std::string testConfigPath;

    void SetUp() override {
        testConfigPath = (std::filesystem::temp_directory_path() / "test_config.ini").string();
    }

    void TearDown() override {
        if (std::filesystem::exists(testConfigPath)) {
            std::filesystem::remove(testConfigPath);
        }
    }

    void createTestConfigFile(const std::string& content) {
        std::ofstream file(testConfigPath);
        file << content;
        file.close();
    }
};

TEST_F(ClientConfigTest, LoadFromIni) {
    createTestConfigFile(R"(
[main]
language=zh_tw
backgroundType=video
background=bg.mp4
windowSize=1920x1080
launcherMethod=2
useSystemWindowFrame=false
headBarKeepRight=true
deviceID=test-device-123

[style]
blurEffect=Gaussian
blurRadius=15
fontPointSize=12
fontFamilies=Arial,Microsoft YaHei

[net]
thread=8
proxy=http://proxy.example.com:8080

[dev]
enable=true
debug=true
server=https://dev.example.com
tls=false

[other]
customTempDir=/tmp/neko
resourceVersion=1.0.0

[minecraft]
minecraftFolder=./.minecraft
javaPath=/usr/bin/java
downloadSource=BMCLAPI
playerName=TestPlayer
account=test@example.com
uuid=12345678-1234-1234-1234-123456789abc
accessToken=token123
targetVersion=1.20.1
maxMemoryLimit=4096
minMemoryLimit=2048
needMemoryLimit=2048
authlibName=authlib-injector.jar
authlibPrefetched=https://example.com/authlib
authlibSha256=abc123
tolerantMode=true
customResolution=1920x1080
joinServerAddress=play.example.com
joinServerPort=25565
)");

    CSimpleIniA ini;
    ini.SetUnicode();
    SI_Error rc = ini.LoadFile(testConfigPath.c_str());
    ASSERT_EQ(rc, SI_OK);

    neko::ClientConfig config(ini);

    EXPECT_STREQ(config.main.lang, "zh_tw");
    EXPECT_STREQ(config.main.backgroundType, "video");
    EXPECT_STREQ(config.main.background, "bg.mp4");
    EXPECT_STREQ(config.main.windowSize, "1920x1080");
    EXPECT_EQ(config.main.launcherMethod, 2);
    EXPECT_EQ(config.main.useSysWindowFrame, false);
    EXPECT_EQ(config.main.headBarKeepRight, true);
    EXPECT_STREQ(config.main.deviceID, "test-device-123");

    EXPECT_STREQ(config.style.blurEffect, "Gaussian");
    EXPECT_EQ(config.style.blurRadius, 15);
    EXPECT_EQ(config.style.fontPointSize, 12);

    EXPECT_EQ(config.net.thread, 8);
    EXPECT_STREQ(config.net.proxy, "http://proxy.example.com:8080");

    EXPECT_EQ(config.dev.enable, true);
    EXPECT_EQ(config.dev.debug, true);
    EXPECT_STREQ(config.dev.server, "https://dev.example.com");
    EXPECT_EQ(config.dev.tls, false);

    EXPECT_STREQ(config.minecraft.playerName, "TestPlayer");
    EXPECT_STREQ(config.minecraft.targetVersion, "1.20.1");
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 4096);
    EXPECT_EQ(config.minecraft.tolerantMode, true);
}

TEST_F(ClientConfigTest, SaveToIni) {
    // Create a config by loading first, then modifying
    CSimpleIniA iniForInit;
    iniForInit.SetUnicode();
    iniForInit.SetValue("main", "language", "zh_cn");
    iniForInit.SetValue("main", "backgroundType", "color");
    
    neko::ClientConfig config(iniForInit);
    
    // Now use setToConfig
    CSimpleIniA ini;
    ini.SetUnicode();
    config.setToConfig(ini);

    SI_Error rc = ini.SaveFile(testConfigPath.c_str());
    ASSERT_EQ(rc, SI_OK);

    // Reload and verify
    CSimpleIniA loadedIni;
    loadedIni.SetUnicode();
    rc = loadedIni.LoadFile(testConfigPath.c_str());
    ASSERT_EQ(rc, SI_OK);

    EXPECT_STREQ(loadedIni.GetValue("main", "language", ""), "zh_cn");
    EXPECT_STREQ(loadedIni.GetValue("main", "backgroundType", ""), "color");
}

TEST_F(ClientConfigTest, DefaultValues) {
    CSimpleIniA ini;
    neko::ClientConfig config(ini);

    EXPECT_STREQ(config.main.lang, "en");
    EXPECT_STREQ(config.main.backgroundType, "image");
    EXPECT_EQ(config.main.useSysWindowFrame, true);
    EXPECT_EQ(config.style.blurRadius, 10);
    EXPECT_EQ(config.net.thread, 0);
    EXPECT_EQ(config.dev.enable, false);
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 2048);
    EXPECT_EQ(config.minecraft.minMemoryLimit, 1024);
    EXPECT_STREQ(config.minecraft.joinServerPort, "25565");
}

// ============================================================================
// Event Types Tests
// ============================================================================

class EventTypesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(EventTypesTest, StartEvent) {
    neko::event::StartEvent event;
    // Just verify it compiles and can be instantiated
    SUCCEED();
}

TEST_F(EventTypesTest, ExitEvent) {
    neko::event::ExitEvent event;
    // Just verify it compiles and can be instantiated
    SUCCEED();
}

TEST_F(EventTypesTest, ShowHintEvent) {
    neko::ui::HintMsg hint;
    hint.title = "Test Title";
    hint.message = "Test Message";

    neko::event::ShowHintEvent event(hint);
    EXPECT_EQ(event.title, "Test Title");
    EXPECT_EQ(event.message, "Test Message");
}

TEST_F(EventTypesTest, ShowLoadEvent) {
    neko::ui::LoadMsg load;
    load.h1 = "Loading";
    load.message = "Please wait";
    load.process = "Initializing";

    neko::event::ShowLoadEvent event(load);
    EXPECT_EQ(event.h1, "Loading");
    EXPECT_EQ(event.message, "Please wait");
    EXPECT_EQ(event.process, "Initializing");
}

TEST_F(EventTypesTest, UpdateLoadingValEvent) {
    neko::event::UpdateLoadingValEvent event;
    event.progressVal = 50;

    EXPECT_EQ(event.progressVal, 50);
}

TEST_F(EventTypesTest, UpdateLoadingNowEvent) {
    neko::event::UpdateLoadingNowEvent event;
    event.process = "Downloading files";

    EXPECT_EQ(event.process, "Downloading files");
}

TEST_F(EventTypesTest, UpdateLoadingEvent) {
    neko::event::UpdateLoadingEvent event("Initializing", 75);

    EXPECT_EQ(event.process, "Initializing");
    EXPECT_EQ(event.progressVal, 75);
}

TEST_F(EventTypesTest, ShowInputEvent) {
    neko::ui::InputMsg input;
    input.title = "Input Required";
    input.message = "Enter your name";

    neko::event::ShowInputEvent event(input);
    EXPECT_EQ(event.title, "Input Required");
    EXPECT_EQ(event.message, "Enter your name");
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
