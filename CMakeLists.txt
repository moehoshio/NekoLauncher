cmake_minimum_required(VERSION 3.20)

# CMake Policy
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(
    NekoLc LANGUAGES CXX VERSION 0.0.1
    DESCRIPTION "Neko Launcher Client"
    HOMEPAGE_URL "https://github.com/moehoshio/NekoLauncher"
)

# ================
# === Config ====
# ================

option(NEKO_LC_STATIC_LINK "Neko Launcher Static Link Libraries" OFF)
option(NEKO_LC_BUILD_TESTS "Neko Launcher Build tests" ON)

# Package path
if(DEFINED NEKO_LC_LIBRARY_DIRS)
    list(APPEND CMAKE_PREFIX_PATH ${NEKO_LC_LIBRARY_DIRS})
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
endif()

if(MSVC)
    if(NEKO_LC_STATIC_LINK)
        set(NEKO_FUNCTION_STATIC_LINK ON CACHE BOOL "NekoFunction static link" FORCE)
        set(NEKO_SYSTEM_STATIC_LINK ON CACHE BOOL "NekoSystem static link" FORCE)
        set(NEKO_NETWORK_STATIC_LINK ON CACHE BOOL "NekoNetwork static link" FORCE)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        # Align with prebuilt Qt (uses /MD or /MDd)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

# Get the current git commit ID and build time
string(TIMESTAMP NEKO_LC_BUILD_TIME "%Y%m%d%H%M%S" UTC)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE NEKO_LC_GIT_COMMIT_ID OUTPUT_STRIP_TRAILING_WHITESPACE)

# ================
# === Library ====
# ================

# Neko Modules
find_package(NekoSchema QUIET)
find_package(NekoEvent QUIET)
find_package(NekoThreadPool QUIET)
find_package(NekoLog QUIET)
find_package(NekoFunction QUIET)
find_package(NekoSystem QUIET)
find_package(NekoNetwork QUIET)

# External Libraries
find_package(Boost COMPONENTS process)
find_package(Qt6 COMPONENTS Core Gui Widgets)
find_package(GTest QUIET)
find_package(nlohmann_json QUIET)
find_package(SimpleIni QUIET)

if (NOT Boost_FOUND)
    message(FATAL_ERROR "Required Boost but not found ; please make sure to set -DNEKO_LC_LIBRARY_DIRS=<path> correctly.")
endif()

if (NOT Qt6_FOUND)
    message(FATAL_ERROR "Required Qt6 but not found ; please make sure to set -DNEKO_LC_LIBRARY_DIRS=<Qt_install_path> correctly.")
endif()

if(NEKO_LC_STATIC_LINK AND DEFINED QT_BUILD_SHARED_LIBS AND QT_BUILD_SHARED_LIBS)
    message(FATAL_ERROR "NEKO_LC_STATIC_LINK=ON requires a static Qt build, but the found Qt is shared (QT_BUILD_SHARED_LIBS=ON). Either provide a static Qt or turn NEKO_LC_STATIC_LINK=OFF to use /MD(/MDd).")
endif()

# Automatically fetch missing dependencies if not found
include(FetchContent)

if(NOT GTest_FOUND AND NEKO_LC_BUILD_TESTS)
    message(STATUS "GTest Not Found; Neko Launcher is fetching GoogleTest...")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

