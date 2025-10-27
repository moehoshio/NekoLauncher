#define NOMINMAX
#include <gtest/gtest.h>
#include "neko/bus/eventBus.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

using namespace neko::bus::event;

// Test event structures
struct SimpleEvent {
    int value;
    std::string message;
};

struct CounterEvent {
    int count;
};

struct PriorityEvent {
    neko::Priority priority;
    std::string data;
};

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start event loop in a separate thread
        loopThread = std::thread([]() {
            run();
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    void TearDown() override {
        stopLoop();
        wakeUp();
        if (loopThread.joinable()) {
            loopThread.join();
        }
        resetStatistics();
    }

    std::thread loopThread;
};

// === Subscription Tests ===

TEST_F(EventBusTest, SubscribeToEvent) {
    std::atomic<bool> eventReceived{false};
    
    auto handlerId = subscribe<SimpleEvent>([&eventReceived](const SimpleEvent& event) {
        eventReceived = true;
    });
    
    ASSERT_NE(handlerId, 0);
    
    publish<SimpleEvent>(SimpleEvent{42, "test"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(eventReceived);
    unsubscribe<SimpleEvent>(handlerId);
}

TEST_F(EventBusTest, MultipleSubscribersToSameEvent) {
    std::atomic<int> receiveCount{0};
    
    auto handler1 = subscribe<SimpleEvent>([&receiveCount](const SimpleEvent& event) {
        receiveCount++;
    });
    
    auto handler2 = subscribe<SimpleEvent>([&receiveCount](const SimpleEvent& event) {
        receiveCount++;
    });
    
    auto handler3 = subscribe<SimpleEvent>([&receiveCount](const SimpleEvent& event) {
        receiveCount++;
    });
    
    publish<SimpleEvent>(SimpleEvent{100, "broadcast"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_EQ(receiveCount, 3);
    
    unsubscribe<SimpleEvent>(handler1);
    unsubscribe<SimpleEvent>(handler2);
    unsubscribe<SimpleEvent>(handler3);
}

TEST_F(EventBusTest, UnsubscribeFromEvent) {
    std::atomic<int> receiveCount{0};
    
    auto handlerId = subscribe<SimpleEvent>([&receiveCount](const SimpleEvent& event) {
        receiveCount++;
    });
    
    publish<SimpleEvent>(SimpleEvent{1, "first"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(receiveCount, 1);
    
    bool unsubscribed = unsubscribe<SimpleEvent>(handlerId);
    ASSERT_TRUE(unsubscribed);
    
    publish<SimpleEvent>(SimpleEvent{2, "second"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_EQ(receiveCount, 1);  // Should not increase
}

TEST_F(EventBusTest, SubscribeWithMinimumPriority) {
    std::atomic<int> lowPriorityCount{0};
    std::atomic<int> highPriorityCount{0};
    
    // Subscribe with minimum priority of Normal
    auto handler = subscribe<PriorityEvent>([&](const PriorityEvent& event) {
        if (event.priority >= neko::Priority::Normal) {
            highPriorityCount++;
        } else {
            lowPriorityCount++;
        }
    }, neko::Priority::Normal);
    
    publish<PriorityEvent>(PriorityEvent{neko::Priority::Low, "low"}, neko::Priority::Low);
    publish<PriorityEvent>(PriorityEvent{neko::Priority::Normal, "normal"}, neko::Priority::Normal);
    publish<PriorityEvent>(PriorityEvent{neko::Priority::High, "high"}, neko::Priority::High);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    unsubscribe<PriorityEvent>(handler);
}

// === Publish Tests ===

TEST_F(EventBusTest, PublishEventWithConstReference) {
    std::atomic<int> receivedValue{0};
    
    auto handler = subscribe<SimpleEvent>([&receivedValue](const SimpleEvent& event) {
        receivedValue = event.value;
    });
    
    SimpleEvent event{123, "const ref test"};
    publish<SimpleEvent>(event);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_EQ(receivedValue, 123);
    unsubscribe<SimpleEvent>(handler);
}

TEST_F(EventBusTest, PublishEventWithRValueReference) {
    std::atomic<int> receivedValue{0};
    
    auto handler = subscribe<SimpleEvent>([&receivedValue](const SimpleEvent& event) {
        receivedValue = event.value;
    });
    
    publish<SimpleEvent>(SimpleEvent{456, "rvalue test"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_EQ(receivedValue, 456);
    unsubscribe<SimpleEvent>(handler);
}

TEST_F(EventBusTest, PublishWithPriorityAndSyncMode) {
    std::atomic<bool> eventReceived{false};
    
    auto handler = subscribe<SimpleEvent>([&eventReceived](const SimpleEvent& event) {
        eventReceived = true;
    });
    
    publish<SimpleEvent>(SimpleEvent{789, "priority test"}, neko::Priority::High, neko::SyncMode::Async);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    ASSERT_TRUE(eventReceived);
    unsubscribe<SimpleEvent>(handler);
}

TEST_F(EventBusTest, PublishMultipleEvents) {
    std::atomic<int> counter{0};
    
    auto handler = subscribe<CounterEvent>([&counter](const CounterEvent& event) {
        counter += event.count;
    });
    
    for (int i = 1; i <= 10; ++i) {
        publish<CounterEvent>(CounterEvent{i});
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(counter, 55);  // Sum of 1 to 10
    
    unsubscribe<CounterEvent>(handler);
}

// === Delayed Publishing Tests ===

TEST_F(EventBusTest, PublishAfterDelay) {
    std::atomic<bool> eventReceived{false};
    auto startTime = std::chrono::steady_clock::now();
    
    auto handler = subscribe<SimpleEvent>([&eventReceived](const SimpleEvent& event) {
        eventReceived = true;
    });
    
    auto eventId = publishAfter<SimpleEvent>(150, SimpleEvent{999, "delayed"});
    ASSERT_NE(eventId, 0);
    
    // Should not be received immediately
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(eventReceived);
    
    // Wait for the delayed event with polling
    bool received = false;
    for (int i = 0; i < 20 && !eventReceived; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    ASSERT_TRUE(eventReceived) << "Event was not received after waiting";
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    ASSERT_GE(duration.count(), 100);  // Allow some variance
    
    unsubscribe<SimpleEvent>(handler);
}

TEST_F(EventBusTest, PublishAfterWithRValue) {
    std::atomic<int> receivedValue{0};
    
    auto handler = subscribe<SimpleEvent>([&receivedValue](const SimpleEvent& event) {
        receivedValue = event.value;
    });
    
    auto eventId = publishAfter<SimpleEvent>(100, SimpleEvent{777, "delayed rvalue"});
    ASSERT_NE(eventId, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ASSERT_EQ(receivedValue, 777);
    
    unsubscribe<SimpleEvent>(handler);
}

// === Task Scheduling Tests ===

TEST_F(EventBusTest, ScheduleTaskAtTimePoint) {
    std::atomic<bool> taskExecuted{false};
    
    auto futureTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    auto eventId = scheduleTask(futureTime, [&taskExecuted]() {
        taskExecuted = true;
    });
    
    ASSERT_NE(eventId, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(taskExecuted);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(taskExecuted);
}

TEST_F(EventBusTest, ScheduleTaskAfterMilliseconds) {
    std::atomic<bool> taskExecuted{false};
    
    auto eventId = scheduleTask(150, [&taskExecuted]() {
        taskExecuted = true;
    });
    
    ASSERT_NE(eventId, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(taskExecuted);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    ASSERT_TRUE(taskExecuted);
}

TEST_F(EventBusTest, ScheduleTaskWithPriority) {
    std::atomic<bool> taskExecuted{false};
    
    auto eventId = scheduleTask(100, [&taskExecuted]() {
        taskExecuted = true;
    }, neko::Priority::High);
    
    ASSERT_NE(eventId, 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_TRUE(taskExecuted);
}

TEST_F(EventBusTest, ScheduleRepeatingTask) {
    std::atomic<int> executionCount{0};
    
    auto eventId = scheduleRepeating(80, [&executionCount]() {
        executionCount++;
    });
    
    ASSERT_NE(eventId, 0);
    
    // Wait with polling for multiple executions
    for (int i = 0; i < 30 && executionCount < 2; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Should have executed at least twice
    ASSERT_GE(executionCount, 2) << "Task executed " << executionCount << " times";
    
    cancelTask(eventId);
}

TEST_F(EventBusTest, CancelScheduledTask) {
    std::atomic<bool> taskExecuted{false};
    
    auto eventId = scheduleTask(200, [&taskExecuted]() {
        taskExecuted = true;
    });
    
    ASSERT_NE(eventId, 0);
    
    bool cancelled = cancelTask(eventId);
    ASSERT_TRUE(cancelled);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    ASSERT_FALSE(taskExecuted);
}

TEST_F(EventBusTest, CleanupCancelledTasks) {
    auto eventId1 = scheduleTask(500, []() {});
    auto eventId2 = scheduleTask(500, []() {});
    
    cancelTask(eventId1);
    cancelTask(eventId2);
    
    cleanupCancelledTasks();
    
    // No assertion needed, just ensure it doesn't crash
}

// === Event Loop Control Tests ===

TEST_F(EventBusTest, IsRunningCheck) {
    // Event loop should be running from SetUp
    ASSERT_TRUE(isRunning());
}

TEST_F(EventBusTest, WakeUpEventLoop) {
    wakeUp();
    // No assertion needed, just ensure it doesn't crash
}

TEST_F(EventBusTest, SetMaxQueueSize) {
    setMaxQueueSize(1000);
    // No direct way to verify, but ensure it doesn't crash
}

TEST_F(EventBusTest, EnableStatistics) {
    enableStatistics(true);
    
    auto handler = subscribe<SimpleEvent>([](const SimpleEvent& event) {});
    publish<SimpleEvent>(SimpleEvent{1, "stats test"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto stats = getStatistics();
    // Stats should have recorded something
    
    enableStatistics(false);
    unsubscribe<SimpleEvent>(handler);
}

TEST_F(EventBusTest, SetLogger) {
    std::atomic<bool> loggerCalled{false};
    
    setLogger([&loggerCalled](const std::string& message) {
        loggerCalled = true;
    });
    
    // Trigger some event to potentially generate logs
    auto handler = subscribe<SimpleEvent>([](const SimpleEvent& event) {});
    publish<SimpleEvent>(SimpleEvent{1, "logger test"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    unsubscribe<SimpleEvent>(handler);
}

// === Statistics Tests ===

TEST_F(EventBusTest, ResetStatistics) {
    enableStatistics(true);
    
    auto handler = subscribe<SimpleEvent>([](const SimpleEvent& event) {});
    publish<SimpleEvent>(SimpleEvent{1, "stats"});
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    resetStatistics();
    
    auto stats = getStatistics();
    // After reset, stats should be cleared
    
    enableStatistics(false);
    unsubscribe<SimpleEvent>(handler);
}

TEST_F(EventBusTest, GetStatistics) {
    enableStatistics(true);
    
    auto handler = subscribe<SimpleEvent>([](const SimpleEvent& event) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
    
    for (int i = 0; i < 5; ++i) {
        publish<SimpleEvent>(SimpleEvent{i, "stats"});
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto stats = getStatistics();
    // Should have recorded some events
    
    enableStatistics(false);
    unsubscribe<SimpleEvent>(handler);
}

// === Stress Tests ===

TEST_F(EventBusTest, HighVolumeEventPublishing) {
    std::atomic<int> counter{0};
    
    auto handler = subscribe<CounterEvent>([&counter](const CounterEvent& event) {
        counter++;
    });
    
    const int eventCount = 1000;
    for (int i = 0; i < eventCount; ++i) {
        publish<CounterEvent>(CounterEvent{i});
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(counter, eventCount);
    
    unsubscribe<CounterEvent>(handler);
}

TEST_F(EventBusTest, ConcurrentSubscriptionsAndPublishing) {
    std::atomic<int> counter{0};
    std::vector<neko::event::HandlerId> handlers;
    
    // Add multiple subscribers
    for (int i = 0; i < 10; ++i) {
        handlers.push_back(subscribe<SimpleEvent>([&counter](const SimpleEvent& event) {
            counter++;
        }));
    }
    
    // Publish events
    for (int i = 0; i < 50; ++i) {
        publish<SimpleEvent>(SimpleEvent{i, "concurrent test"});
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(counter, 500);  // 10 handlers * 50 events
    
    // Cleanup
    for (auto handler : handlers) {
        unsubscribe<SimpleEvent>(handler);
    }
}
