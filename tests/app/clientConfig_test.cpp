#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "neko/app/clientConfig.hpp"
#include <gtest/gtest.h>
#include <SimpleIni.h>
#include <filesystem>
#include <fstream>

using namespace neko;

class ClientConfigTest : public ::testing::Test {
protected:
    std::string testConfigFile;

    void SetUp() override {
        testConfigFile = (std::filesystem::temp_directory_path() / "test_config.ini").string();
    }

    void TearDown() override {
        if (std::filesystem::exists(testConfigFile)) {
            std::filesystem::remove(testConfigFile);
        }
    }

    void createTestConfigFile() {
        std::ofstream file(testConfigFile);
        file << "[main]\n"
             << "language=en\n"
             << "backgroundType=image\n"
             << "background=img/bg.png\n"
             << "windowSize=1920x1080\n"
             << "launcherMethod=1\n"
             << "useSystemWindowFrame=true\n"
             << "headBarKeepRight=true\n"
             << "resourceVersion=1.0.0\n"
             << "deviceID=test-device-id\n"
             << "\n[style]\n"
             << "blurEffect=Animation\n"
             << "blurRadius=10\n"
             << "fontPointSize=12\n"
             << "fontFamilies=Arial,Sans\n"
             << "\n[net]\n"
             << "thread=4\n"
             << "proxy=true\n"
             << "\n[dev]\n"
             << "enable=false\n"
             << "debug=false\n"
             << "server=auto\n"
             << "tls=true\n"
             << "\n[other]\n"
             << "customTempDir=/tmp/test\n"
             << "\n[minecraft]\n"
             << "minecraftFolder=.minecraft\n"
             << "javaPath=/usr/bin/java\n"
             << "downloadSource=Official\n"
             << "playerName=TestPlayer\n"
             << "account=test@example.com\n"
             << "uuid=12345678-1234-1234-1234-123456789012\n"
             << "accessToken=test-token\n"
             << "targetVersion=1.20.1\n"
             << "maxMemoryLimit=4096\n"
             << "minMemoryLimit=1024\n"
             << "needMemoryLimit=2048\n"
             << "authlibName=authlib-injector.jar\n"
             << "authlibPrefetched=\n"
             << "authlibSha256=\n"
             << "tolerantMode=false\n"
             << "customResolution=1920x1080\n"
             << "joinServerAddress=mc.example.com\n"
             << "joinServerPort=25565\n";
        file.close();
    }
};

// Test default constructor
TEST_F(ClientConfigTest, DefaultConstructor) {
    ClientConfig config;
    // Default constructor should create an empty config
    SUCCEED();
}

// Test loading from SimpleIni
TEST_F(ClientConfigTest, LoadFromSimpleIni) {
    createTestConfigFile();
    
    CSimpleIniA ini;
    ASSERT_EQ(ini.LoadFile(testConfigFile.c_str()), SI_OK);
    
    ClientConfig config(ini);
    
    // Test main section
    EXPECT_STREQ(config.main.lang, "en");
    EXPECT_STREQ(config.main.backgroundType, "image");
    EXPECT_STREQ(config.main.background, "img/bg.png");
    EXPECT_STREQ(config.main.windowSize, "1920x1080");
    EXPECT_EQ(config.main.launcherMethod, 1);
    EXPECT_TRUE(config.main.useSysWindowFrame);
    EXPECT_TRUE(config.main.headBarKeepRight);
    EXPECT_STREQ(config.main.resourceVersion, "1.0.0");
    EXPECT_STREQ(config.main.deviceID, "test-device-id");
}

// Test style section
TEST_F(ClientConfigTest, StyleSection) {
    createTestConfigFile();
    
    CSimpleIniA ini;
    ini.LoadFile(testConfigFile.c_str());
    
    ClientConfig config(ini);
    
    EXPECT_STREQ(config.style.blurEffect, "Animation");
    EXPECT_EQ(config.style.blurRadius, 10);
    EXPECT_EQ(config.style.fontPointSize, 12);
    EXPECT_STREQ(config.style.fontFamilies, "Arial,Sans");
}

