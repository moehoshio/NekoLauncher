#include "neko/schema/api.hpp"
#include "library/nlohmann/json.hpp"
#include <cassert>
#include <iostream>

using namespace neko::schema;

void test_to_json_Error() {
    Error e{"err", "type", "msg"};
    nlohmann::json j;
    to_json(j, e);
    assert(j["error"] == "err");
    assert(j["errorType"] == "type");
    assert(j["errorMessage"] == "msg");
    std::cout << "Error: " << j.dump() << std::endl;
}

void test_to_json_Meta() {
    Meta m{"1.0", "0.9", "build42", "2024-06-01", "deprecated soon", 123456789, true};
    nlohmann::json j;
    to_json(j, m);
    assert(j["apiVersion"] == "1.0");
    assert(j["minApiVersion"] == "0.9");
    assert(j["buildVersion"] == "build42");
    assert(j["releaseDate"] == "2024-06-01");
    assert(j["deprecatedMessage"] == "deprecated soon");
    assert(j["timestamp"] == 123456789);
    assert(j["isDeprecated"] == true);
    std::cout << "Meta: " << j.dump() << std::endl;
}

void test_to_json_Preferences() {
    Preferences p{"zh-TW"};
    nlohmann::json j;
    to_json(j, p);
    assert(j["language"] == "zh-TW");
    std::cout << "Preferences: " << j.dump() << std::endl;
}

void test_to_json_App() {
    App a{"Neko", "1.2.3", "4.5.6", "build123"};
    nlohmann::json j;
    to_json(j, a);
    assert(j["appName"] == "Neko");
    assert(j["coreVersion"] == "1.2.3");
    assert(j["resourceVersion"] == "4.5.6");
    assert(j["buildId"] == "build123");
    std::cout << "App: " << j.dump() << std::endl;
}

void test_to_json_System() {
    System s{"Windows", "x64", "10.0.19045"};
    nlohmann::json j;
    to_json(j, s);
    assert(j["os"] == "Windows");
    assert(j["arch"] == "x64");
    assert(j["osVersion"] == "10.0.19045");
    std::cout << "System: " << j.dump() << std::endl;
}

void test_to_json_Extra() {
    Extra e;
    nlohmann::json j;
    to_json(j, e);
    assert(j.is_object());
    std::cout << "Extra: " << j.dump() << std::endl;
}

void test_to_json_ClientInfo() {
    ClientInfo ci;
    ci.app = App{"Neko", "1.2.3", "4.5.6", "build123"};
    ci.system = System{"Windows", "x64", "10.0.19045"};
    ci.extra = Extra{};
    ci.deviceId = "device-xyz";
    nlohmann::json j;
    to_json(j, ci);
    assert(j["app"]["appName"] == "Neko");
    assert(j["system"]["os"] == "Windows");
    assert(j["deviceId"] == "device-xyz");
    std::cout << "ClientInfo: " << j.dump() << std::endl;
}

void test_to_json_Auth() {
    Auth auth;
    auth.account = Auth::Account{"user", "pass"};
    auth.signature = Auth::Signature{"id", "123456", "sig"};
    auth.token = Auth::Token{"tokenA", "tokenR"};
    auth.preferences = Preferences{"en"};
    nlohmann::json j;
    to_json(j, auth);
    assert(j["account"]["username"] == "user");
    assert(j["signature"]["identifier"] == "id");
    assert(j["token"]["accessToken"] == "tokenA");
    assert(j["preferences"]["language"] == "en");
    std::cout << "Auth: " << j.dump() << std::endl;
}

void test_to_json_LauncherConfigResponse() {
    LauncherConfigResponse lcr;
    lcr.host = {"host1", "host2"};
    lcr.retryIntervalSec = 10;
    lcr.maxRetryCount = 5;
    lcr.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    lcr.webSocket = LauncherConfigResponse::WebSocket{true, "ws://host", 30};
    lcr.security = LauncherConfigResponse::Security{true, 3600, 7, "login", "logout", "refresh"};
    nlohmann::json j;
    to_json(j, lcr);
    assert(j["host"].size() == 2);
    assert(j["webSocket"]["enable"] == true);
    assert(j["security"]["enableAuthentication"] == true);
    std::cout << "LauncherConfigResponse: " << j.dump() << std::endl;
}

void test_to_json_MaintenanceResponse() {
    MaintenanceResponse mr;
    mr.status = "progress";
    mr.message = "Maintaining";
    mr.startTime = "2024-06-01T00:00:00Z";
    mr.exEndTime = "2024-06-01T02:00:00Z";
    mr.posterUrl = "http://poster";
    mr.link = "http://link";
    mr.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    nlohmann::json j;
    to_json(j, mr);
    assert(j["status"] == "progress");
    assert(j["meta"]["apiVersion"] == "1.0");
    std::cout << "MaintenanceResponse: " << j.dump() << std::endl;
}

