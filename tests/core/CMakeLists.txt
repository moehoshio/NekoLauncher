cmake_minimum_required(VERSION 3.20)
project(NekoLc_Core_test LANGUAGES CXX)

enable_testing()
include(GoogleTest)

# launcherProcess test
add_executable(NekoLcCore_launcherProcess_test ${CMAKE_CURRENT_SOURCE_DIR}/launcherProcess_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../src/neko/core/launcherProcess.cpp)
target_link_libraries(NekoLcCore_launcherProcess_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main Boost::process)
target_compile_features(NekoLcCore_launcherProcess_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcCore_launcherProcess_test DISCOVERY_TIMEOUT 60)

# update test
add_executable(NekoLcCore_update_test ${CMAKE_CURRENT_SOURCE_DIR}/update_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../src/neko/core/launcherProcess.cpp)
target_link_libraries(NekoLcCore_update_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main Boost::process)
target_compile_features(NekoLcCore_update_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcCore_update_test DISCOVERY_TIMEOUT 60)

# maintenance test
add_executable(NekoLcCore_maintenance_test ${CMAKE_CURRENT_SOURCE_DIR}/maintenance_test.cpp)
target_link_libraries(NekoLcCore_maintenance_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcCore_maintenance_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcCore_maintenance_test DISCOVERY_TIMEOUT 60)

# downloadPoster test
add_executable(NekoLcCore_downloadPoster_test ${CMAKE_CURRENT_SOURCE_DIR}/downloadPoster_test.cpp)
target_link_libraries(NekoLcCore_downloadPoster_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcCore_downloadPoster_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcCore_downloadPoster_test DISCOVERY_TIMEOUT 60)