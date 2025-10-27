#define NOMINMAX
#include <gtest/gtest.h>
#include "neko/bus/threadBus.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <algorithm>

using namespace neko::bus::thread;

class ThreadBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset thread pool to default state
        setThreadCount(std::thread::hardware_concurrency());
    }

    void TearDown() override {
        waitForGlobalTasks();
    }
};

// === Basic Task Submission Tests ===

TEST_F(ThreadBusTest, SubmitSimpleTask) {
    std::atomic<bool> taskExecuted{false};
    
    auto future = submit([&taskExecuted]() {
        taskExecuted = true;
        return 42;
    });
    
    ASSERT_EQ(future.get(), 42);
    ASSERT_TRUE(taskExecuted);
}

TEST_F(ThreadBusTest, SubmitTaskWithArguments) {
    auto future = submit([](int a, int b) {
        return a + b;
    }, 10, 20);
    
    ASSERT_EQ(future.get(), 30);
}

TEST_F(ThreadBusTest, SubmitMultipleTasks) {
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 100; ++i) {
        futures.push_back(submit([&counter]() {
            counter++;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    ASSERT_EQ(counter, 100);
}

// === Priority Task Submission Tests ===

TEST_F(ThreadBusTest, SubmitWithHighPriority) {
    auto future = submitWithPriority(neko::Priority::High, []() {
        return "high priority task";
    });
    
    ASSERT_STREQ(future.get(), "high priority task");
}

TEST_F(ThreadBusTest, SubmitWithDifferentPriorities) {
    std::atomic<int> executionOrder{0};
    std::vector<int> order;
    std::mutex orderMutex;
    
    // Submit low priority task first
    auto lowFuture = submitWithPriority(neko::Priority::Low, [&]() {
        std::lock_guard<std::mutex> lock(orderMutex);
        order.push_back(1);
    });
    
    // Submit high priority task
    auto highFuture = submitWithPriority(neko::Priority::High, [&]() {
        std::lock_guard<std::mutex> lock(orderMutex);
        order.push_back(2);
    });
    
    lowFuture.get();
    highFuture.get();
    
    // Note: Priority ordering is not guaranteed in all cases
    ASSERT_EQ(order.size(), 2);
}

// === Worker-Specific Task Submission Tests ===

TEST_F(ThreadBusTest, SubmitToSpecificWorker) {
    auto workerIds = getWorkerIds();
    ASSERT_FALSE(workerIds.empty());
    
    auto future = submitToWorker(workerIds[0], []() {
        return std::this_thread::get_id();
    });
    
    auto threadId = future.get();
    ASSERT_NE(threadId, std::thread::id());
}

TEST_F(ThreadBusTest, SubmitToMultipleWorkers) {
    auto workerIds = getWorkerIds();
    ASSERT_FALSE(workerIds.empty());
    
    std::vector<std::future<std::thread::id>> futures;
    for (auto workerId : workerIds) {
        futures.push_back(submitToWorker(workerId, []() {
            return std::this_thread::get_id();
        }));
    }
    
    for (auto& future : futures) {
        auto threadId = future.get();
        ASSERT_NE(threadId, std::thread::id());
    }
}

// === Thread Information Tests ===

TEST_F(ThreadBusTest, GetThreadCount) {
    auto count = getThreadCount();
    ASSERT_GT(count, 0);
    ASSERT_LE(count, std::thread::hardware_concurrency() * 2);
}

TEST_F(ThreadBusTest, GetWorkerIds) {
    auto workerIds = getWorkerIds();
    ASSERT_FALSE(workerIds.empty());
    ASSERT_EQ(workerIds.size(), getThreadCount());
}

TEST_F(ThreadBusTest, GetPendingTaskCount) {
    auto count = getPendingTaskCount();
    ASSERT_GE(count, 0);
}

TEST_F(ThreadBusTest, GetMaxQueueSize) {
    auto maxSize = getMaxQueueSize();
    ASSERT_GT(maxSize, 0);
}

// === Queue Status Tests ===

TEST_F(ThreadBusTest, IsQueueFull) {
    auto isFull = isQueueFull();
    ASSERT_FALSE(isFull);  // Should not be full initially
}

TEST_F(ThreadBusTest, GetQueueUtilization) {
    auto utilization = getQueueUtilization();
    ASSERT_GE(utilization, 0.0);
    ASSERT_LE(utilization, 1.0);
}

TEST_F(ThreadBusTest, GetThreadUtilization) {
    auto utilization = getThreadUtilization();
    ASSERT_GE(utilization, 0.0);
    ASSERT_LE(utilization, 1.0);
}

// === Control Tests ===

TEST_F(ThreadBusTest, WaitForGlobalTasks) {
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 10; ++i) {
        submit([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
        });
    }
    
    waitForGlobalTasks();
    ASSERT_EQ(counter, 10);
}

TEST_F(ThreadBusTest, WaitForGlobalTasksWithTimeout) {
    submit([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });
    
    auto result = waitForGlobalTasks(std::chrono::milliseconds(10));
    ASSERT_FALSE(result);  // Should timeout
    
    waitForGlobalTasks();  // Wait for completion
}

TEST_F(ThreadBusTest, SetThreadCount) {
    auto originalCount = getThreadCount();
    neko::uint64 newCount = (originalCount > 4) ? (originalCount / 2) : 2;
    
    setThreadCount(newCount);
    ASSERT_EQ(getThreadCount(), newCount);
    
    // Restore original count
    setThreadCount(originalCount);
    ASSERT_EQ(getThreadCount(), originalCount);
}

TEST_F(ThreadBusTest, SetMaxQueueSize) {
    auto originalSize = getMaxQueueSize();
    neko::uint64 newSize = 500;
    
    setMaxQueueSize(newSize);
    ASSERT_EQ(getMaxQueueSize(), newSize);
    
    // Restore original size
    setMaxQueueSize(originalSize);
}

TEST_F(ThreadBusTest, StopThreadPool) {
    std::atomic<bool> taskExecuted{false};
    
    submit([&taskExecuted]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        taskExecuted = true;
    });
    
    stop(true);  // Wait for completion
    ASSERT_TRUE(taskExecuted);
    
    // Note: After stopping, we cannot restart the thread pool in this test
    // The TearDown will handle cleanup
}

// === Exception Handling Tests ===

TEST_F(ThreadBusTest, TaskWithException) {
    auto future = submit([]() -> int {
        throw std::runtime_error("Test exception");
    });
    
    ASSERT_THROW(future.get(), std::runtime_error);
}

// === Performance Tests ===

TEST_F(ThreadBusTest, HighVolumeTaskSubmission) {
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;
    
    const int taskCount = 1000;
    for (int i = 0; i < taskCount; ++i) {
        futures.push_back(submit([&counter]() {
            counter++;
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    ASSERT_EQ(counter, taskCount);
}

TEST_F(ThreadBusTest, ConcurrentTaskExecution) {
    auto startTime = std::chrono::steady_clock::now();
    
    std::vector<std::future<void>> futures;
    const int taskCount = 10;
    for (int i = 0; i < taskCount; ++i) {
        futures.push_back(submit([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // With parallel execution, should take much less than sequential time
    ASSERT_LT(duration.count(), taskCount * 100);
}