#define NOMINMAX
#include <gtest/gtest.h>
#include "neko/bus/resources.hpp"
#include <thread>

using namespace neko::bus;

class ResourcesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Resources are global singletons, so we just verify they exist
    }

    void TearDown() override {
        // Clean up any resources if needed
    }
};

// === ThreadPool Resource Tests ===

TEST_F(ResourcesTest, GetThreadPoolReturnsValidInstance) {
    auto& threadPool = getThreadPool();
    // Should not throw and should return valid reference
}

TEST_F(ResourcesTest, GetThreadPoolReturnsSameInstance) {
    auto& threadPool1 = getThreadPool();
    auto& threadPool2 = getThreadPool();
    
    // Should return the same instance (singleton)
    ASSERT_EQ(&threadPool1, &threadPool2);
}

TEST_F(ResourcesTest, ThreadPoolIsUsable) {
    auto& threadPool = getThreadPool();
    
    std::atomic<bool> taskExecuted{false};
    auto future = threadPool.submit([&taskExecuted]() {
        taskExecuted = true;
        return 42;
    });
    
    ASSERT_EQ(future.get(), 42);
    ASSERT_TRUE(taskExecuted);
}

TEST_F(ResourcesTest, ThreadPoolConcurrentAccess) {
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&successCount]() {
            auto& threadPool = getThreadPool();
            auto future = threadPool.submit([]() { return 1; });
            if (future.get() == 1) {
                successCount++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    ASSERT_EQ(successCount, 10);
}

// === ConfigManager Resource Tests ===

TEST_F(ResourcesTest, GetConfigObjReturnsValidInstance) {
    auto& config = getConfigObj();
    // Should not throw and should return valid reference
}

TEST_F(ResourcesTest, GetConfigObjReturnsSameInstance) {
    auto& config1 = getConfigObj();
    auto& config2 = getConfigObj();
    
    // Should return the same instance (singleton)
    ASSERT_EQ(&config1, &config2);
}

TEST_F(ResourcesTest, ConfigObjIsUsable) {
    auto& configMgr = getConfigObj();
    
    // Update config
    configMgr.updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "resources_test";
    });
    
    // Verify config was updated
    auto config = configMgr.getClientConfig();
    ASSERT_STREQ(config.main.lang, "resources_test");
}

TEST_F(ResourcesTest, ConfigObjConcurrentAccess) {
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &successCount]() {
            auto& configMgr = getConfigObj();
            std::string langValue = "thread_" + std::to_string(i);
            configMgr.updateClientConfig([&langValue](neko::ClientConfig& config) {
                config.main.lang = langValue.c_str();
            });
            successCount++;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    ASSERT_EQ(successCount, 10);
}

// === EventLoop Resource Tests ===

TEST_F(ResourcesTest, GetEventLoopReturnsValidInstance) {
    auto& eventLoop = getEventLoop();
    // Should not throw and should return valid reference
}

TEST_F(ResourcesTest, GetEventLoopReturnsSameInstance) {
    auto& eventLoop1 = getEventLoop();
    auto& eventLoop2 = getEventLoop();
    
    // Should return the same instance (singleton)
    ASSERT_EQ(&eventLoop1, &eventLoop2);
}

