#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "neko/app/appinfo.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace neko::app;

class AppInfoTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test getAppName function
TEST_F(AppInfoTest, GetAppName) {
    auto appName = getAppName();
    EXPECT_NE(appName, nullptr);
    EXPECT_GT(std::strlen(appName), 0);
}

// Test getVersion function
TEST_F(AppInfoTest, GetVersion) {
    auto version = getVersion();
    EXPECT_NE(version, nullptr);
    EXPECT_GT(std::strlen(version), 0);
}

// Test getBuildId function
TEST_F(AppInfoTest, GetBuildId) {
    auto buildId = getBuildId();
    EXPECT_NE(buildId, nullptr);
    // BuildId might be empty in some builds, so we just check it's not null
}

// Test getConfigFileName function
TEST_F(AppInfoTest, GetConfigFileName) {
    auto configFileName = getConfigFileName();
    EXPECT_NE(configFileName, nullptr);
    EXPECT_GT(std::strlen(configFileName), 0);
    
    // Should be a .ini file or similar
    std::string filename(configFileName);
    EXPECT_TRUE(filename.find("config") != std::string::npos || 
                filename.find(".ini") != std::string::npos);
}

// Test getStaticRemoteConfigUrl function
TEST_F(AppInfoTest, GetStaticRemoteConfigUrl) {
    auto url = getStaticRemoteConfigUrl();
    EXPECT_NE(url, nullptr);
    // URL might be empty if not configured
}

// Test constexpr functions are compile-time evaluable
TEST_F(AppInfoTest, ConstexprFunctions) {
    constexpr auto name = getAppName();
    constexpr auto ver = getVersion();
    constexpr auto buildId = getBuildId();
    constexpr auto configFile = getConfigFileName();
    constexpr auto remoteUrl = getStaticRemoteConfigUrl();
    
    EXPECT_NE(name, nullptr);
    EXPECT_NE(ver, nullptr);
    EXPECT_NE(buildId, nullptr);
    EXPECT_NE(configFile, nullptr);
    EXPECT_NE(remoteUrl, nullptr);
}

// Test getPreferences function
TEST_F(AppInfoTest, GetPreferences) {
    auto prefs = getPreferences();
    EXPECT_FALSE(prefs.language.empty());
}

// Note: The following functions require a properly initialized config bus:
// - getResourceVersion()
// - getDeviceId()
// - getClientInfo()
// - getRequestJson()
//
// These should be tested in integration tests with proper setup of the
// configuration system.

TEST_F(AppInfoTest, AppNameConsistency) {
    auto name1 = getAppName();
    auto name2 = getAppName();
    
    EXPECT_STREQ(name1, name2);
}

TEST_F(AppInfoTest, VersionConsistency) {
    auto ver1 = getVersion();
    auto ver2 = getVersion();
    
    EXPECT_STREQ(ver1, ver2);
}

TEST_F(AppInfoTest, BuildIdConsistency) {
    auto id1 = getBuildId();
    auto id2 = getBuildId();
    
    EXPECT_STREQ(id1, id2);
}

TEST_F(AppInfoTest, ConfigFileNameConsistency) {
    auto file1 = getConfigFileName();
    auto file2 = getConfigFileName();
    
    EXPECT_STREQ(file1, file2);
}

TEST_F(AppInfoTest, RemoteConfigUrlConsistency) {
    auto url1 = getStaticRemoteConfigUrl();
    auto url2 = getStaticRemoteConfigUrl();
    
    EXPECT_STREQ(url1, url2);
}

TEST_F(AppInfoTest, PreferencesLanguageNotEmpty) {
    auto prefs = getPreferences();
    // Language should have a default value even if not set
    EXPECT_FALSE(prefs.language.empty());
}
