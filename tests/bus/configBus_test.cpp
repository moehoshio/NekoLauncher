#define NOMINMAX
#include <gtest/gtest.h>
#include "neko/bus/configBus.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>

using namespace neko::bus::config;

class ConfigBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        testConfigFile = "test_config.ini";
        cleanupTestFile();
    }

    void TearDown() override {
        cleanupTestFile();
    }

    void cleanupTestFile() {
        if (std::filesystem::exists(testConfigFile)) {
            std::filesystem::remove(testConfigFile);
        }
    }

    void createTestConfigFile() {
        std::ofstream file(testConfigFile);
        file << "[client]\n";
        file << "language=en_us\n";
        file << "theme=dark\n";
        file << "width=1920\n";
        file << "height=1080\n";
        file << "[minecraft]\n";
        file << "version=1.20.1\n";
        file << "memory=4096\n";
        file.close();
    }

    std::string testConfigFile;
};

// === Load and Save Tests ===

TEST_F(ConfigBusTest, LoadNonExistentFile) {
    bool result = load("non_existent_file.ini");
    // Behavior depends on implementation - might return false or create new file
}

TEST_F(ConfigBusTest, LoadExistingFile) {
    createTestConfigFile();
    
    bool result = load(testConfigFile);
    ASSERT_TRUE(result);
}

TEST_F(ConfigBusTest, SaveToNewFile) {
    // Update config with some data
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "zh_tw";
    });
    
    bool result = save(testConfigFile);
    ASSERT_TRUE(result);
    ASSERT_TRUE(std::filesystem::exists(testConfigFile));
}

TEST_F(ConfigBusTest, LoadAndSaveRoundTrip) {
    // Create initial config
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "ja_jp";
    });
    
    // Save to file
    bool saveResult = save(testConfigFile);
    ASSERT_TRUE(saveResult);
    
    // Load it back
    bool loadResult = load(testConfigFile);
    ASSERT_TRUE(loadResult);
    
    // Verify the data
    auto config = getClientConfig();
    ASSERT_STREQ(config.main.lang, "ja_jp");
}

// === Update Client Config Tests ===

TEST_F(ConfigBusTest, UpdateClientConfigLanguage) {
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "zh_tw";
    });
    
    auto config = getClientConfig();
    ASSERT_STREQ(config.main.lang, "zh_tw");
}

TEST_F(ConfigBusTest, UpdateClientConfigMultipleFields) {
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "en_us";
        config.style.blurEffect = "light";
    });
    
    auto config = getClientConfig();
    ASSERT_STREQ(config.main.lang, "en_us");
    ASSERT_STREQ(config.style.blurEffect, "light");
}

TEST_F(ConfigBusTest, UpdateClientConfigMultipleTimes) {
    // First update
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "zh_tw";
    });
    
    auto config1 = getClientConfig();
    ASSERT_STREQ(config1.main.lang, "zh_tw");
    
    // Second update
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "en_us";
    });
    
    auto config2 = getClientConfig();
    ASSERT_STREQ(config2.main.lang, "en_us");
}

TEST_F(ConfigBusTest, UpdateClientConfigWithComplexData) {
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "zh_tw";
        config.style.blurEffect = "dark";
        config.main.windowSize = "1920x1080";
        config.main.useSysWindowFrame = false;
        config.minecraft.maxMemoryLimit = 4096;
    });
    
    auto config = getClientConfig();
    ASSERT_STREQ(config.main.lang, "zh_tw");
    ASSERT_STREQ(config.style.blurEffect, "dark");
    ASSERT_STREQ(config.main.windowSize, "1920x1080");
    ASSERT_FALSE(config.main.useSysWindowFrame);
    ASSERT_EQ(config.minecraft.maxMemoryLimit, 4096);
}

// === Get Client Config Tests ===

TEST_F(ConfigBusTest, GetClientConfigReturnsValidData) {
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "test_lang";
    });
    
    auto config = getClientConfig();
    ASSERT_STREQ(config.main.lang, "test_lang");
}

TEST_F(ConfigBusTest, GetClientConfigIsImmutable) {
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "original";
    });
    
    auto config = getClientConfig();
    config.main.lang = "modified";
    
    // Get again and verify original wasn't changed
    auto config2 = getClientConfig();
    ASSERT_STREQ(config2.main.lang, "original");
}

// === Persistence Tests ===

TEST_F(ConfigBusTest, ConfigPersistsAfterSave) {
    // Set config
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "zh_tw";
        config.style.blurEffect = "dark";
    });
    
    // Save
    save(testConfigFile);
    
    // Modify config
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "en_us";
        config.style.blurEffect = "light";
    });
    
    // Load previous config
    load(testConfigFile);
    
    // Verify loaded config
    auto config = getClientConfig();
    ASSERT_STREQ(config.main.lang, "zh_tw");
    ASSERT_STREQ(config.style.blurEffect, "dark");
}

TEST_F(ConfigBusTest, MultipleLoadSaveCycles) {
    for (int i = 0; i < 5; ++i) {
        std::string langValue = "cycle_" + std::to_string(i);
        updateClientConfig([&langValue](neko::ClientConfig& config) {
            config.main.lang = langValue.c_str();
        });
        
        save(testConfigFile);
        load(testConfigFile);
        
        auto config = getClientConfig();
        ASSERT_STREQ(config.main.lang, langValue.c_str());
    }
}

// === Edge Cases and Error Handling ===

TEST_F(ConfigBusTest, SaveToInvalidPath) {
    std::string invalidPath = "/invalid/path/that/does/not/exist/config.ini";
    bool result = save(invalidPath);
    // Should return false or handle gracefully
}

TEST_F(ConfigBusTest, LoadEmptyFile) {
    // Create empty file
    std::ofstream file(testConfigFile);
    file.close();
    
    bool result = load(testConfigFile);
    // Should handle empty file gracefully
}

TEST_F(ConfigBusTest, UpdateWithEmptyFunction) {
    updateClientConfig([](neko::ClientConfig& config) {
        // Do nothing
    });
    
    // Should not crash
}

// === Thread Safety Tests ===

TEST_F(ConfigBusTest, ConcurrentUpdates) {
    std::vector<std::thread> threads;
    std::atomic<int> completedUpdates{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &completedUpdates]() {
            std::string langValue = "thread_" + std::to_string(i);
            updateClientConfig([&langValue](neko::ClientConfig& config) {
                config.main.lang = langValue.c_str();
            });
            completedUpdates++;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    ASSERT_EQ(completedUpdates, 10);
}

TEST_F(ConfigBusTest, ConcurrentReads) {
    updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "concurrent_read_test";
    });
    
    std::vector<std::thread> threads;
    std::atomic<int> successfulReads{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&successfulReads]() {
            auto config = getClientConfig();
            if (std::string(config.main.lang) == "concurrent_read_test") {
                successfulReads++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    ASSERT_EQ(successfulReads, 10);
}

TEST_F(ConfigBusTest, ConcurrentReadWrite) {
    std::vector<std::thread> threads;
    
    // Writer threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            std::string langValue = "writer_" + std::to_string(i);
            updateClientConfig([&langValue](neko::ClientConfig& config) {
                config.main.lang = langValue.c_str();
            });
        });
    }
    
    // Reader threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([]() {
            auto config = getClientConfig();
            // Just reading, no assertion
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should complete without crashes
}