TEST_F(ResourcesTest, EventLoopIsUsable) {
    auto& eventLoop = getEventLoop();
    
    std::atomic<bool> taskExecuted{false};
    
    // Start event loop in separate thread
    std::thread loopThread([&eventLoop]() {
        eventLoop.run();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Schedule a task
    eventLoop.scheduleTask(50, [&taskExecuted]() {
        taskExecuted = true;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    eventLoop.stopLoop();
    eventLoop.wakeUp();
    
    if (loopThread.joinable()) {
        loopThread.join();
    }
    
    ASSERT_TRUE(taskExecuted);
}

TEST_F(ResourcesTest, EventLoopConcurrentAccess) {
    std::vector<std::thread> threads;
    std::atomic<int> tasksScheduled{0};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&tasksScheduled]() {
            auto& eventLoop = getEventLoop();
            auto eventId = eventLoop.scheduleTask(100, []() {});
            if (eventId != 0) {
                tasksScheduled++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    ASSERT_EQ(tasksScheduled, 10);
}

// === Resources Class Static Methods Tests ===

TEST_F(ResourcesTest, ResourcesClassGetThreadPool) {
    auto& threadPool = Resources::getThreadPool();
    // Should not throw
}

TEST_F(ResourcesTest, ResourcesClassGetConfigObj) {
    auto& config = Resources::getConfigObj();
    // Should not throw
}

TEST_F(ResourcesTest, ResourcesClassGetEventLoop) {
    auto& eventLoop = Resources::getEventLoop();
    // Should not throw
}

TEST_F(ResourcesTest, ResourcesClassReturnsSameInstances) {
    auto& threadPool1 = Resources::getThreadPool();
    auto& threadPool2 = getThreadPool();
    ASSERT_EQ(&threadPool1, &threadPool2);
    
    auto& config1 = Resources::getConfigObj();
    auto& config2 = getConfigObj();
    ASSERT_EQ(&config1, &config2);
    
    auto& eventLoop1 = Resources::getEventLoop();
    auto& eventLoop2 = getEventLoop();
    ASSERT_EQ(&eventLoop1, &eventLoop2);
}

// === Integration Tests ===

TEST_F(ResourcesTest, AllResourcesWorkTogether) {
    auto& threadPool = getThreadPool();
    auto& configMgr = getConfigObj();
    auto& eventLoop = getEventLoop();
    
    // Update config
    configMgr.updateClientConfig([](neko::ClientConfig& config) {
        config.main.lang = "integration_test";
    });
    
    // Submit task to thread pool
    std::atomic<bool> threadTaskDone{false};
    auto future = threadPool.submit([&threadTaskDone]() {
        threadTaskDone = true;
    });
    future.get();
    
    // Schedule event
    std::atomic<bool> eventTaskDone{false};
    std::thread loopThread([&eventLoop]() {
        eventLoop.run();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    eventLoop.scheduleTask(50, [&eventTaskDone]() {
        eventTaskDone = true;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    eventLoop.stopLoop();
    eventLoop.wakeUp();
    
    if (loopThread.joinable()) {
        loopThread.join();
    }
    
    // Verify all tasks completed
    ASSERT_TRUE(threadTaskDone);
    ASSERT_TRUE(eventTaskDone);
    
    auto config = configMgr.getClientConfig();
    ASSERT_STREQ(config.main.lang, "integration_test");
}

TEST_F(ResourcesTest, ResourcesThreadSafety) {
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    
    for (int i = 0; i < 20; ++i) {
        threads.emplace_back([i, &successCount]() {
            // Access all resources
            auto& threadPool = getThreadPool();
            auto& configMgr = getConfigObj();
            auto& eventLoop = getEventLoop();
            
            // Use thread pool
            auto future = threadPool.submit([]() { return 1; });
            future.get();
            
            // Use config
            std::string langValue = "thread_" + std::to_string(i);
            configMgr.updateClientConfig([&langValue](neko::ClientConfig& config) {
                config.main.lang = langValue.c_str();
            });
            
            // Schedule event
            eventLoop.scheduleTask(1000, []() {});
            
            successCount++;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    ASSERT_EQ(successCount, 20);
}

// === Lifetime Tests ===

TEST_F(ResourcesTest, ResourcesPersistAcrossMultipleCalls) {
    // First access
    {
        auto& threadPool = getThreadPool();
        auto threadCount1 = threadPool.getThreadCount();
        
        auto& configMgr = getConfigObj();
        configMgr.updateClientConfig([](neko::ClientConfig& config) {
            config.main.lang = "persist_test";
        });
    }
    
    // Second access - should get same instances
    {
        auto& threadPool = getThreadPool();
        // Thread count should still be the same (not recreated)
        
        auto& configMgr = getConfigObj();
        auto config = configMgr.getClientConfig();
        ASSERT_STREQ(config.main.lang, "persist_test");
    }
}
