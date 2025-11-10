#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "neko/minecraft/launcherMinecraft.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace neko::minecraft;

class LauncherMinecraftTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test LauncherMinecraftConfig default construction
TEST_F(LauncherMinecraftTest, Config_DefaultConstruction) {
    LauncherMinecraftConfig config;
    
    EXPECT_TRUE(config.minecraftFolder.empty());
    EXPECT_TRUE(config.targetVersion.empty());
    EXPECT_TRUE(config.javaPath.empty());
    EXPECT_TRUE(config.playerName.empty());
    EXPECT_TRUE(config.uuid.empty());
    EXPECT_TRUE(config.accessToken.empty());
    EXPECT_EQ(config.joinServerAddress, "");
    EXPECT_EQ(config.joinServerPort, "25565");
}

// Test LauncherMinecraftConfig boolean defaults
TEST_F(LauncherMinecraftTest, Config_BooleanDefaults) {
    LauncherMinecraftConfig config;
    
    EXPECT_FALSE(config.tolerantMode);
    EXPECT_FALSE(config.isDemoUser);
    EXPECT_FALSE(config.hasCustomResolution);
}

// Test LauncherMinecraftConfig memory defaults
TEST_F(LauncherMinecraftTest, Config_MemoryDefaults) {
    LauncherMinecraftConfig config;
    
    EXPECT_EQ(config.maxMemoryLimit, 8);
    EXPECT_EQ(config.minMemoryLimit, 2);
    EXPECT_EQ(config.needMemoryLimit, 7);
}

// Test LauncherMinecraftConfig resolution defaults
TEST_F(LauncherMinecraftTest, Config_ResolutionDefaults) {
    LauncherMinecraftConfig config;
    
    EXPECT_EQ(config.resolutionWidth, "1280");
    EXPECT_EQ(config.resolutionHeight, "720");
}

// Test setting basic fields
TEST_F(LauncherMinecraftTest, Config_SetBasicFields) {
    LauncherMinecraftConfig config;
    
    config.minecraftFolder = "/path/to/.minecraft";
    config.targetVersion = "1.20.1";
    config.javaPath = "/usr/bin/java";
    config.playerName = "TestPlayer";
    config.uuid = "123e4567-e89b-12d3-a456-426614174000";
    config.accessToken = "test_token_12345";
    
    EXPECT_EQ(config.minecraftFolder, "/path/to/.minecraft");
    EXPECT_EQ(config.targetVersion, "1.20.1");
    EXPECT_EQ(config.javaPath, "/usr/bin/java");
    EXPECT_EQ(config.playerName, "TestPlayer");
    EXPECT_EQ(config.uuid, "123e4567-e89b-12d3-a456-426614174000");
    EXPECT_EQ(config.accessToken, "test_token_12345");
}

// Test setting server join information
TEST_F(LauncherMinecraftTest, Config_SetServerJoinInfo) {
    LauncherMinecraftConfig config;
    
    config.joinServerAddress = "play.example.com";
    config.joinServerPort = "25566";
    
    EXPECT_EQ(config.joinServerAddress, "play.example.com");
    EXPECT_EQ(config.joinServerPort, "25566");
}

// Test setting server port without address
TEST_F(LauncherMinecraftTest, Config_ServerPortWithoutAddress) {
    LauncherMinecraftConfig config;
    
    config.joinServerPort = "19132"; // Custom port
    
    EXPECT_EQ(config.joinServerAddress, ""); // Should still be empty
    EXPECT_EQ(config.joinServerPort, "19132");
}

// Test tolerant mode
TEST_F(LauncherMinecraftTest, Config_TolerantMode) {
    LauncherMinecraftConfig config;
    
    EXPECT_FALSE(config.tolerantMode); // Default is false
    
    config.tolerantMode = true;
    EXPECT_TRUE(config.tolerantMode);
    
    config.tolerantMode = false;
    EXPECT_FALSE(config.tolerantMode);
}

