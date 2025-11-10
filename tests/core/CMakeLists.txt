cmake_minimum_required(VERSION 3.20)
project(NekoLc_Core_test LANGUAGES CXX)

enable_testing()
include(GoogleTest)

if(NEKO_LC_STATIC_LINK AND MSVC)
    # Set MSVC runtime library to static for all targets (must be before any target creation)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_executable(NekoLcCore_launcherProcess_test ${CMAKE_CURRENT_SOURCE_DIR}/launcherProcess_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../src/neko/core/launcherProcess.cpp)
target_link_libraries(NekoLcCore_launcherProcess_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main Boost::filesystem)
target_compile_features(NekoLcCore_launcherProcess_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcCore_launcherProcess_test DISCOVERY_TIMEOUT 60)