// Test net section
TEST_F(ClientConfigTest, NetSection) {
    createTestConfigFile();
    
    CSimpleIniA ini;
    ini.LoadFile(testConfigFile.c_str());
    
    ClientConfig config(ini);
    
    EXPECT_EQ(config.net.thread, 4);
    EXPECT_STREQ(config.net.proxy, "true");
}

// Test dev section
TEST_F(ClientConfigTest, DevSection) {
    createTestConfigFile();
    
    CSimpleIniA ini;
    ini.LoadFile(testConfigFile.c_str());
    
    ClientConfig config(ini);
    
    EXPECT_FALSE(config.dev.enable);
    EXPECT_FALSE(config.dev.debug);
    EXPECT_STREQ(config.dev.server, "auto");
    EXPECT_TRUE(config.dev.tls);
}

// Test other section
TEST_F(ClientConfigTest, OtherSection) {
    createTestConfigFile();
    
    CSimpleIniA ini;
    ini.LoadFile(testConfigFile.c_str());
    
    ClientConfig config(ini);
    
    EXPECT_STREQ(config.other.tempFolder, "/tmp/test");
}

// Test minecraft section
TEST_F(ClientConfigTest, MinecraftSection) {
    createTestConfigFile();
    
    CSimpleIniA ini;
    ini.LoadFile(testConfigFile.c_str());
    
    ClientConfig config(ini);
    
    EXPECT_STREQ(config.minecraft.minecraftFolder, ".minecraft");
    EXPECT_STREQ(config.minecraft.javaPath, "/usr/bin/java");
    EXPECT_STREQ(config.minecraft.downloadSource, "Official");
    EXPECT_STREQ(config.minecraft.playerName, "TestPlayer");
    EXPECT_STREQ(config.minecraft.account, "test@example.com");
    EXPECT_STREQ(config.minecraft.uuid, "12345678-1234-1234-1234-123456789012");
    EXPECT_STREQ(config.minecraft.accessToken, "test-token");
    EXPECT_STREQ(config.minecraft.targetVersion, "1.20.1");
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 4096);
    EXPECT_EQ(config.minecraft.minMemoryLimit, 1024);
    EXPECT_EQ(config.minecraft.needMemoryLimit, 2048);
    EXPECT_STREQ(config.minecraft.authlibName, "authlib-injector.jar");
    EXPECT_FALSE(config.minecraft.tolerantMode);
    EXPECT_STREQ(config.minecraft.customResolution, "1920x1080");
    EXPECT_STREQ(config.minecraft.joinServerAddress, "mc.example.com");
    EXPECT_STREQ(config.minecraft.joinServerPort, "25565");
}

// Test default values when keys are missing
TEST_F(ClientConfigTest, DefaultValues) {
    CSimpleIniA ini;
    ClientConfig config(ini);
    
    // Check some default values
    EXPECT_STREQ(config.main.lang, "en");
    EXPECT_STREQ(config.main.backgroundType, "image");
    EXPECT_EQ(config.main.launcherMethod, 1);
    EXPECT_TRUE(config.main.useSysWindowFrame);
    EXPECT_EQ(config.style.blurRadius, 10);
    EXPECT_EQ(config.style.fontPointSize, 10);
    EXPECT_EQ(config.net.thread, 0);
    EXPECT_FALSE(config.dev.enable);
    EXPECT_FALSE(config.dev.debug);
    EXPECT_TRUE(config.dev.tls);
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 2048);
    EXPECT_EQ(config.minecraft.minMemoryLimit, 1024);
    EXPECT_FALSE(config.minecraft.tolerantMode);
}

