#include <gtest/gtest.h>

#include "neko/core/update.hpp"
#include "neko/app/api.hpp"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Test fixture for update tests
class UpdateTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "neko_update_test";
        fs::create_directories(testDir);
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
};

// Test parseUpdate with valid JSON
TEST_F(UpdateTest, ParseUpdateValidJson) {
    std::string validJson = R"({
        "updateResponse": {
            "title": "Test Update",
            "description": "Test Description",
            "posterUrl": "https://example.com/poster.png",
            "publishTime": "2025-12-02",
            "resourceVersion": "1.0.0",
            "isMandatory": true,
            "files": [
                {
                    "url": "https://example.com/file1.zip",
                    "fileName": "file1.zip",
                    "checksum": "abc123",
                    "downloadMeta": {
                        "hashAlgorithm": "sha256",
                        "suggestMultiThread": true,
                        "isCoreFile": false,
                        "isAbsoluteUrl": true
                    }
                }
            ]
        }
    })";

    auto result = neko::core::update::parseUpdate(validJson);
    
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.title, "Test Update");
    EXPECT_EQ(result.description, "Test Description");
    EXPECT_EQ(result.posterUrl, "https://example.com/poster.png");
    EXPECT_EQ(result.publishTime, "2025-12-02");
    EXPECT_EQ(result.resourceVersion, "1.0.0");
    EXPECT_TRUE(result.isMandatory);
    EXPECT_EQ(result.files.size(), 1);
    EXPECT_EQ(result.files[0].url, "https://example.com/file1.zip");
    EXPECT_EQ(result.files[0].fileName, "file1.zip");
    EXPECT_EQ(result.files[0].checksum, "abc123");
    EXPECT_EQ(result.files[0].hashAlgorithm, "sha256");
    EXPECT_TRUE(result.files[0].suggestMultiThread);
    EXPECT_FALSE(result.files[0].isCoreFile);
    EXPECT_TRUE(result.files[0].isAbsoluteUrl);
}

// Test parseUpdate with invalid JSON
TEST_F(UpdateTest, ParseUpdateInvalidJson) {
    std::string invalidJson = "not a json";
    EXPECT_THROW({ neko::core::update::parseUpdate(invalidJson); }, neko::ex::ParseError);
}

// Test parseUpdate with missing fields
TEST_F(UpdateTest, ParseUpdateMissingFields) {
    std::string incompleteJson = R"({
        "updateResponse": {
            "title": "Test Update",
            "description": "Test Description"
        }
    })";

    EXPECT_THROW({ neko::core::update::parseUpdate(incompleteJson); }, neko::ex::RangeError);
}

// Test parseUpdate with empty files array
TEST_F(UpdateTest, ParseUpdateEmptyFiles) {
    std::string jsonWithEmptyFiles = R"({
        "updateResponse": {
            "title": "Test Update",
            "description": "Test Description",
            "posterUrl": "https://example.com/poster.png",
            "publishTime": "2025-12-02",
            "resourceVersion": "1.0.0",
            "isMandatory": false,
            "files": []
        }
    })";
    
    auto result = neko::core::update::parseUpdate(jsonWithEmptyFiles);
    EXPECT_TRUE(result.empty());
}

// Test update with empty data
TEST_F(UpdateTest, UpdateWithEmptyData) {
    neko::api::UpdateResponse emptyData;

    EXPECT_THROW({ neko::core::update::update(emptyData); }, neko::ex::ArgumentError);
}

// Test UpdateResponse::File structure
TEST_F(UpdateTest, UpdateResponseFileStructure) {
    neko::api::UpdateResponse::File file{
        .url = "https://example.com/file.zip",
        .fileName = "file.zip",
        .checksum = "abc123",
        .hashAlgorithm = "sha256",
        .suggestMultiThread = true,
        .isCoreFile = false,
        .isAbsoluteUrl = true
    };
    
    EXPECT_EQ(file.url, "https://example.com/file.zip");
    EXPECT_EQ(file.fileName, "file.zip");
    EXPECT_EQ(file.checksum, "abc123");
    EXPECT_EQ(file.hashAlgorithm, "sha256");
    EXPECT_TRUE(file.suggestMultiThread);
    EXPECT_FALSE(file.isCoreFile);
    EXPECT_TRUE(file.isAbsoluteUrl);
}

// Test UpdateResponse::empty() method
TEST_F(UpdateTest, UpdateResponseEmptyMethod) {
    neko::api::UpdateResponse emptyResponse;
    EXPECT_TRUE(emptyResponse.empty());
    
    neko::api::UpdateResponse nonEmptyResponse;
    nonEmptyResponse.title = "Test";
    nonEmptyResponse.files.push_back({});
    EXPECT_FALSE(nonEmptyResponse.empty());
}

// Test parseUpdate with multiple files
TEST_F(UpdateTest, ParseUpdateMultipleFiles) {
    std::string jsonWithMultipleFiles = R"({
        "updateResponse": {
            "title": "Multi-file Update",
            "description": "Update with multiple files",
            "posterUrl": "https://example.com/poster.png",
            "publishTime": "2025-12-02",
            "resourceVersion": "2.0.0",
            "isMandatory": false,
            "files": [
                {
                    "url": "file1.zip",
                    "fileName": "file1.zip",
                    "checksum": "hash1",
                    "downloadMeta": {
                        "hashAlgorithm": "sha256",
                        "suggestMultiThread": false,
                        "isCoreFile": true,
                        "isAbsoluteUrl": false
                    }
                },
                {
                    "url": "https://cdn.example.com/file2.zip",
                    "fileName": "file2.zip",
                    "checksum": "hash2",
                    "downloadMeta": {
                        "hashAlgorithm": "md5",
                        "suggestMultiThread": true,
                        "isCoreFile": false,
                        "isAbsoluteUrl": true
                    }
                }
            ]
        }
    })";
    
    auto result = neko::core::update::parseUpdate(jsonWithMultipleFiles);
    
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.files.size(), 2);
    EXPECT_EQ(result.files[0].fileName, "file1.zip");
    EXPECT_TRUE(result.files[0].isCoreFile);
    EXPECT_FALSE(result.files[0].isAbsoluteUrl);
    EXPECT_EQ(result.files[1].fileName, "file2.zip");
    EXPECT_FALSE(result.files[1].isCoreFile);
    EXPECT_TRUE(result.files[1].isAbsoluteUrl);
}
