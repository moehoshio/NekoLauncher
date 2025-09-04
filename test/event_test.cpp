#include "neko/event/event.hpp"

#include <cassert>
#include <string>
#include <vector>
#include <iostream>

void testEventSystem() {
    neko::event::Event<int> intEvent(42);
    neko::event::Event<std::string> stringEvent("Hello, World!");

    // Check event types
    assert(intEvent.getType() == std::type_index(typeid(int)));
    assert(stringEvent.getType() == std::type_index(typeid(std::string)));

    // Check event data
    assert(intEvent.data == 42);
    assert(stringEvent.data == "Hello, World!");
    std::cout << "1. testEventSystem ok\n";
}

void testEventDefaultConstruction() {
    neko::event::Event<double> doubleEvent;
    // Default constructed data should be default value
    assert(doubleEvent.data == double());
    // Type check
    assert(doubleEvent.getType() == std::type_index(typeid(double)));
    std::cout << "2. testEventDefaultConstruction ok\n";
}

void testEventCustomPriorityAndMode() {
    neko::event::Event<int> event;
    event.priority = neko::Priority::High;
    event.mode = neko::SyncMode::Sync;
    assert(event.priority == neko::Priority::High);
    assert(event.mode == neko::SyncMode::Sync);
    std::cout << "3. testEventCustomPriorityAndMode ok\n";
}

void testEventWithComplexType() {
    std::vector<int> vec = {1, 2, 3};
    neko::event::Event<std::vector<int>> vecEvent(vec);
    assert(vecEvent.data == vec);
    assert(vecEvent.getType() == std::type_index(typeid(std::vector<int>)));
    std::cout << "4. testEventWithComplexType ok\n";
}

void testEventTimestamp() {
    neko::event::Event<int> event(123);
    auto now = std::chrono::steady_clock::now();
    // The timestamp should not be far in the future
    assert(event.timestamp <= now);
    std::cout << "5. testEventTimestamp ok\n";
}

int main() {
    testEventSystem();
    testEventDefaultConstruction();
    testEventCustomPriorityAndMode();
    testEventWithComplexType();
    testEventTimestamp();
    std::cout << "All tests passed.\n";
    return 0;
}