// Test setToConfig function
TEST_F(ClientConfigTest, SetToConfig) {
    createTestConfigFile();
    
    CSimpleIniA ini;
    ini.LoadFile(testConfigFile.c_str());
    
    // Load config from ini
    ClientConfig config(ini);
    
    // Create a new ini and write config to it
    CSimpleIniA ini2;
    config.setToConfig(ini2);
    
    // Verify values were written correctly
    EXPECT_STREQ(ini2.GetValue("main", "language"), "en");
    EXPECT_STREQ(ini2.GetValue("main", "resourceVersion"), "1.0.0");
    EXPECT_EQ(ini2.GetLongValue("style", "blurRadius"), 10);
    EXPECT_EQ(ini2.GetLongValue("net", "thread"), 4);
    EXPECT_STREQ(ini2.GetValue("minecraft", "playerName"), "TestPlayer");
    EXPECT_EQ(ini2.GetLongValue("minecraft", "maxMemoryLimit"), 4096);
}

// Test round-trip: load, save, reload
TEST_F(ClientConfigTest, RoundTrip) {
    createTestConfigFile();
    
    CSimpleIniA ini1;
    ini1.LoadFile(testConfigFile.c_str());
    
    ClientConfig config1(ini1);
    
    // Save to new ini
    CSimpleIniA ini2;
    config1.setToConfig(ini2);
    
    // Load from new ini
    ClientConfig config2(ini2);
    
    // Values should be the same
    EXPECT_STREQ(config2.main.lang, config1.main.lang);
    EXPECT_EQ(config2.style.blurRadius, config1.style.blurRadius);
    EXPECT_STREQ(config2.main.backgroundType, config1.main.backgroundType);
    EXPECT_EQ(config2.net.thread, config1.net.thread);
}

// Test boolean values
TEST_F(ClientConfigTest, BooleanValues) {
    CSimpleIniA ini;
    
    // Set boolean values in ini
    ini.SetBoolValue("main", "useSystemWindowFrame", true);
    ini.SetBoolValue("main", "headBarKeepRight", false);
    ini.SetBoolValue("dev", "enable", true);
    ini.SetBoolValue("dev", "debug", true);
    ini.SetBoolValue("minecraft", "tolerantMode", true);
    
    // Load config from ini
    ClientConfig config(ini);
    
    EXPECT_TRUE(config.main.useSysWindowFrame);
    EXPECT_FALSE(config.main.headBarKeepRight);
    EXPECT_TRUE(config.dev.enable);
    EXPECT_TRUE(config.dev.debug);
    EXPECT_TRUE(config.minecraft.tolerantMode);
    
    // Write back to ini2
    CSimpleIniA ini2;
    config.setToConfig(ini2);
    
    // Verify values are preserved
    EXPECT_TRUE(ini2.GetBoolValue("main", "useSystemWindowFrame"));
    EXPECT_FALSE(ini2.GetBoolValue("main", "headBarKeepRight"));
    EXPECT_TRUE(ini2.GetBoolValue("dev", "enable"));
    EXPECT_TRUE(ini2.GetBoolValue("dev", "debug"));
    EXPECT_TRUE(ini2.GetBoolValue("minecraft", "tolerantMode"));
}

// Test memory limit values
TEST_F(ClientConfigTest, MemoryLimits) {
    CSimpleIniA ini;
    
    // Set memory limits in ini
    ini.SetLongValue("minecraft", "maxMemoryLimit", 16384);
    ini.SetLongValue("minecraft", "minMemoryLimit", 2048);
    ini.SetLongValue("minecraft", "needMemoryLimit", 4096);
    
    // Load config from ini
    ClientConfig config(ini);
    
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 16384);
    EXPECT_EQ(config.minecraft.minMemoryLimit, 2048);
    EXPECT_EQ(config.minecraft.needMemoryLimit, 4096);
    
    // Write back to ini2
    CSimpleIniA ini2;
    config.setToConfig(ini2);
    
    // Verify values are preserved
    EXPECT_EQ(ini2.GetLongValue("minecraft", "maxMemoryLimit"), 16384);
    EXPECT_EQ(ini2.GetLongValue("minecraft", "minMemoryLimit"), 2048);
    EXPECT_EQ(ini2.GetLongValue("minecraft", "needMemoryLimit"), 4096);
}
