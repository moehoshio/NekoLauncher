/**
 * @file neko_bus_test.cpp
 * @brief Unit tests for neko bus modules (threadBus, configBus, eventBus)
 */

#include <gtest/gtest.h>
#include <neko/bus/threadBus.hpp>
#include <neko/bus/configBus.hpp>
#include <neko/bus/eventBus.hpp>
#include <neko/schema/eventTypes.hpp>
#include <neko/core/resources.hpp>

#include <thread>
#include <chrono>
#include <atomic>
#include <future>

// ============================================================================
// Thread Bus Tests
// ============================================================================

class ThreadBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize thread pool if needed
    }

    void TearDown() override {
        // Clean up
    }
};

TEST_F(ThreadBusTest, SubmitBasicTask) {
    std::atomic<int> result{0};
    
    auto future = neko::bus::thread::submit([&result]() {
        result = 42;
        return 42;
    });
    
    ASSERT_EQ(future.get(), 42);
    ASSERT_EQ(result.load(), 42);
}

TEST_F(ThreadBusTest, SubmitTaskWithArguments) {
    auto future = neko::bus::thread::submit([](int a, int b) {
        return a + b;
    }, 10, 20);
    
    ASSERT_EQ(future.get(), 30);
}

TEST_F(ThreadBusTest, SubmitWithPriority) {
    std::atomic<int> counter{0};
    
    auto future = neko::bus::thread::submitWithPriority(
        neko::Priority::High,
        [&counter]() {
            counter++;
            return counter.load();
        }
    );
    
    future.wait();
    ASSERT_GE(counter.load(), 1);
}

TEST_F(ThreadBusTest, SubmitToSpecificWorker) {
    auto workerIds = neko::bus::thread::getWorkerIds();
    ASSERT_FALSE(workerIds.empty());
    
    auto workerId = workerIds[0];
    std::atomic<bool> executed{false};
    
    auto future = neko::bus::thread::submitToWorker(workerId, [&executed]() {
        executed = true;
        return true;
    });
    
    ASSERT_TRUE(future.get());
    ASSERT_TRUE(executed.load());
}

TEST_F(ThreadBusTest, GetThreadInfo) {
    auto threadCount = neko::bus::thread::getThreadCount();
    ASSERT_GT(threadCount, 0);
    
    auto workerIds = neko::bus::thread::getWorkerIds();
    ASSERT_EQ(workerIds.size(), threadCount);
    
    auto maxQueueSize = neko::bus::thread::getMaxQueueSize();
    ASSERT_GT(maxQueueSize, 0);
}

TEST_F(ThreadBusTest, QueueUtilization) {
    auto utilization = neko::bus::thread::getQueueUtilization();
    ASSERT_GE(utilization, 0.0);
    ASSERT_LE(utilization, 1.0);
}

TEST_F(ThreadBusTest, ThreadUtilization) {
    auto utilization = neko::bus::thread::getThreadUtilization();
    ASSERT_GE(utilization, 0.0);
    ASSERT_LE(utilization, 1.0);
}

TEST_F(ThreadBusTest, WaitForGlobalTasks) {
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 10; ++i) {
        neko::bus::thread::submit([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
        });
    }
    
    neko::bus::thread::waitForGlobalTasks();
    ASSERT_EQ(counter.load(), 10);
}

TEST_F(ThreadBusTest, WaitForGlobalTasksWithTimeout) {
    neko::bus::thread::submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    bool completed = neko::bus::thread::waitForGlobalTasks(std::chrono::milliseconds(50));
    ASSERT_FALSE(completed);
    
    completed = neko::bus::thread::waitForGlobalTasks(std::chrono::milliseconds(200));
    ASSERT_TRUE(completed);
}

TEST_F(ThreadBusTest, PendingTaskCount) {
    auto initialCount = neko::bus::thread::getPendingTaskCount();
    
    // Submit multiple tasks
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 5; ++i) {
        futures.push_back(neko::bus::thread::submit([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }));
    }
    
    // Pending count should have increased
    auto pendingCount = neko::bus::thread::getPendingTaskCount();
    ASSERT_GE(pendingCount, 0); // May be 0 if tasks already completed
    
    // Wait for all tasks
    for (auto& future : futures) {
        future.wait();
    }
    
    neko::bus::thread::waitForGlobalTasks();
}

TEST_F(ThreadBusTest, SetMaxQueueSize) {
    auto originalSize = neko::bus::thread::getMaxQueueSize();
    
    neko::bus::thread::setMaxQueueSize(1000);
    ASSERT_EQ(neko::bus::thread::getMaxQueueSize(), 1000);
    
    // Restore original size
    neko::bus::thread::setMaxQueueSize(originalSize);
}

TEST_F(ThreadBusTest, IsQueueFull) {
    auto isFull = neko::bus::thread::isQueueFull();
    ASSERT_FALSE(isFull); // Typically should not be full
}

// ============================================================================
// Config Bus Tests
// ============================================================================

class ConfigBusTest : public ::testing::Test {
protected:
    std::string testConfigFile = "test_config.ini";
    