void test_to_json_UpdateResponse_File() {
    UpdateResponse::File f;
    f.url = "http://file";
    f.fileName = "file.zip";
    f.checksum = "abc123";
    f.hashAlgorithm = "sha256";
    f.suggestMultiThread = true;
    f.isCoreFile = false;
    f.isAbsoluteUrl = true;
    nlohmann::json j;
    to_json(j, f);
    assert(j["url"] == "http://file");
    assert(j["fileName"] == "file.zip");
    assert(j["suggestMultiThread"] == true);
    std::cout << "UpdateResponse::File: " << j.dump() << std::endl;
}

void test_to_json_UpdateResponse() {
    UpdateResponse ur;
    ur.title = "Update";
    ur.description = "Desc";
    ur.posterUrl = "http://poster";
    ur.publishTime = "2024-06-01T00:00:00Z";
    ur.resourceVersion = "1.2.3";
    ur.isMandatory = true;
    ur.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    ur.files = {UpdateResponse::File{"http://file", "file.zip", "abc123", "sha256", true, false, true}};
    nlohmann::json j;
    to_json(j, ur);
    assert(j["title"] == "Update");
    assert(j["files"].size() == 1);
    std::cout << "UpdateResponse: " << j.dump() << std::endl;
}

void test_to_json_FeedbackLogRequest() {
    FeedbackLogRequest flr;
    flr.clientInfo = ClientInfo{App{"Neko", "1.2.3", "4.5.6", "build123"}, System{"Windows", "x64", "10.0.19045"}, Extra{}, "device-xyz"};
    flr.timestamp = 123456789;
    flr.logContent = "Log content";
    flr.preferences = Preferences{"en"};
    nlohmann::json j;
    to_json(j, flr);
    assert(j["clientInfo"]["deviceId"] == "device-xyz");
    assert(j["timestamp"] == 123456789);
    std::cout << "FeedbackLogRequest: " << j.dump() << std::endl;
}

void test_to_json_WebSocketServerSide_NotifyChanged() {
    WebSocketServerSide::NotifyChanged nc;
    nc.type = "typeA";
    nc.message = "msg";
    nc.clientInfo = ClientInfo{App{"Neko", "1.2.3", "4.5.6", "build123"}, System{"Windows", "x64", "10.0.19045"}, Extra{}, "device-xyz"};
    nlohmann::json j;
    to_json(j, nc);
    assert(j["type"] == "typeA");
    assert(j["clientInfo"]["deviceId"] == "device-xyz");
    std::cout << "WebSocketServerSide::NotifyChanged: " << j.dump() << std::endl;
}

void test_to_json_WebSocketServerSide() {
    WebSocketServerSide ws;
    ws.action = "ping";
    ws.messageId = "msgid";
    ws.errors = {Error{"err", "type", "msg"}};
    ws.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    ws.notifyChanged = WebSocketServerSide::NotifyChanged{"typeA", "msg", ClientInfo{App{"Neko", "1.2.3", "4.5.6", "build123"}, System{"Windows", "x64", "10.0.19045"}, Extra{}, "device-xyz"}};
    nlohmann::json j;
    to_json(j, ws);
    assert(j["action"] == "ping");
    assert(j["errors"].size() == 1);
    assert(j["notifyChanged"]["type"] == "typeA");
    std::cout << "WebSocketServerSide: " << j.dump() << std::endl;
}

void test_to_json_WebSocketClientSide() {
    WebSocketClientSide ws;
    ws.action = "query";
    ws.accessToken = "token";
    ws.lastMessageId = "lastid";
    ws.clientInfo = ClientInfo{App{"Neko", "1.2.3", "4.5.6", "build123"}, System{"Windows", "x64", "10.0.19045"}, Extra{}, "device-xyz"};
    ws.preferences = Preferences{"en"};
    nlohmann::json j;
    to_json(j, ws);
    assert(j["action"] == "query");
    assert(j["accessToken"] == "token");
    assert(j["clientInfo"]["deviceId"] == "device-xyz");
    std::cout << "WebSocketClientSide: " << j.dump() << std::endl;
}

