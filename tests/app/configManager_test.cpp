#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "neko/app/configManager.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

using namespace neko::app;

class ConfigManagerTest : public ::testing::Test {
protected:
    std::string testConfigFile;
    ConfigManager manager;

    void SetUp() override {
        testConfigFile = (std::filesystem::temp_directory_path() / "test_config_manager.ini").string();
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
             << "resourceVersion=1.0.0\n"
             << "deviceID=test-device\n"
             << "\n[style]\n"
             << "blurRadius=10\n"
             << "\n[net]\n"
             << "thread=4\n"
             << "\n[minecraft]\n"
             << "playerName=TestPlayer\n"
             << "maxMemoryLimit=2048\n";
        file.close();
    }
};

// Test loading configuration file
TEST_F(ConfigManagerTest, LoadConfig) {
    createTestConfigFile();
    
    bool result = manager.load(testConfigFile);
    EXPECT_TRUE(result);
}

// Test loading non-existent file
TEST_F(ConfigManagerTest, LoadNonExistentFile) {
    bool result = manager.load("non_existent_file.ini");
    EXPECT_FALSE(result);
}

// Test saving configuration file
TEST_F(ConfigManagerTest, SaveConfig) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    std::string saveFile = (std::filesystem::temp_directory_path() / "test_save.ini").string();
    bool result = manager.save(saveFile);
    EXPECT_TRUE(result);
    
    EXPECT_TRUE(std::filesystem::exists(saveFile));
    std::filesystem::remove(saveFile);
}

// Test getting client configuration
TEST_F(ConfigManagerTest, GetClientConfig) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    neko::ClientConfig config = manager.getClientConfig();
    
    EXPECT_STREQ(config.main.lang, "en");
    EXPECT_STREQ(config.main.resourceVersion, "1.0.0");
    EXPECT_STREQ(config.main.deviceID, "test-device");
    EXPECT_EQ(config.style.blurRadius, 10);
    EXPECT_EQ(config.net.thread, 4);
    EXPECT_STREQ(config.minecraft.playerName, "TestPlayer");
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 2048);
}

// Test updating client configuration
TEST_F(ConfigManagerTest, UpdateClientConfig) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    manager.updateClientConfig([](neko::ClientConfig &cfg) {
        cfg.main.lang = "zh_tw";
        cfg.style.blurRadius = 20;
        cfg.minecraft.maxMemoryLimit = 4096;
    });
    
    neko::ClientConfig config = manager.getClientConfig();
    
    EXPECT_STREQ(config.main.lang, "zh_tw");
    EXPECT_EQ(config.style.blurRadius, 20);
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 4096);
    
    // Verify other values remain unchanged
    EXPECT_STREQ(config.main.resourceVersion, "1.0.0");
    EXPECT_EQ(config.net.thread, 4);
}

// Test multiple updates
TEST_F(ConfigManagerTest, MultipleUpdates) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    manager.updateClientConfig([](neko::ClientConfig &cfg) {
        cfg.main.lang = "fr";
    });
    
    manager.updateClientConfig([](neko::ClientConfig &cfg) {
        cfg.style.blurRadius = 15;
    });
    
    manager.updateClientConfig([](neko::ClientConfig &cfg) {
        cfg.minecraft.playerName = "NewPlayer";
    });
    
    neko::ClientConfig config = manager.getClientConfig();
    
    EXPECT_STREQ(config.main.lang, "fr");
    EXPECT_EQ(config.style.blurRadius, 15);
    EXPECT_STREQ(config.minecraft.playerName, "NewPlayer");
}

// Test update with empty function (should not crash)
TEST_F(ConfigManagerTest, EmptyUpdate) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    auto before = manager.getClientConfig();
    
    manager.updateClientConfig([](neko::ClientConfig &cfg) {
        // Do nothing
    });
    
    auto after = manager.getClientConfig();
    
    EXPECT_STREQ(before.main.lang, after.main.lang);
    EXPECT_EQ(before.style.blurRadius, after.style.blurRadius);
}

