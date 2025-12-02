#include <gtest/gtest.h>

#include "neko/core/maintenance.hpp"

#include <filesystem>

namespace fs = std::filesystem;

// Test fixture for maintenance tests
class MaintenanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "neko_maintenance_test";
        fs::create_directories(testDir);
    }

    void TearDown() override {
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
};

// Test MaintenanceInfo structure creation
TEST_F(MaintenanceTest, MaintenanceInfoDefaultValues) {
    neko::core::MaintenanceInfo info;
    
    EXPECT_FALSE(info.isMaintenance);
    EXPECT_TRUE(info.message.empty());
    EXPECT_TRUE(info.posterPath.empty());
    EXPECT_TRUE(info.openLinkCmd.empty());
}

// Test MaintenanceInfo with values
TEST_F(MaintenanceTest, MaintenanceInfoWithValues) {
    neko::core::MaintenanceInfo info{
        .isMaintenance = true,
        .message = "Server is under maintenance",
        .posterPath = "/tmp/poster.png",
        .openLinkCmd = "open https://example.com"
    };
    
    EXPECT_TRUE(info.isMaintenance);
    EXPECT_EQ(info.message, "Server is under maintenance");
    EXPECT_EQ(info.posterPath, "/tmp/poster.png");
    EXPECT_EQ(info.openLinkCmd, "open https://example.com");
}

// Test MaintenanceInfo copy and move
TEST_F(MaintenanceTest, MaintenanceInfoCopyAndMove) {
    neko::core::MaintenanceInfo original{
        .isMaintenance = true,
        .message = "Test message",
        .posterPath = "/path/to/poster",
        .openLinkCmd = "open link"
    };
    
    // Test copy
    neko::core::MaintenanceInfo copied = original;
    EXPECT_TRUE(copied.isMaintenance);
    EXPECT_EQ(copied.message, original.message);
    EXPECT_EQ(copied.posterPath, original.posterPath);
    EXPECT_EQ(copied.openLinkCmd, original.openLinkCmd);
    
    // Test move
    neko::core::MaintenanceInfo moved = std::move(original);
    EXPECT_TRUE(moved.isMaintenance);
    EXPECT_EQ(moved.message, "Test message");
    EXPECT_EQ(moved.posterPath, "/path/to/poster");
    EXPECT_EQ(moved.openLinkCmd, "open link");
}

// Test MaintenanceInfo message formatting
TEST_F(MaintenanceTest, MaintenanceInfoMessageFormatting) {
    neko::core::MaintenanceInfo info;
    info.isMaintenance = true;
    
    std::string startTime = "2025-12-02 10:00:00";
    std::string endTime = "2025-12-02 14:00:00";
    std::string description = "System upgrade in progress";
    
    info.message = "Maintenance scheduled from " + startTime + 
                   " to " + endTime + 
                   ". Reason: " + description;
    
    EXPECT_FALSE(info.message.empty());
    EXPECT_TRUE(info.message.find(startTime) != std::string::npos);
    EXPECT_TRUE(info.message.find(endTime) != std::string::npos);
    EXPECT_TRUE(info.message.find(description) != std::string::npos);
}

// Test MaintenanceInfo with multiline message
TEST_F(MaintenanceTest, MaintenanceInfoMultilineMessage) {
    neko::core::MaintenanceInfo info{
        .isMaintenance = true,
        .message = "Line 1\nLine 2\nLine 3"
    };
    
    EXPECT_TRUE(info.message.find('\n') != std::string::npos);
    
    // Count newlines
    int newlineCount = 0;
    for (char c : info.message) {
        if (c == '\n') newlineCount++;
    }
    EXPECT_EQ(newlineCount, 2);
}

// Test MaintenanceInfo with special characters
TEST_F(MaintenanceTest, MaintenanceInfoSpecialCharacters) {
    neko::core::MaintenanceInfo info{
        .isMaintenance = true,
        .message = "維護中 - Maintenance 🔧",
        .openLinkCmd = "start https://example.com?param=value&other=123"
    };
    
    EXPECT_FALSE(info.message.empty());
    EXPECT_FALSE(info.openLinkCmd.empty());
}

// Test MaintenanceInfo equality
TEST_F(MaintenanceTest, MaintenanceInfoComparison) {
    neko::core::MaintenanceInfo info1{
        .isMaintenance = true,
        .message = "Test",
        .posterPath = "/path",
        .openLinkCmd = "cmd"
    };
    
    neko::core::MaintenanceInfo info2{
        .isMaintenance = true,
        .message = "Test",
        .posterPath = "/path",
        .openLinkCmd = "cmd"
    };
    
    neko::core::MaintenanceInfo info3{
        .isMaintenance = false,
        .message = "Different",
        .posterPath = "/other",
        .openLinkCmd = "other_cmd"
    };
    
    // Manual comparison since there's no operator==
    EXPECT_EQ(info1.isMaintenance, info2.isMaintenance);
    EXPECT_EQ(info1.message, info2.message);
    EXPECT_NE(info1.isMaintenance, info3.isMaintenance);
    EXPECT_NE(info1.message, info3.message);
}

// Test MaintenanceInfo with empty poster path
TEST_F(MaintenanceTest, MaintenanceInfoEmptyPosterPath) {
    neko::core::MaintenanceInfo info{
        .isMaintenance = true,
        .message = "Maintenance active",
        .posterPath = "",
        .openLinkCmd = "open https://status.example.com"
    };
    
    EXPECT_TRUE(info.isMaintenance);
    EXPECT_TRUE(info.posterPath.empty());
    EXPECT_FALSE(info.openLinkCmd.empty());
}

// Test MaintenanceInfo with empty link command
TEST_F(MaintenanceTest, MaintenanceInfoEmptyLinkCommand) {
    neko::core::MaintenanceInfo info{
        .isMaintenance = true,
        .message = "Maintenance scheduled",
        .posterPath = "/tmp/poster.png",
        .openLinkCmd = ""
    };
    
    EXPECT_TRUE(info.isMaintenance);
    EXPECT_FALSE(info.posterPath.empty());
    EXPECT_TRUE(info.openLinkCmd.empty());
}