void test_to_json_StaticConfig_StaticLauncherConfig() {
    StaticConfig::StaticLauncherConfig slc;
    slc.host = {"host1"};
    slc.retryIntervalSec = 10;
    slc.maxRetryCount = 5;
    slc.webSocket = LauncherConfigResponse::WebSocket{true, "ws://host", 30};
    slc.security = LauncherConfigResponse::Security{true, 3600, 7, "login", "logout", "refresh"};
    slc.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    StaticConfig::StaticLauncherConfig::CheckUpdateUrls cu;
    cu.system = System{"Windows", "x64", "10.0.19045"};
    cu.url = "http://update";
    slc.checkUpdateUrls = {cu};
    nlohmann::json j;
    to_json(j, slc);
    assert(j["host"].size() == 1);
    assert(j["checkUpdateUrls"].size() == 1);
    assert(j["checkUpdateUrls"][0]["url"] == "http://update");
    std::cout << "StaticLauncherConfig: " << j.dump() << std::endl;
}

void test_to_json_StaticConfig_StaticMaintenanceInfo() {
    StaticConfig::StaticMaintenanceInfo smi;
    smi.status = "completed";
    smi.message = "done";
    smi.startTime = "2024-06-01T00:00:00Z";
    smi.exEndTime = "2024-06-01T02:00:00Z";
    smi.posterUrl = "http://poster";
    smi.link = "http://link";
    smi.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    nlohmann::json j;
    to_json(j, smi);
    assert(j["status"] == "completed");
    std::cout << "StaticMaintenanceInfo: " << j.dump() << std::endl;
}

void test_to_json_StaticConfig() {
    StaticConfig sc;
    sc.staticLauncherConfig.host = {"host1"};
    sc.staticLauncherConfig.retryIntervalSec = 10;
    sc.staticLauncherConfig.maxRetryCount = 5;
    sc.staticLauncherConfig.webSocket = LauncherConfigResponse::WebSocket{true, "ws://host", 30};
    sc.staticLauncherConfig.security = LauncherConfigResponse::Security{true, 3600, 7, "login", "logout", "refresh"};
    sc.staticLauncherConfig.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    StaticConfig::StaticLauncherConfig::CheckUpdateUrls cu;
    cu.system = System{"Windows", "x64", "10.0.19045"};
    cu.url = "http://update";
    sc.staticLauncherConfig.checkUpdateUrls = {cu};
    sc.staticMaintenanceInfo.status = "completed";
    sc.staticMaintenanceInfo.message = "done";
    sc.staticMaintenanceInfo.startTime = "2024-06-01T00:00:00Z";
    sc.staticMaintenanceInfo.exEndTime = "2024-06-01T02:00:00Z";
    sc.staticMaintenanceInfo.posterUrl = "http://poster";
    sc.staticMaintenanceInfo.link = "http://link";
    sc.staticMaintenanceInfo.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    nlohmann::json j;
    to_json(j, sc);
    assert(j["staticLauncherConfig"]["host"].size() == 1);
    assert(j["staticMaintenanceInfo"]["status"] == "completed");
    std::cout << "StaticConfig: " << j.dump() << std::endl;
}

void test_to_json_StaticUpdateInfo() {
    StaticUpdateInfo sui;
    sui.coreVersion = "1.2.3";
    sui.resourceVersion = "4.5.6";
    sui.updateResponse.title = "Update";
    sui.updateResponse.description = "Desc";
    sui.updateResponse.posterUrl = "http://poster";
    sui.updateResponse.publishTime = "2024-06-01T00:00:00Z";
    sui.updateResponse.resourceVersion = "1.2.3";
    sui.updateResponse.isMandatory = true;
    sui.updateResponse.meta = Meta{"1.0", "0.9", "build42", "2024-06-01", "", 123456789, false};
    sui.updateResponse.files = {UpdateResponse::File{"http://file", "file.zip", "abc123", "sha256", true, false, true}};
    nlohmann::json j;
    to_json(j, sui);
    assert(j["coreVersion"] == "1.2.3");
    assert(j["updateResponse"]["title"] == "Update");
    std::cout << "StaticUpdateInfo: " << j.dump() << std::endl;
}

int main() {
    test_to_json_Error();
    test_to_json_Meta();
    test_to_json_Preferences();
    test_to_json_App();
    test_to_json_System();
    test_to_json_Extra();
    test_to_json_ClientInfo();
    test_to_json_Auth();
    test_to_json_LauncherConfigResponse();
    test_to_json_MaintenanceResponse();
    test_to_json_UpdateResponse_File();
    test_to_json_UpdateResponse();
    test_to_json_FeedbackLogRequest();
    test_to_json_WebSocketServerSide_NotifyChanged();
    test_to_json_WebSocketServerSide();
    test_to_json_WebSocketClientSide();
    test_to_json_StaticConfig_StaticLauncherConfig();
    test_to_json_StaticConfig_StaticMaintenanceInfo();
    test_to_json_StaticConfig();
    test_to_json_StaticUpdateInfo();
    std::cout << "All to_json tests passed." << std::endl;
    return 0;
}