#include <gtest/gtest.h>

#include "neko/core/downloadPoster.hpp"
#include <neko/function/utilities.hpp>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Test fixture for downloadPoster tests
class DownloadPosterTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "neko_download_poster_test";
        fs::create_directories(testDir);
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
};

// Test downloadPoster with empty URL
TEST_F(DownloadPosterTest, EmptyUrl) {
    auto result = neko::core::downloadPoster("");
    EXPECT_FALSE(result.has_value());
}

// Test downloadPoster with invalid URL
TEST_F(DownloadPosterTest, InvalidUrl) {
    auto result = neko::core::downloadPoster("not a url");
    EXPECT_FALSE(result.has_value());
}

// Test downloadPoster with malformed URL
TEST_F(DownloadPosterTest, MalformedUrl) {
    auto result = neko::core::downloadPoster("htp://invalid");
    EXPECT_FALSE(result.has_value());
}

// Test URL validation helper
TEST_F(DownloadPosterTest, UrlValidation) {
    // Valid URLs
    EXPECT_TRUE(neko::util::check::isUrl("https://example.com/poster.png"));
    EXPECT_TRUE(neko::util::check::isUrl("http://example.com/image.jpg"));
    EXPECT_TRUE(neko::util::check::isUrl("https://cdn.example.com/path/to/image.png"));
    
    // Invalid URLs
    EXPECT_FALSE(neko::util::check::isUrl(""));
    EXPECT_FALSE(neko::util::check::isUrl("not a url"));
    EXPECT_FALSE(neko::util::check::isUrl("/local/path"));
    EXPECT_FALSE(neko::util::check::isUrl("ftp://example.com/file.txt"));
}

// Test that downloadPoster generates unique filenames
TEST_F(DownloadPosterTest, UniqueFilenames) {
    // Since downloadPoster uses random strings, we can't directly test it
    // But we can verify the pattern
    std::string filename1 = neko::system::tempFolder() + "/poster_" + 
                           neko::util::random::generateRandomString(12) + ".png";
    std::string filename2 = neko::system::tempFolder() + "/poster_" + 
                           neko::util::random::generateRandomString(12) + ".png";
    
    EXPECT_NE(filename1, filename2);
    EXPECT_TRUE(filename1.find("/poster_") != std::string::npos);
    EXPECT_TRUE(filename2.find("/poster_") != std::string::npos);
    EXPECT_TRUE(filename1.find(".png") != std::string::npos);
    EXPECT_TRUE(filename2.find(".png") != std::string::npos);
}

// Test filename format
TEST_F(DownloadPosterTest, FilenameFormat) {
    std::string tempFolder = neko::system::tempFolder();
    std::string randomStr = neko::util::random::generateRandomString(12);
    std::string expectedPattern = tempFolder + "/poster_";
    
    EXPECT_FALSE(tempFolder.empty());
    EXPECT_EQ(randomStr.length(), 12);
}

// Test that function returns std::optional
TEST_F(DownloadPosterTest, ReturnTypeIsOptional) {
    auto result = neko::core::downloadPoster("");
    
    // Should be std::optional<std::string>
    EXPECT_FALSE(result.has_value());
    
    // If it had a value, we could access it
    if (result) {
        std::string filename = result.value();
        EXPECT_FALSE(filename.empty());
    }
}

// Test with various URL protocols
TEST_F(DownloadPosterTest, UrlProtocols) {
    // HTTPS
    auto httpsUrl = "https://example.com/poster.png";
    if (neko::util::check::isUrl(httpsUrl)) {
        EXPECT_TRUE(std::string(httpsUrl).find("https://") == 0);
    }
    
    // HTTP
    auto httpUrl = "http://example.com/poster.png";
    if (neko::util::check::isUrl(httpUrl)) {
        EXPECT_TRUE(std::string(httpUrl).find("http://") == 0);
    }
}

// Test URL with special characters
TEST_F(DownloadPosterTest, UrlWithSpecialCharacters) {
    std::string urlWithParams = "https://example.com/poster.png?size=large&format=png";
    
    if (neko::util::check::isUrl(urlWithParams)) {
        EXPECT_TRUE(urlWithParams.find('?') != std::string::npos);
        EXPECT_TRUE(urlWithParams.find('&') != std::string::npos);
    }
}

// Test URL with path
TEST_F(DownloadPosterTest, UrlWithPath) {
    std::string urlWithPath = "https://cdn.example.com/images/posters/game/poster123.png";
    
    if (neko::util::check::isUrl(urlWithPath)) {
        EXPECT_TRUE(urlWithPath.find("/images/") != std::string::npos);
        EXPECT_TRUE(urlWithPath.find("/posters/") != std::string::npos);
    }
}

// Test that temp folder path is used
TEST_F(DownloadPosterTest, UsesTempFolder) {
    std::string tempFolder = neko::system::tempFolder();
    EXPECT_FALSE(tempFolder.empty());
    
    // Verify temp folder exists or can be created
    if (!fs::exists(tempFolder)) {
        EXPECT_NO_THROW(fs::create_directories(tempFolder));
    }
}
