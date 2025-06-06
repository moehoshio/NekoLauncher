#pragma once
#include <filesystem>
#include <optional>
#include <string>

/**
 * @namespace neko::system
 * @brief System-related information and utilities.
 */
namespace neko::system {

    /**
     * @struct PlatformInfo
     * @brief Provides static platform identification information.
     *
     * Contains compile-time constants for operating system name and architecture.
     */
    struct PlatformInfo {

        /**
         * @brief Operating system name identifier.
         * @return String representing the OS ("windows", "osx", "linux", or "unknown")
         */
        constexpr static const char *osName =
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
         * @return String representing the architecture ("x64", "x86", "arm64", "arm", or "unknown")
         */
        constexpr static const char *osArch =
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
     * @brief Gets or sets the temporary directory path.
     * @param setTempDir Optional parameter to set a new temporary directory.
     * @return The current temporary directory path.
     *
     * @details Uses exec::unifiedPaths from exec.hpp for path normalization.
     * Creates the directory if it doesn't exist.
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
     * @brief Gets or sets the current working directory.
     * @param setPath Optional parameter to set a new working directory.
     * @return The current working directory path.
     *
     * @details Uses exec::unifiedPaths from exec.hpp for path normalization.
     */
    inline std::string workPath(const std::string &setPath = "") {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        if (!setPath.empty() && std::filesystem::is_directory(setPath))
            std::filesystem::current_path(setPath);

        return std::filesystem::current_path().string() | exec::unifiedPaths;
    }

    /**
     * @brief Gets the user's home directory.
     * @return The path to the user's home directory.
     *
     * @details Uses exec::unifiedThePaths from exec.hpp for path normalization.
     * Platform-specific implementation using environment variables.
     */
    inline std::optional<std::string> getHome() {
        const char *path = std::getenv(
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
     * @brief Gets the operating system name.
     * @return String identifier for the operating system.
     */
    constexpr inline const char *getOsName() {
        return PlatformInfo::osName;
    }

    /**
     * @brief Gets the system architecture.
     * @return String identifier for the CPU architecture.
     */
    constexpr inline const char *getOsArch() {
        return PlatformInfo::osArch;
    }

} // namespace neko::system