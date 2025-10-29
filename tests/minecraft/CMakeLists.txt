cmake_minimum_required(VERSION 3.20)
project(NekoLc_Minecraft_test LANGUAGES CXX)

enable_testing()
include(GoogleTest)

# Auth
add_executable(NekoLcMinecraft_auth_test "${CMAKE_CURRENT_SOURCE_DIR}/auth_test.cpp")
target_link_libraries(NekoLcMinecraft_auth_test PRIVATE Neko_Commons Neko_Commons_Other GTest::gtest GTest::gtest_main)
target_compile_features(NekoLcMinecraft_auth_test PRIVATE cxx_std_20)
gtest_discover_tests(NekoLcMinecraft_auth_test)