cmake_minimum_required(VERSION 3.20)
project(NekoLc_App_test LANGUAGES CXX)

# =====================
# == NekoLc App Test ==
# =====================

enable_testing()
include(GoogleTest)

# Api
add_executable(NekoLcApp_Api_test "${CMAKE_CURRENT_SOURCE_DIR}/api_test.cpp")
target_link_libraries(NekoLcApp_Api_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcApp_Api_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcApp_Api_test DISCOVERY_TIMEOUT 60)

# App
add_executable(NekoLcApp_App_test "${CMAKE_CURRENT_SOURCE_DIR}/app_test.cpp")
target_link_libraries(NekoLcApp_App_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcApp_App_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcApp_App_test DISCOVERY_TIMEOUT 60)

# App Info
add_executable(NekoLcApp_AppInfo_test "${CMAKE_CURRENT_SOURCE_DIR}/appinfo_test.cpp")
target_link_libraries(NekoLcApp_AppInfo_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcApp_AppInfo_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcApp_AppInfo_test DISCOVERY_TIMEOUT 60)

# ClientConfig
add_executable(NekoLcApp_ClientConfig_test "${CMAKE_CURRENT_SOURCE_DIR}/clientConfig_test.cpp")
target_link_libraries(NekoLcApp_ClientConfig_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcApp_ClientConfig_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcApp_ClientConfig_test DISCOVERY_TIMEOUT 60)

# Config Manager
add_executable(NekoLcApp_ConfigManager_test "${CMAKE_CURRENT_SOURCE_DIR}/configManager_test.cpp")
target_link_libraries(NekoLcApp_ConfigManager_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcApp_ConfigManager_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcApp_ConfigManager_test DISCOVERY_TIMEOUT 60)

# Lang
add_executable(NekoLcApp_Lang_test "${CMAKE_CURRENT_SOURCE_DIR}/lang_test.cpp")
target_link_libraries(NekoLcApp_Lang_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcApp_Lang_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcApp_Lang_test DISCOVERY_TIMEOUT 60)