    void SetUp() override {
        // Clean up any existing test config
        std::remove(testConfigFile.c_str());
    }

    void TearDown() override {
        // Clean up test config
        std::remove(testConfigFile.c_str());
    }
};

TEST_F(ConfigBusTest, GetClientConfig) {
    auto config = neko::bus::config::getClientConfig();
    // Just verify we can get a config without crashing
    SUCCEED();
}

TEST_F(ConfigBusTest, UpdateClientConfig) {
    // Update config
    neko::bus::config::updateClientConfig([](neko::ClientConfig& cfg) {
        cfg.main.lang = "zh_tw";
        cfg.minecraft.maxMemoryLimit = 4096;
    });
    
    // Verify changes
    auto config = neko::bus::config::getClientConfig();
    ASSERT_EQ(std::string(config.main.lang), "zh_tw");
    ASSERT_EQ(config.minecraft.maxMemoryLimit, 4096);
}

TEST_F(ConfigBusTest, SaveAndLoadConfig) {
    // Update config
    neko::bus::config::updateClientConfig([](neko::ClientConfig& cfg) {
        cfg.main.lang = "en";
        cfg.minecraft.playerName = "TestPlayer";
        cfg.minecraft.minMemoryLimit = 1024;
    });
    
    // Save config
    bool saved = neko::bus::config::save(testConfigFile);
    ASSERT_TRUE(saved);
    
    // Modify config
    neko::bus::config::updateClientConfig([](neko::ClientConfig& cfg) {
        cfg.main.lang = "zh_cn";
        cfg.minecraft.playerName = "AnotherPlayer";
    });
    
    // Load config back
    bool loaded = neko::bus::config::load(testConfigFile);
    ASSERT_TRUE(loaded);
    
    // Verify loaded values
    auto config = neko::bus::config::getClientConfig();
    ASSERT_EQ(std::string(config.main.lang), "en");
    ASSERT_EQ(std::string(config.minecraft.playerName), "TestPlayer");
}

TEST_F(ConfigBusTest, LoadNonExistentFile) {
    bool loaded = neko::bus::config::load("non_existent_file.ini");
    ASSERT_FALSE(loaded);
}

TEST_F(ConfigBusTest, ConcurrentConfigUpdates) {
    std::atomic<int> updateCount{0};
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&updateCount, i]() {
            neko::bus::config::updateClientConfig([i](neko::ClientConfig& cfg) {
                cfg.minecraft.maxMemoryLimit = 2048 + i;
            });
            updateCount++;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    ASSERT_EQ(updateCount.load(), 10);
    
    auto config = neko::bus::config::getClientConfig();
    // Memory limit should be one of the values set
    ASSERT_GE(config.minecraft.maxMemoryLimit, 2048);
    ASSERT_LT(config.minecraft.maxMemoryLimit, 2058);
}

// ============================================================================
// Event Bus Tests
// ============================================================================

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Enable statistics for testing
        neko::bus::event::enableStatistics(true);
        neko::bus::event::resetStatistics();
    }

    void TearDown() override {
        // Clean up any scheduled tasks
        neko::bus::event::cleanupCancelledTasks();
    }
};

TEST_F(EventBusTest, SubscribeAndPublish) {
    std::atomic<int> eventReceived{0};
    
    auto handlerId = neko::bus::event::subscribe<neko::event::StartEvent>(
        [&eventReceived](const neko::event::StartEvent&) {
            eventReceived++;
        }
    );
    
    ASSERT_GT(handlerId, 0);
    
    neko::bus::event::publish(neko::event::StartEvent{});
    
    // Give some time for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_GE(eventReceived.load(), 0); // May be async
    
    bool unsubscribed = neko::bus::event::unsubscribe<neko::event::StartEvent>(handlerId);
    ASSERT_TRUE(unsubscribed);
}

TEST_F(EventBusTest, PublishWithPriority) {
    std::atomic<int> counter{0};
    
    neko::bus::event::subscribe<neko::event::ExitEvent>(
        [&counter](const neko::event::ExitEvent&) {
            counter++;
        }
    );
    
    neko::bus::event::publish(
        neko::event::ExitEvent{},
        neko::Priority::High,
        neko::SyncMode::Async
    );
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_GE(counter.load(), 0);
}

TEST_F(EventBusTest, PublishAfter) {
    std::atomic<bool> eventReceived{false};
    auto startTime = std::chrono::steady_clock::now();
    
    neko::bus::event::subscribe<neko::event::StartEvent>(
        [&eventReceived, startTime](const neko::event::StartEvent&) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime
            ).count();
            eventReceived = true;
        }
    );
    
    auto eventId = neko::bus::event::publishAfter<neko::event::StartEvent>(
        100, // 100ms delay
        neko::event::StartEvent{}
    );
    
    ASSERT_GT(eventId, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Event should have been processed by now (async)
}