// Test custom memory limits
TEST_F(LauncherMinecraftTest, Config_CustomMemoryLimits) {
    LauncherMinecraftConfig config;
    
    config.maxMemoryLimit = 16;
    config.minMemoryLimit = 4;
    config.needMemoryLimit = 12;
    
    EXPECT_EQ(config.maxMemoryLimit, 16);
    EXPECT_EQ(config.minMemoryLimit, 4);
    EXPECT_EQ(config.needMemoryLimit, 12);
}

// Test minimal memory configuration
TEST_F(LauncherMinecraftTest, Config_MinimalMemory) {
    LauncherMinecraftConfig config;
    
    config.maxMemoryLimit = 2;
    config.minMemoryLimit = 1;
    config.needMemoryLimit = 2;
    
    EXPECT_EQ(config.maxMemoryLimit, 2);
    EXPECT_EQ(config.minMemoryLimit, 1);
    EXPECT_EQ(config.needMemoryLimit, 2);
}

// Test high memory configuration
TEST_F(LauncherMinecraftTest, Config_HighMemory) {
    LauncherMinecraftConfig config;
    
    config.maxMemoryLimit = 32;
    config.minMemoryLimit = 8;
    config.needMemoryLimit = 24;
    
    EXPECT_EQ(config.maxMemoryLimit, 32);
    EXPECT_EQ(config.minMemoryLimit, 8);
    EXPECT_EQ(config.needMemoryLimit, 24);
}

// Test demo user mode
TEST_F(LauncherMinecraftTest, Config_DemoUser) {
    LauncherMinecraftConfig config;
    
    EXPECT_FALSE(config.isDemoUser); // Default is false
    
    config.isDemoUser = true;
    EXPECT_TRUE(config.isDemoUser);
}

// Test custom resolution disabled
TEST_F(LauncherMinecraftTest, Config_CustomResolutionDisabled) {
    LauncherMinecraftConfig config;
    
    EXPECT_FALSE(config.hasCustomResolution);
    EXPECT_EQ(config.resolutionWidth, "1280");
    EXPECT_EQ(config.resolutionHeight, "720");
}

// Test custom resolution enabled with custom values
TEST_F(LauncherMinecraftTest, Config_CustomResolutionEnabled) {
    LauncherMinecraftConfig config;
    
    config.hasCustomResolution = true;
    config.resolutionWidth = "1920";
    config.resolutionHeight = "1080";
    
    EXPECT_TRUE(config.hasCustomResolution);
    EXPECT_EQ(config.resolutionWidth, "1920");
    EXPECT_EQ(config.resolutionHeight, "1080");
}

// Test various resolution configurations
TEST_F(LauncherMinecraftTest, Config_VariousResolutions) {
    LauncherMinecraftConfig config;
    config.hasCustomResolution = true;
    
    // 4K resolution
    config.resolutionWidth = "3840";
    config.resolutionHeight = "2160";
    EXPECT_EQ(config.resolutionWidth, "3840");
    EXPECT_EQ(config.resolutionHeight, "2160");
    
    // 720p resolution
    config.resolutionWidth = "1280";
    config.resolutionHeight = "720";
    EXPECT_EQ(config.resolutionWidth, "1280");
    EXPECT_EQ(config.resolutionHeight, "720");
    
    // Custom small resolution
    config.resolutionWidth = "800";
    config.resolutionHeight = "600";
    EXPECT_EQ(config.resolutionWidth, "800");
    EXPECT_EQ(config.resolutionHeight, "600");
}