if (NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json Not Found; Neko Launcher is fetching nlohmann_json...")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

if (NOT SimpleIni_FOUND)
    message(STATUS "SimpleIni Not Found; Neko Launcher is fetching SimpleIni...")
    FetchContent_Declare(
        simpleini
        GIT_REPOSITORY https://github.com/brofield/simpleini.git
        GIT_TAG        v4.22
    )
    FetchContent_MakeAvailable(simpleini)
endif()

# Neko Modules

if (NOT NekoSchema_FOUND)
    message(STATUS "NekoSchema Not Found; Neko Launcher is fetching NekoSchema...")
    FetchContent_Declare(
        NekoSchema
        GIT_REPOSITORY https://github.com/moehoshio/NekoSchema.git
        GIT_TAG        main
    )
    set(NEKO_SCHEMA_BUILD_TESTS OFF CACHE BOOL "Build NekoSchema tests" FORCE)
    FetchContent_MakeAvailable(NekoSchema)
endif()

if (NOT NekoEvent_FOUND)
    message(STATUS "NekoEvent Not Found; Neko Launcher is fetching NekoEvent...")
    FetchContent_Declare(
        NekoEvent
        GIT_REPOSITORY https://github.com/moehoshio/NekoEvent.git
        GIT_TAG        main
    )
    set(NEKO_EVENT_BUILD_TESTS OFF CACHE BOOL "Build NekoEvent tests" FORCE)
    FetchContent_MakeAvailable(NekoEvent)
endif()

if (NOT NekoThreadPool_FOUND)
    message(STATUS "NekoThreadPool Not Found; Neko Launcher is fetching NekoThreadPool...")
    FetchContent_Declare(
        NekoThreadPool
        GIT_REPOSITORY https://github.com/moehoshio/NekoThreadPool.git
        GIT_TAG        main
    )
    set(NEKO_THREAD_POOL_BUILD_TESTS OFF CACHE BOOL "Build NekoThreadPool tests" FORCE)
    FetchContent_MakeAvailable(NekoThreadPool)
endif()

if (NOT NekoLog_FOUND)
    message(STATUS "NekoLog Not Found; Neko Launcher is fetching NekoLog...")
    FetchContent_Declare(
        NekoLog
        GIT_REPOSITORY https://github.com/moehoshio/NekoLog.git
        GIT_TAG        main
    )
    set(NEKO_LOG_BUILD_TESTS OFF CACHE BOOL "Build NekoLog tests" FORCE)
    FetchContent_MakeAvailable(NekoLog)
endif()

if (NOT NekoFunction_FOUND)
    message(STATUS "NekoFunction Not Found; Neko Launcher is fetching NekoFunction...")
    FetchContent_Declare(
        NekoFunction
        GIT_REPOSITORY https://github.com/moehoshio/NekoFunction.git
        GIT_TAG        main
    )
    set(NEKO_FUNCTION_BUILD_TESTS OFF CACHE BOOL "Build NekoFunction tests" FORCE)
    FetchContent_MakeAvailable(NekoFunction)
endif()

if (NOT NekoSystem_FOUND)
    message(STATUS "NekoSystem Not Found; Neko Launcher is fetching NekoSystem...")
    FetchContent_Declare(
        NekoSystem
        GIT_REPOSITORY https://github.com/moehoshio/NekoSystem.git
        GIT_TAG        main
    )
    set(NEKO_SYSTEM_BUILD_TESTS OFF CACHE BOOL "Build NekoSystem tests" FORCE)
    FetchContent_MakeAvailable(NekoSystem)
endif()

if (NOT NekoNetwork_FOUND)
    message(STATUS "NekoNetwork Not Found; Neko Launcher is fetching NekoNetwork...")
    FetchContent_Declare(
        NekoNetwork
        GIT_REPOSITORY https://github.com/moehoshio/NekoNetwork.git
        GIT_TAG        main
    )
    set(NEKO_NETWORK_BUILD_TESTS OFF CACHE BOOL "Build NekoNetwork tests" FORCE)
    FetchContent_MakeAvailable(NekoNetwork)
endif()

# ================
# ====  Info  ====
# ================

# Print configuration summary
message(STATUS "===============================================")
message(STATUS "NekoLauncher Configuration Summary")
message(STATUS "===============================================")
message(STATUS "")
message(STATUS "Project Information:")
message(STATUS "  - Project version: ${PROJECT_VERSION}")
message(STATUS "  - Git commit ID: ${NEKO_LC_GIT_COMMIT_ID}")
message(STATUS "  - Build time (UTC): ${NEKO_LC_BUILD_TIME}")
message(STATUS "")
message(STATUS "Build Configuration:")
message(STATUS "  - Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - Static linking: ${NEKO_LC_STATIC_LINK}")
message(STATUS "  - Build tests: ${NEKO_LC_BUILD_TESTS}")
message(STATUS "  - C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
message(STATUS "CMake Configuration:")
message(STATUS "  - CMake version: ${CMAKE_VERSION}")
message(STATUS "  - CMake Prefix Path: ${CMAKE_PREFIX_PATH}")
message(STATUS "  - Library directories: ${NEKO_LC_LIBRARY_DIRS}")
message(STATUS "")
message(STATUS "Dependency summary:")
message(STATUS "  - Qt6 : ${Qt6_FOUND} version : ${Qt6_VERSION}")
message(STATUS "  - Boost : ${Boost_FOUND} version : ${Boost_VERSION}")
message(STATUS "")
message(STATUS "  - NekoSchema : ${NekoSchema_FOUND} version : ${NekoSchema_VERSION}")
message(STATUS "  - NekoEvent : ${NekoEvent_FOUND} version : ${NekoEvent_VERSION}")
message(STATUS "  - NekoThreadPool : ${NekoThreadPool_FOUND} version : ${NekoThreadPool_VERSION}")
message(STATUS "  - NekoLog : ${NekoLog_FOUND} version : ${NekoLog_VERSION}")
message(STATUS "  - NekoFunction : ${NekoFunction_FOUND} version : ${NekoFunction_VERSION}")
message(STATUS "  - NekoSystem : ${NekoSystem_FOUND} version : ${NekoSystem_VERSION}")
message(STATUS "  - NekoNetwork : ${NekoNetwork_FOUND} version : ${NekoNetwork_VERSION}")
message(STATUS "")
message(STATUS "  - nlohmann_json : ${nlohmann_json_FOUND} version : ${nlohmann_json_VERSION}")
message(STATUS "  - SimpleIni : ${SimpleIni_FOUND} version : ${SimpleIni_VERSION}")
message(STATUS "  - GTest : ${GTest_FOUND} version : ${GTest_VERSION}")
message(STATUS "")
message(STATUS "===============================================")
message(STATUS "")

add_definitions(
    -DNEKO_LC_NAME="${PROJECT_NAME}"
    -DNEKO_LC_VERSION="${PROJECT_VERSION}"
    -DNEKO_LC_BUILD_TIME="${NEKO_LC_BUILD_TIME}"
    -DNEKO_LC_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    -DNEKO_LC_GIT_COMMIT_ID="${NEKO_LC_GIT_COMMIT_ID}"
)

if (WIN32)
    add_definitions(-DNOMINMAX)
endif()

# ================
# === Add Src ====
# ================

# Include directories
list(APPEND NEKO_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NEKO_LC_LIBRARY_DIRS}/include
    ${simpleini_SOURCE_DIR}
)

# Src files
list(APPEND NEKO_SRCFILES
    # Core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/core/launcherProcess.cpp
    # UI
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/dialogs/noticeDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/dialogs/inputDialog.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/pages/homePage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/pages/loadingPage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/pages/settingPage.cpp
    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/windows/nekoWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/windows/logViewerWindow.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/ui/widgets/pixmapWidget.cpp
    # Headers (for Qt MOC)
    ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/ui/dialogs/noticeDialog.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/ui/dialogs/inputDialog.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/ui/pages/homePage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/ui/pages/loadingPage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/ui/pages/settingPage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/ui/windows/nekoWindow.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/neko/ui/windows/logViewerWindow.hpp
    
    # Main
    ${CMAKE_CURRENT_SOURCE_DIR}/src/neko/main.cpp
)

# =====================
# = Commons Libraries =
# =====================

add_library(Neko_Commons INTERFACE)
target_include_directories(Neko_Commons INTERFACE ${NEKO_INCLUDE_DIRS} )
target_link_libraries(Neko_Commons INTERFACE Neko::Schema Neko::Event Neko::ThreadPool Neko::Log Neko::Function Neko::System Neko::Network)

add_library(Neko_Commons_Other INTERFACE)
target_link_libraries(Neko_Commons_Other INTERFACE nlohmann_json SimpleIni)

# ================
#  Main Executable
# ================

set(CMAKE_AUTOMOC ON)

add_executable(NekoLc ${NEKO_SRCFILES})
target_compile_features(NekoLc PRIVATE cxx_std_20)
target_link_libraries(NekoLc PRIVATE 
    # Neko Modules
    Neko_Commons 
    # Others
    Neko_Commons_Other
    # Qt
    Qt6::Core Qt6::Widgets Qt6::Gui
    # Boost
    Boost::process
)

# Tools
add_subdirectory(tools/update)

# Os-specific settings
if(WIN32)
    
    # Set not to use the console window
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(NekoLc PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
    target_sources(NekoLc PRIVATE resource/windows/icon.rc)

elseif(APPLE)

    # Set the application bundle properties for macOS
    # if(CMAKE_BUILD_TYPE STREQUAL "Release")
    #     set_target_properties(NekoLc PROPERTIES
    #         MACOSX_BUNDLE TRUE
    #         MACOSX_BUNDLE_BUNDLE_NAME "com.moehoshio.NekoLauncher"
    #         MACOSX_BUNDLE_ICON_FILE "NekoLc.icns"
    #         MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resource/macos/Info.plist
    #     )
    # endif()
    # set(MACOSX_ICON ${CMAKE_SOURCE_DIR}/resource/macos/NekoLc.icns)
    # target_sources(NekoLc PRIVATE ${MACOSX_ICON})
    # set_source_files_properties(${MACOSX_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

elseif(UNIX)

    # Set the executable properties for Linux
    # set_target_properties(NekoLc PROPERTIES
    #     OUTPUT_NAME "NekoLc"
    #     PREFIX ""
    #     SUFFIX ""
    # )
    # # Add a custom command to copy the icon file to the resource directory
    # add_custom_command(TARGET NekoLc POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resource/linux/icon.png ${CMAKE_BINARY_DIR}/icon.png
    #     COMMENT "Copying icon.png to build directory"
    # )

endif() # os

# ================
# ====  Test  ====
# ================

if(NEKO_LC_BUILD_TESTS)
    message(STATUS "Neko Launcher tests enabled (NEKO_LC_BUILD_TESTS=ON)")
    enable_testing()
    set(CMAKE_AUTOMOC OFF)
    add_subdirectory(tests)
else()
    message(STATUS "Neko Launcher tests disabled (NEKO_LC_BUILD_TESTS=OFF)")
endif()

message(STATUS "Neko Launcher End of ")