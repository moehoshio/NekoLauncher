cmake_minimum_required(VERSION 3.20)
project(NekoLc_Bus_test LANGUAGES CXX)

enable_testing()
include(GoogleTest)

# Thread Bus
add_executable(NekoLcBus_thread_test "${CMAKE_CURRENT_SOURCE_DIR}/threadBus_test.cpp")
target_link_libraries(NekoLcBus_thread_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcBus_thread_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcBus_thread_test DISCOVERY_TIMEOUT 60)

# Event
add_executable(NekoLcBus_event_test "${CMAKE_CURRENT_SOURCE_DIR}/event_test.cpp")
target_link_libraries(NekoLcBus_event_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcBus_event_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcBus_event_test DISCOVERY_TIMEOUT 60)

# Config Bus
add_executable(NekoLcBus_config_test "${CMAKE_CURRENT_SOURCE_DIR}/configBus_test.cpp")
target_link_libraries(NekoLcBus_config_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcBus_config_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcBus_config_test DISCOVERY_TIMEOUT 60)

# Resources
add_executable(NekoLcBus_resources_test "${CMAKE_CURRENT_SOURCE_DIR}/resources_test.cpp")
target_link_libraries(NekoLcBus_resources_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcBus_resources_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcBus_resources_test DISCOVERY_TIMEOUT 60)