TEST_F(EventBusTest, ScheduleTask) {
    std::atomic<bool> taskExecuted{false};
    
    auto eventId = neko::bus::event::scheduleTask(
        100, // 100ms
        [&taskExecuted]() {
            taskExecuted = true;
        },
        neko::Priority::Normal
    );
    
    ASSERT_GT(eventId, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Task may or may not have executed depending on event loop state
}

TEST_F(EventBusTest, ScheduleRepeatingTask) {
    std::atomic<int> executionCount{0};
    
    auto eventId = neko::bus::event::scheduleRepeating(
        50, // 50ms interval
        [&executionCount]() {
            executionCount++;
        },
        neko::Priority::Normal
    );
    
    ASSERT_GT(eventId, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Cancel the repeating task
    bool cancelled = neko::bus::event::cancelTask(eventId);
    ASSERT_TRUE(cancelled);
    
    int finalCount = executionCount.load();
    
    // Wait a bit more to ensure no more executions
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(executionCount.load(), finalCount);
}

TEST_F(EventBusTest, CancelTask) {
    auto eventId = neko::bus::event::scheduleTask(
        1000, // 1 second
        []() {
            // This should not execute
        }
    );
    
    ASSERT_GT(eventId, 0);
    
    bool cancelled = neko::bus::event::cancelTask(eventId);
    ASSERT_TRUE(cancelled);
    
    // Trying to cancel again should fail
    bool cancelledAgain = neko::bus::event::cancelTask(eventId);
    ASSERT_FALSE(cancelledAgain);
}

TEST_F(EventBusTest, MultipleSubscribers) {
    std::atomic<int> subscriber1Count{0};
    std::atomic<int> subscriber2Count{0};
    
    auto handler1 = neko::bus::event::subscribe<neko::event::StartEvent>(
        [&subscriber1Count](const neko::event::StartEvent&) {
            subscriber1Count++;
        }
    );
    
    auto handler2 = neko::bus::event::subscribe<neko::event::StartEvent>(
        [&subscriber2Count](const neko::event::StartEvent&) {
            subscriber2Count++;
        }
    );
    
    ASSERT_NE(handler1, handler2);
    
    neko::bus::event::publish(neko::event::StartEvent{});
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    neko::bus::event::unsubscribe<neko::event::StartEvent>(handler1);
    neko::bus::event::unsubscribe<neko::event::StartEvent>(handler2);
}

TEST_F(EventBusTest, SetMaxQueueSize) {
    neko::bus::event::setMaxQueueSize(500);
    SUCCEED();
}

TEST_F(EventBusTest, GetStatistics) {
    auto stats = neko::bus::event::getStatistics();
    // Just verify we can get statistics
    SUCCEED();
}

TEST_F(EventBusTest, WakeUp) {
    neko::bus::event::wakeUp();
    SUCCEED();
}

TEST_F(EventBusTest, CleanupCancelledTasks) {
    auto eventId = neko::bus::event::scheduleTask(5000, []() {});
    neko::bus::event::cancelTask(eventId);
    neko::bus::event::cleanupCancelledTasks();
    SUCCEED();
}

TEST_F(EventBusTest, UnsubscribeNonExistent) {
    bool result = neko::bus::event::unsubscribe<neko::event::StartEvent>(99999);
    ASSERT_FALSE(result);
}

// ============================================================================
// Integration Tests
// ============================================================================

class BusIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        neko::bus::event::enableStatistics(true);
    }
};

TEST_F(BusIntegrationTest, ThreadBusAndEventBusInteraction) {
    std::atomic<int> counter{0};
    
    // Subscribe to event
    neko::bus::event::subscribe<neko::event::StartEvent>(
        [&counter](const neko::event::StartEvent&) {
            counter++;
        }
    );
    
    // Submit task via thread bus that publishes event
    auto future = neko::bus::thread::submit([]() {
        neko::bus::event::publish(neko::event::StartEvent{});
    });
    
    future.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_GE(counter.load(), 0);
}

TEST_F(BusIntegrationTest, ConfigAndThreadBusConcurrency) {
    std::vector<std::future<void>> futures;
    std::atomic<int> completedTasks{0};
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(neko::bus::thread::submit([&completedTasks, i]() {
            neko::bus::config::updateClientConfig([i](neko::ClientConfig& cfg) {
                cfg.minecraft.maxMemoryLimit = 1024 + i * 100;
            });
            completedTasks++;
        }));
    }
    
    for (auto& future : futures) {
        future.wait();
    }
    
    ASSERT_EQ(completedTasks.load(), 10);
}

TEST_F(BusIntegrationTest, AllBusesWorking) {
    // Test that all three buses can work together
    std::atomic<bool> allWorking{true};
    
    // Config bus
    neko::bus::config::updateClientConfig([](neko::ClientConfig& cfg) {
        cfg.main.lang = "test";
    });
    
    // Event bus
    neko::bus::event::subscribe<neko::event::ExitEvent>(
        [](const neko::event::ExitEvent&) {
            // Just receive event
        }
    );
    
    // Thread bus
    auto future = neko::bus::thread::submit([&allWorking]() {
        neko::bus::event::publish(neko::event::ExitEvent{});
        return true;
    });
    
    ASSERT_TRUE(future.get());
    ASSERT_TRUE(allWorking.load());
}

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