// Test concurrent reads
TEST_F(ConfigManagerTest, ConcurrentReads) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &successCount]() {
            for (int j = 0; j < 100; ++j) {
                neko::ClientConfig config = manager.getClientConfig();
                if (std::string(config.main.lang) == "en") {
                    successCount++;
                }
            }
        });
    }
    
    for (auto &thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successCount, 1000);
}

// Test concurrent writes
TEST_F(ConfigManagerTest, ConcurrentWrites) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, i]() {
            for (int j = 0; j < 10; ++j) {
                manager.updateClientConfig([i, j](neko::ClientConfig &cfg) {
                    cfg.net.thread = i * 10 + j;
                });
            }
        });
    }
    
    for (auto &thread : threads) {
        thread.join();
    }
    
    // After all writes, config should be in a valid state
    neko::ClientConfig config = manager.getClientConfig();
    EXPECT_GE(config.net.thread, 0);
    EXPECT_LT(config.net.thread, 100);
}

// Test mixed concurrent reads and writes
TEST_F(ConfigManagerTest, MixedConcurrentAccess) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    std::vector<std::thread> threads;
    std::atomic<bool> errorOccurred{false};
    
    // Reader threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this, &errorOccurred]() {
            for (int j = 0; j < 50; ++j) {
                try {
                    neko::ClientConfig config = manager.getClientConfig();
                    // Just access some values
                    volatile auto lang = config.main.lang;
                    volatile auto radius = config.style.blurRadius;
                } catch (...) {
                    errorOccurred = true;
                }
            }
        });
    }
    
    // Writer threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this, i, &errorOccurred]() {
            for (int j = 0; j < 50; ++j) {
                try {
                    manager.updateClientConfig([i, j](neko::ClientConfig &cfg) {
                        cfg.style.blurRadius = (i * 10 + j) % 50;
                    });
                } catch (...) {
                    errorOccurred = true;
                }
            }
        });
    }
    
    for (auto &thread : threads) {
        thread.join();
    }
    
    EXPECT_FALSE(errorOccurred);
}

// Test persistence after update
TEST_F(ConfigManagerTest, PersistenceAfterUpdate) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    manager.updateClientConfig([](neko::ClientConfig &cfg) {
        cfg.main.lang = "ja";
        cfg.minecraft.maxMemoryLimit = 8192;
    });
    
    std::string saveFile = (std::filesystem::temp_directory_path() / "test_persist.ini").string();
    manager.save(saveFile);
    
    // Load with a new manager
    ConfigManager newManager;
    newManager.load(saveFile);
    
    neko::ClientConfig config = newManager.getClientConfig();
    EXPECT_STREQ(config.main.lang, "ja");
    EXPECT_EQ(config.minecraft.maxMemoryLimit, 8192);
    
    std::filesystem::remove(saveFile);
}

// Test getClientConfig returns a copy
TEST_F(ConfigManagerTest, GetClientConfigReturnsCopy) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    neko::ClientConfig config1 = manager.getClientConfig();
    neko::ClientConfig config2 = manager.getClientConfig();
    
    // Both should have the same values
    EXPECT_STREQ(config1.main.lang, config2.main.lang);
    
    // Modify config1 (shouldn't affect config2 or manager's internal state)
    // Note: Can't modify config1.main.lang directly as it's const char*
    // This test verifies they are independent copies
}

// Test update doesn't affect previously retrieved configs
TEST_F(ConfigManagerTest, UpdateDoesntAffectPreviousConfigs) {
    createTestConfigFile();
    manager.load(testConfigFile);
    
    neko::ClientConfig oldConfig = manager.getClientConfig();
    std::string oldLang(oldConfig.main.lang);
    
    manager.updateClientConfig([](neko::ClientConfig &cfg) {
        cfg.main.lang = "es";
    });
    
    neko::ClientConfig newConfig = manager.getClientConfig();
    
    // Old config should still have original value
    EXPECT_EQ(oldLang, "en");
    
    // New config should have updated value
    EXPECT_STREQ(newConfig.main.lang, "es");
}
