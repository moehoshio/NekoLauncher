#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "neko/minecraft/downloadSource.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace neko::minecraft;

class DownloadSourceTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test getMinecraftListUrl function
TEST_F(DownloadSourceTest, GetMinecraftListUrl_Official) {
    std::string url = getMinecraftListUrl(DownloadSource::Official);
    EXPECT_EQ(url, "https://piston-meta.mojang.com/mc/game/version_manifest.json");
}

TEST_F(DownloadSourceTest, GetMinecraftListUrl_BMCLAPI) {
    std::string url = getMinecraftListUrl(DownloadSource::BMCLAPI);
    EXPECT_EQ(url, "https://bmclapi2.bangbang93.com/mc/game/version_manifest.json");
}

TEST_F(DownloadSourceTest, GetMinecraftListUrl_Default) {
    std::string url = getMinecraftListUrl();
    EXPECT_EQ(url, "https://piston-meta.mojang.com/mc/game/version_manifest.json");
}

// Test replaceWithBMCLAPI function
TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_PistonMeta) {
    std::string originalUrl = "https://piston-meta.mojang.com/v1/packages/test.json";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/v1/packages/test.json";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_LauncherMeta) {
    std::string originalUrl = "https://launchermeta.mojang.com/v1/packages/test.json";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/v1/packages/test.json";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_Launcher) {
    std::string originalUrl = "https://launcher.mojang.com/assets/test.jar";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/assets/test.jar";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_Libraries) {
    std::string originalUrl = "https://libraries.minecraft.net/com/mojang/authlib/1.5.25/authlib-1.5.25.jar";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/maven/com/mojang/authlib/1.5.25/authlib-1.5.25.jar";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_Resources) {
    std::string originalUrl = "https://resources.download.minecraft.net/00/00a1b2c3d4e5f6.png";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/assets/00/00a1b2c3d4e5f6.png";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_MinecraftForge_FilesURL) {
    std::string originalUrl = "https://files.minecraftforge.net/maven/net/minecraftforge/forge/1.16.5-36.2.0/forge-1.16.5-36.2.0-installer.jar";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/maven/maven/net/minecraftforge/forge/1.16.5-36.2.0/forge-1.16.5-36.2.0-installer.jar";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_MinecraftForge_MavenURL) {
    std::string originalUrl = "https://maven.minecraftforge.net/net/minecraftforge/forge/1.16.5-36.2.0/forge-1.16.5-36.2.0.jar";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/maven/net/minecraftforge/forge/1.16.5-36.2.0/forge-1.16.5-36.2.0.jar";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_JavaRuntime) {
    std::string originalUrl = "https://launchermeta.mojang.com/v1/products/java-runtime/2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/v1/products/java-runtime/2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_NoMatch) {
    std::string originalUrl = "https://example.com/some/path/file.jar";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, originalUrl); // Should remain unchanged
}

TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_EmptyString) {
    std::string originalUrl = "";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, "");
}

// Test downloadSourceMap
TEST_F(DownloadSourceTest, DownloadSourceMap_Contains_Official) {
    EXPECT_TRUE(downloadSourceMap.find(DownloadSource::Official) != downloadSourceMap.end());
    EXPECT_EQ(downloadSourceMap[DownloadSource::Official], "Official");
}

TEST_F(DownloadSourceTest, DownloadSourceMap_Contains_BMCLAPI) {
    EXPECT_TRUE(downloadSourceMap.find(DownloadSource::BMCLAPI) != downloadSourceMap.end());
    EXPECT_EQ(downloadSourceMap[DownloadSource::BMCLAPI], "BMCLAPI");
}

// Test edge cases for replaceWithBMCLAPI
TEST_F(DownloadSourceTest, ReplaceWithBMCLAPI_MultipleReplacements) {
    // Test that only the first occurrence is replaced
    std::string originalUrl = "https://piston-meta.mojang.com/test/https://piston-meta.mojang.com/test2";
    std::string expectedUrl = "https://bmclapi2.bangbang93.com/test/https://piston-meta.mojang.com/test2";
    std::string result = replaceWithBMCLAPI(originalUrl);
    EXPECT_EQ(result, expectedUrl);
}
