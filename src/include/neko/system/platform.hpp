/**
 * @file platform.hpp
 * @author moehoshio
 * @date 2025/06/07
 * @brief Provides platform-related information and utility functions, including OS/architecture identification, temporary/work/home directory helpers.
 *
 * This file is defined under the neko::system namespace and contains the PlatformInfo structure and several platform utility functions.
 */

#pragma once

#include "neko/schema/types.hpp"

// Unified paths utility
#include "neko/function/exec.hpp"

#include <filesystem>
#include <optional>
#include <string>

/**
 * @namespace neko::system
 * @brief Provides system-related information and utility functions.
 */
namespace neko::system {

    /**
     * @struct PlatformInfo
     * @brief Provides static platform identification information.
     *
     * Contains compile-time constants for operating system name and CPU architecture.
     */
    struct PlatformInfo {

        /**
         * @brief Operating system name identifier.
         * @details Possible values: "windows", "osx", "linux", or "unknown".
         */
        constexpr static neko::cstr osName =
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
        constexpr static neko::cstr osArch =
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
    }; // class PlatformInfo

    /**
     * @brief Get or set the temporary directory path.
     * @param setTempDir [optional] Specify a new temporary directory path. If empty, only returns the current setting.
     * @return std::string The current temporary directory path (normalized).
     *
     * @details
     * - Uses exec::unifiedPaths for path normalization.
     * - Automatically creates the directory if it does not exist.
     * - Prefers ClientConfig setting if available, otherwise uses the system default temp directory.
     */
    inline std::string temporaryFolder(const std::string &setTempDir = "") {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);

        auto init = []() -> std::string {
            ClientConfig cfg(exec::getConfigObj());
            if (std::filesystem::is_directory(cfg.more.tempDir))
                return std::string(cfg.more.tempDir) | exec::unifiedPaths;
            else
                return (std::filesystem::temp_directory_path().string() + "/Nekolc") | exec::unifiedPaths;
        };

        static std::string tempDir = init();

        if (!setTempDir.empty() && std::filesystem::is_directory(setTempDir)) {
            tempDir = setTempDir | exec::unifiedPaths;
        }

        if (!std::filesystem::exists(tempDir))
            std::filesystem::create_directory(tempDir);

        return tempDir;
    }

    /**
     * @brief Get or set the current working directory.
     * @param setPath [optional] Specify a new working directory path. If empty, only returns the current directory.
     * @return std::string The current working directory path (normalized).
     *
     * @details
     * - Uses exec::unifiedPaths for path normalization.
     * - Changes working directory if setPath is valid.
     */
    inline std::string workPath(const std::string &setPath = "") {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        if (!setPath.empty() && std::filesystem::is_directory(setPath))
            std::filesystem::current_path(setPath);

        return std::filesystem::current_path().string() | exec::unifiedPaths;
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
    constexpr inline neko::cstr getOsName() {
        return PlatformInfo::osName;
    }

    /**
     * @brief Get the system CPU architecture identifier.
     * @return const char* Architecture name.
     */
    constexpr inline neko::cstr getOsArch() {
        return PlatformInfo::osArch;
    }

} // namespace neko::system