/**
 * @file platform.hpp
 * @author moehoshio
 * @date 2025/06/07
 * @brief Provides platform-related information and utility functions, including OS/architecture identification, temporary/work/home directory helpers.
 *
 * This file is defined under the neko::system namespace and contains the PlatformInfo and several platform utility functions.
 */

#pragma once

#include "neko/schema/types.hpp"

// Unified paths utility
#include "neko/function/utilities.hpp"

#include <filesystem>
#include <optional>
#include <string>

/**
 * @namespace neko::system
 * @brief Provides system-related information and utility functions.
 */
namespace neko::system {

    /**
     * @brief Operating system name identifier.
     * @details Possible values: "windows", "osx", "linux", or "unknown".
     */
    constexpr neko::cstr osName =
#if defined(_WIN32)
        "windows";
#elif defined(__APPLE__)
        "osx";
#elif defined(__linux__)
        "linux";
#else
        "unknown";
#endif

    /**
     * @brief CPU architecture identifier.
     * @details Possible values: "x64", "x86", "arm64", "arm", or "unknown".
     */
    constexpr neko::cstr osArch =
#if defined(__x86_64__) || defined(_M_X64)
        "x64";
#elif defined(__i386__) || defined(_M_IX86)
        "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
        "arm64";
#elif defined(__arm__) || defined(_M_ARM)
        "arm";
#else
            "unknown";
#endif

    constexpr bool isMacOS() const {
        return osName == neko::strview("osx");
    }

    constexpr bool isWindows() const {
        return osName == neko::strview("windows");
    }

    constexpr bool isLinux() const {
        return osName == neko::strview("linux");
    }

    constexpr bool isArchX64() {
        return osArch == neko::strview("x64");
    }

    constexpr bool isArchX86() {
        return osArch == neko::strview("x86");
    }

    constexpr bool isArchArm64() {
        return osArch == neko::strview("arm64");
    }

    constexpr bool isArchArm() {
        return osArch == neko::strview("arm");
    }

    /**
     * @brief Get or set the temporary directory path.
     * @param setTempDir [optional] Specify a new temporary directory path. If empty, only returns the current setting.
     * @return std::string The current temporary directory path (normalized).
     *
     * @details
     * - Uses exec::unifiedPath for path normalization.
     * - Automatically creates the directory if it does not exist.
     * - Prefers ClientConfig setting if available, otherwise uses the system default temp directory.
     */
    inline std::string temporaryFolder(const std::string &setTempDir = "") {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);

        using namespace neko::ops::pipe;

        auto init = []() -> std::string {
            ClientConfig cfg(exec::getConfigObj());
            if (std::filesystem::is_directory(cfg.other.tempFolder))
                return std::string(cfg.other.tempFolder) | util::unifiedPath;
            else
                return (std::filesystem::temp_directory_path().string() + "/Nekolc") | util::unifiedPath;
        };

        static std::string tempFolder = init();

        if (!setTempDir.empty() && std::filesystem::is_directory(setTempDir)) {
            tempFolder = setTempDir | util::unifiedPath;
        }

        if (!std::filesystem::exists(tempFolder))
            std::filesystem::create_directory(tempFolder);

        return tempFolder;
    }

    /**
     * @brief Get or set the current working directory.
     * @param setPath [optional] Specify a new working directory path. If empty, only returns the current directory.
     * @return std::string The current working directory path (normalized).
     *
     * @details
     * - Uses exec::unifiedPath for path normalization.
     * - Changes working directory if setPath is valid.
     */
    inline std::string workPath(const std::string &setPath = "") {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        if (!setPath.empty() && std::filesystem::is_directory(setPath))
            std::filesystem::current_path(setPath);

        using namespace neko::ops::pipe;

        return std::filesystem::current_path().string() | util::unifiedPath;
    }

    /**
     * @brief Get the user's home directory path.
     * @return std::optional<std::string> Returns the home directory path (normalized) if successful, otherwise std::nullopt.
     *
     * @details
     * - Uses exec::unifiedThePaths for path normalization.
     * - Automatically selects the environment variable by platform (Windows: USERPROFILE, others: HOME).
     */
    inline std::optional<std::string> getHome() {
        neko::cstr path = std::getenv(
#ifdef _WIN32
            "USERPROFILE"
#else
            "HOME"
#endif
        );
        if (path)
            return exec::unifiedThePaths<std::string>(path);
        return std::nullopt;
    }

    /**
     * @brief Get the operating system name identifier.
     * @return const char* OS name.
     */
    constexpr neko::cstr getOsName() {
        return osName;
    }

    /**
     * @brief Get the system CPU architecture identifier.
     * @return const char* Architecture name.
     */
    constexpr neko::cstr getOsArch() {
        return osArch;
    }

    std::string getOsVersion();

} // namespace neko::system