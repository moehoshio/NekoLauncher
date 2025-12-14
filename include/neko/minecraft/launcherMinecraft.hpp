/**
 * @file launcherMinecraft.hpp
 * @brief Minecraft launcher management
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include "neko/app/clientConfig.hpp"

#include <optional>
#include <string>
#include <functional>

namespace neko::minecraft {

    /**
     * @struct LauncherMinecraftConfig
     * @brief Configuration structure for Minecraft launcher settings.
     */
    struct LauncherMinecraftConfig {
        /**
         * @var minecraftFolder
         * @brief Minecraft directory path.
         * @example The directory path to the Minecraft installation (e.g., /path/to/.minecraft).
         * @note If a relative path is provided, it will be automatically converted to an absolute path.
         */
        std::string minecraftFolder;
        /**
         * @var minecraftVersionName
         * @brief Minecraft version name.
         * @example The version of Minecraft to launch (e.g., 1.16.5).
         * @note If not specified, the launcher will attempt to launch the first found version (note: this is not guaranteed and the order may be inconsistent each time, unless there is only one version).
         */
        std::string targetVersion;

        /**
         * @var javaPath
         * @brief Path to the Java executable.
         * @example The path to the Java executable used to run Minecraft (e.g., /path/to/java).
         */
        std::string javaPath;

        /**
         * @var playerName
         * @brief Player's name.
         * @example The name of the player in Minecraft (e.g., Steve).
         */
        std::string playerName;

        /**
         * @var uuid
         * @brief Player's unique identifier.
         * @example The UUID of the player in Minecraft (e.g., 123e4567-e89b-12d3-a456-426614174000).
         */
        std::string uuid;

        /**
         * @var accessToken
         * @brief Player's access token.
         * @example The access token used to authenticate the player in Minecraft (e.g., abcdefghijklmnopqrstuvwxyz1234567890).
         */
        std::string accessToken;

        /**
         * @var joinServerAddress
         * @brief Address of the server to join.
         * @example The address of the server to connect to (e.g., play.example.com).
         * @note This is optional and can be left empty if the player is not joining a
         */
        std::string joinServerAddress = "";
        /**
         * @var joinServerPort
         * @brief Port of the server to join.
         * @example The port number of the server to connect to (e.g., 25565).
         */
        std::string joinServerPort = "25565";

        /**
         * @var tolerantMode
         * @brief Determines whether the launcher operates in tolerant mode (true) or strict mode (false).
         *        In tolerant mode, certain parsing or runtime errors will be tolerated and not cause the process to abort.
         *        In strict mode, such errors will cause the process to terminate or throw exceptions.
         * @default false (strict mode)
         * @note This does not affect fatal errors, such as missing core configuration.
         */
        bool tolerantMode = false;

        /**
         * @var maxMemoryLimit
         * @brief Maximum memory limit for the JVM in gigabytes.
         * @default 8 (8 GB)
         * @note This is the maximum amount of memory that the JVM can use when running Minecraft.
         */
        int maxMemoryLimit = 8;
        /**
         * @var minMemoryLimit
         * @brief Minimum memory limit for the JVM in gigabytes.
         * @default 2 (2 GB)
         * @note This is the minimum amount of memory that the JVM will use when running Minecraft.
         */
        int minMemoryLimit = 2;

        /**
         * @var needMemoryLimit
         * @brief Memory limit needed for Minecraft in gigabytes.
         * @default 7 (7 GB)
         * @note This is the amount of memory that Minecraft needs to run.
         * @note For vanilla Minecraft, it is recommended to set 2 GB of memory.
         */
        int needMemoryLimit = 7;

        /**
         * @var isDemoUser
         * @brief Indicates whether the user is running in demo mode.
         */
        bool isDemoUser = false;
        /**
         * @var hasCustomResolution
         * @brief Indicates whether a custom screen resolution is set for the game.
         */
        bool hasCustomResolution = false;

        // only used when hasCustomResolution is true
        std::string resolutionWidth = "1280"; // Custom resolution width, if any
        std::string resolutionHeight = "720"; // Custom resolution height, if any

        struct authlib {

            bool enabled = true; // whether to use authlib injector

            /**
             * @var prefetched
             */
            std::string prefetched;

            /**
             * @var name
             * @brief Name of the Authlib Injector JAR file.
             * @example The name of the Authlib Injector JAR file (e.g., authlib-injector.jar).
             */
            std::string name = "authlib-injector.jar";

            /**
             * @var sha256
             * @brief SHA-256 hash of the Authlib Injector JAR file.
             * @example The SHA-256 hash of the Authlib Injector JAR file (e.g., 1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef).
             */
            std::string sha256;

        } authlib;
    };

    // may throw neko::ex FileError, Parse, OutOfRange , NetworkError
    std::string getLauncherMinecraftCommand(const LauncherMinecraftConfig &cfg);

    /**
     * @brief Launches Minecraft with the specified configuration.
     * @throws ex::FileError if the minecraft folder is not a directory or does not exist
     * @throws ex::Parse if the minecraft version json is invalid or does not contain required fields
     * @throws ex::OutOfRange if the minecraft version json does not contain required keys
     * @throws ex::NetworkError if the download fails or the file hash does not match
     */
    void launcherMinecraft(neko::ClientConfig cfg, std::function<void()> onStart = nullptr, std::function<void(int)> onExit = nullptr, bool detach = false);
} // namespace neko::minecraft