// Test configuration with all options enabled
TEST_F(LauncherMinecraftTest, Config_AllOptionsEnabled) {
    LauncherMinecraftConfig config;
    
    config.minecraftFolder = "C:/Games/Minecraft";
    config.targetVersion = "1.20.4";
    config.javaPath = "C:/Java/bin/java.exe";
    config.playerName = "SuperPlayer";
    config.uuid = "abcd1234-5678-90ef-ghij-klmnopqrstuv";
    config.accessToken = "very_long_access_token_string";
    config.joinServerAddress = "mc.server.net";
    config.joinServerPort = "25565";
    config.tolerantMode = true;
    config.maxMemoryLimit = 16;
    config.minMemoryLimit = 4;
    config.needMemoryLimit = 12;
    config.isDemoUser = false;
    config.hasCustomResolution = true;
    config.resolutionWidth = "2560";
    config.resolutionHeight = "1440";
    
    EXPECT_EQ(config.minecraftFolder, "C:/Games/Minecraft");
    EXPECT_EQ(config.targetVersion, "1.20.4");
    EXPECT_EQ(config.javaPath, "C:/Java/bin/java.exe");
    EXPECT_EQ(config.playerName, "SuperPlayer");
    EXPECT_EQ(config.uuid, "abcd1234-5678-90ef-ghij-klmnopqrstuv");
    EXPECT_EQ(config.accessToken, "very_long_access_token_string");
    EXPECT_EQ(config.joinServerAddress, "mc.server.net");
    EXPECT_EQ(config.joinServerPort, "25565");
    EXPECT_TRUE(config.tolerantMode);
    EXPECT_EQ(config.maxMemoryLimit, 16);
    EXPECT_EQ(config.minMemoryLimit, 4);
    EXPECT_EQ(config.needMemoryLimit, 12);
    EXPECT_FALSE(config.isDemoUser);
    EXPECT_TRUE(config.hasCustomResolution);
    EXPECT_EQ(config.resolutionWidth, "2560");
    EXPECT_EQ(config.resolutionHeight, "1440");
}

// Test copying configuration
TEST_F(LauncherMinecraftTest, Config_CopyConfiguration) {
    LauncherMinecraftConfig config1;
    config1.minecraftFolder = "/test/path";
    config1.targetVersion = "1.19.2";
    config1.playerName = "Player1";
    config1.maxMemoryLimit = 10;
    config1.tolerantMode = true;
    
    LauncherMinecraftConfig config2 = config1;
    
    EXPECT_EQ(config2.minecraftFolder, "/test/path");
    EXPECT_EQ(config2.targetVersion, "1.19.2");
    EXPECT_EQ(config2.playerName, "Player1");
    EXPECT_EQ(config2.maxMemoryLimit, 10);
    EXPECT_TRUE(config2.tolerantMode);
}

// Test modifying copy doesn't affect original
TEST_F(LauncherMinecraftTest, Config_CopyIndependence) {
    LauncherMinecraftConfig config1;
    config1.playerName = "Original";
    
    LauncherMinecraftConfig config2 = config1;
    config2.playerName = "Modified";
    
    EXPECT_EQ(config1.playerName, "Original");
    EXPECT_EQ(config2.playerName, "Modified");
}

// Test empty strings vs default values
TEST_F(LauncherMinecraftTest, Config_EmptyStrings) {
    LauncherMinecraftConfig config;
    
    config.minecraftFolder = "";
    config.targetVersion = "";
    config.javaPath = "";
    config.playerName = "";
    config.uuid = "";
    config.accessToken = "";
    
    EXPECT_TRUE(config.minecraftFolder.empty());
    EXPECT_TRUE(config.targetVersion.empty());
    EXPECT_TRUE(config.javaPath.empty());
    EXPECT_TRUE(config.playerName.empty());
    EXPECT_TRUE(config.uuid.empty());
    EXPECT_TRUE(config.accessToken.empty());
}

// Test version strings with various formats
TEST_F(LauncherMinecraftTest, Config_VersionFormats) {
    LauncherMinecraftConfig config;
    
    // Standard release version
    config.targetVersion = "1.20.1";
    EXPECT_EQ(config.targetVersion, "1.20.1");
    
    // Snapshot version
    config.targetVersion = "23w51a";
    EXPECT_EQ(config.targetVersion, "23w51a");
    
    // Old version
    config.targetVersion = "1.8.9";
    EXPECT_EQ(config.targetVersion, "1.8.9");
    
    // Very old version
    config.targetVersion = "1.2.5";
    EXPECT_EQ(config.targetVersion, "1.2.5");
    
    // Custom/modded version
    config.targetVersion = "1.16.5-forge-36.2.39";
    EXPECT_EQ(config.targetVersion, "1.16.5-forge-36.2.39");
}
