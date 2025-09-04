/**
 * @file launcherMinecraft.hpp
 * @brief Minecraft launcher management
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include "neko/function/archiver.hpp"
#include "neko/schema/clientconfig.hpp"
#include "neko/schema/nekodefine.hpp"
#include "neko/schema/types.hpp"

#include "neko/schema/exception.hpp"

#include "neko/function/utilities.hpp"
#include "neko/function/info.hpp"

#include "neko/core/launcherProcess.hpp"

#include "neko/system/memoryinfo.hpp"
#include "neko/system/platform.hpp"

#include "neko/network/network.hpp"

#include "library/nlohmann/json.hpp"

#include <optional>
#include <regex>

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

            authlib &setEnabled(bool v) {
                enabled = v;
                return *this;
            }
            authlib &setPrefetched(const std::string &v) {
                prefetched = v;
                return *this;
            }
            authlib &setName(const std::string &v) {
                name = v;
                return *this;
            }
            authlib &setSha256(const std::string &v) {
                sha256 = v;
                return *this;
            }
        } authlib;

        // Chainable setters for LauncherMinecraftConfig
        LauncherMinecraftConfig &setMinecraftFolder(const std::string &v) {
            minecraftFolder = v;
            return *this;
        }
        LauncherMinecraftConfig &setTargetVersion(const std::string &v) {
            targetVersion = v;
            return *this;
        }
        LauncherMinecraftConfig &setJavaPath(const std::string &v) {
            javaPath = v;
            return *this;
        }
        LauncherMinecraftConfig &setPlayerName(const std::string &v) {
            playerName = v;
            return *this;
        }
        LauncherMinecraftConfig &setUuid(const std::string &v) {
            uuid = v;
            return *this;
        }
        LauncherMinecraftConfig &setAccessToken(const std::string &v) {
            accessToken = v;
            return *this;
        }
        LauncherMinecraftConfig &setJoinServerAddress(const std::string &v) {
            joinServerAddress = v;
            return *this;
        }
        LauncherMinecraftConfig &setJoinServerPort(const std::string &v) {
            joinServerPort = v;
            return *this;
        }

        LauncherMinecraftConfig &setResolutionWidth(const std::string &v) {
            resolutionWidth = v;
            return *this;
        }
        LauncherMinecraftConfig &setResolutionHeight(const std::string &v) {
            resolutionHeight = v;
            return *this;
        }
        LauncherMinecraftConfig &setTolerantMode(bool v) {
            tolerantMode = v;
            return *this;
        }
        LauncherMinecraftConfig &setMaxMemoryLimit(int v) {
            maxMemoryLimit = v;
            return *this;
        }
        LauncherMinecraftConfig &setMinMemoryLimit(int v) {
            minMemoryLimit = v;
            return *this;
        }
        LauncherMinecraftConfig &setNeedMemoryLimit(int v) {
            needMemoryLimit = v;
            return *this;
        }
        LauncherMinecraftConfig &setIsDemoUser(bool v) {
            isDemoUser = v;
            return *this;
        }
        LauncherMinecraftConfig &setHasCustomResolution(bool v) {
            hasCustomResolution = v;
            return *this;
        }
    };

    namespace internal {

        using namespace neko::ops::pipe;

        std::string buildMinecraftVersionDir(const std::string &targetDir, const std::string &versionName) noexcept {
            return util::math::sum(targetDir, "/", versionName) | util::unifiedPath;
        }

        std::string buildMinecraftVersionJsonPath(const std::string &targetDir, const std::string &versionName) noexcept {
            return util::math::sum(targetDir, "/", versionName, "/", versionName, ".json") | util::unifiedPath;
        }

        /// @param targetDir The target directory to check. e.g. "./.minecraft"
        /// @returns The absolute path of the Minecraft directory as a string. e.g. "/path/to/.minecraft"
        /// @throws ex::FileError if the target directory does not exist or is not a directory.
        std::string getAbsoluteMinecraftPath(const std::string &targetDir) {
            if (!std::filesystem::is_directory(targetDir)) {
                throw ex::FileError(
                    std::string("Minecraft directory not found or is not a directory: ") + targetDir,
                    ex::ExceptionExtensionInfo{});
            }
            return std::filesystem::absolute(targetDir).string() | util::unifiedPath;
        }

        /// @param targetDir The target directory to search for Minecraft versions. e.g. "/path/to/.minecraft/versions"
        /// @returns The name of the first Minecraft version found in the target directory.
        /// @throws ex::FileError if no Minecraft version is found.
        std::string getMinecraftVersionName(const std::string &targetDir) {
            for (const auto &it : std::filesystem::directory_iterator(targetDir)) {
                if (it.is_directory()) {
                    return it.path().filename().string() | util::unifiedPath;
                }
            }
            throw ex::FileError(
                std::string("No minecraft version found in: ") + targetDir,
                ex::ExceptionExtensionInfo{});
        }

        /// @brief Retrieves a list of all Minecraft versions found in the specified directory.
        /// @param targetDir The target directory to search for Minecraft versions. e.g. "/path/to/.minecraft/versions"
        /// @returns A vector of strings containing the names of all Minecraft versions found. if none found, returns an empty vector.
        std::vector<std::string> getMinecraftVersionList(const std::string &targetDir) {
            std::vector<std::string> versionList;
            for (const auto &it : std::filesystem::directory_iterator(targetDir)) {
                if (it.is_directory()) {
                    versionList.push_back(it.path().filename().string() | util::unifiedPath);
                }
            }
            return versionList;
        }

        /// @brief Reads the content of a Minecraft version JSON file.
        /// @param targetPath The path to the Minecraft version JSON file.
        /// @returns The content of the Minecraft version JSON file as a string.
        /// @throws ex::FileError if the file does not exist or cannot be opened.
        std::string getMinecraftVersionJsonContent(const std::string &targetPath) {
            if (!std::filesystem::exists(targetPath)) {
                throw ex::FileError(
                    std::string("minecraft version json file not exists: ") + targetPath,
                    ex::ExceptionExtensionInfo{});
            }

            std::ifstream jsonFile(targetPath);
            if (!jsonFile.is_open()) {
                throw ex::FileError(
                    std::string("failed to open minecraft version json file: ") + targetPath,
                    ex::ExceptionExtensionInfo{});
            }
            std::ostringstream jsonOss;
            jsonOss << jsonFile.rdbuf();
            return jsonOss.str();
        }

        /// @param path The file path to check and unified paths
        /// @returns The absolute file path as a string.
        /// @throws ex::FileError if the path does not exist or is not a regular file.
        std::string getAbsoluteFilePath(const std::string &path) {
            if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
                throw ex::FileError(
                    std::string("Path is not exists or is not a file: ") + path,
                    ex::ExceptionExtensionInfo{});
            }
            return std::filesystem::absolute(path).string() | util::unifiedPath;
        }

        /// @brief If dirPath is not a directory or not exists, an throw FileError exception
        /// @param errorMsg The error message to be included in the exception if the directory does not exist.
        /// @param errorMsg e.g the "directory not exists: "
        /// @throws ex::FileError if the directory does not exist or is not a directory.
        void assertDirectoryExists(const std::string &dirPath, const std::string &errorMsg) {
            if (!std::filesystem::is_directory(dirPath)) {
                throw ex::FileError(
                    errorMsg + dirPath,
                    ex::ExceptionExtensionInfo{});
            }
        }

        /**
         * @brief Constructs a file path from a raw name in the format "package:name:version".
         * @param rawName The raw name string to be processed.
         * @return A string representing the constructed file path. e.g., "package/name/version/name-version.jar".
         * @throws ex::Parse if the raw name does not match the expected format.
         */
        std::string constructPath(const std::string &rawName) {
            std::smatch match;
            if (std::regex_match(rawName, match, std::regex("([^:]+):([^:]+):([^:]+)"))) {
                std::string package = match[1].str();
                std::string name = match[2].str();
                std::string version = match[3].str();
                std::replace(package.begin(), package.end(), '.', '/');
                return package + "/" + name + "/" + version + "/" + name + "-" + version + ".jar";
            }
            throw ex::Parse{
                "Invalid raw name : " + rawName + ", expected format: package:name:version",
                ex::ExceptionExtensionInfo{}};
        };

        /**
         * @brief Constructs a classpath string from a vector of paths.
         * @param paths A vector of strings representing the paths to be included in the classpath.
         * @param osName The name of the operating system (e.g., "windows", "linux", "macos").
         * @return A string representing the classpath, with paths separated by the appropriate separator for the OS.
         */
        std::string constructClassPath(const std::vector<std::string> &paths, const std::string &osName) {
            const std::string separator = (osName == "windows") ? ";" : ":";
            return std::accumulate(std::next(paths.begin()), paths.end(), paths[0],
                                   [&](std::string acc, const std::string &path) {
                                       return acc + separator + path;
                                   });
        };

        // replace placeholders
        void applyPlaceholders(std::vector<std::string> &argsVec, const std::map<std::string, std::string> &placeholders) {
            for (auto &arg : argsVec) {
                for (const auto &[key, value] : placeholders) {
                    std::string::size_type pos;
                    while ((pos = arg.find(key)) != std::string::npos) {
                        arg.replace(pos, key.length(), value);
                    }
                }
            }
        };
        std::vector<std::string> withPlaceholdersReplaced(const std::vector<std::string> &argsVec, const std::map<std::string, std::string> &placeholders) {
            std::vector<std::string> result = argsVec;
            for (auto &arg : result) {
                for (const auto &[key, value] : placeholders) {
                    std::string::size_type pos;
                    while ((pos = arg.find(key)) != std::string::npos) {
                        arg.replace(pos, key.length(), value);
                    }
                }
            }
            return result;
        };

        struct RulesMap {
            std::string
                action,
                osName,
                osVersion,
                osArch;
            bool
                isDemoUser = false,
                hasCustomResolution = false;
        };

        struct Classifiers {
            std::string
                path,
                url,
                sha1;
            neko::uint32 size;
            bool empty() {
                for (auto it : {path.empty(), url.empty(), sha1.empty()}) {
                    if (!it) {
                        return false;
                    }
                }
                return true;
            }
        };

        struct ArtifactMap {
            Classifiers
                artifact,
                classifiers;
            std::string natives;

            bool empty() {
                for (auto it : {artifact.empty(), natives.empty(), classifiers.empty()}) {
                    if (!it) {
                        return false;
                    }
                }
                return true;
            };
        };

        /**
         * @brief Downloads a single archive file.
         * @param single The Classifiers object containing the archive information.
         * @throws ex::NetworkError if the download fails.
         */
        void downloadTask(const Classifiers &single) {
            network::Network net;
            network::RequestConfig reqConfig;
            reqConfig.setUrl(single.url)
                .setMethod(network::RequestType::DownloadFile)
                .setFileName(single.path)
                .setRequestId("minecraft-archives-" + single.sha1 + "-" + util::random::generateRandomString(6));
            auto res = net.executeWithRetry(reqConfig);
            if (!res.isSuccess()) {
                throw ex::NetworkError{
                    "Archives download failed, path: " + single.path + ", ex sha1: " + single.sha1 + ", error: " + res.errorMessage,
                    ex::ExceptionExtensionInfo{}};
            }
        };

        bool checkFeatures(const RulesMap &rules, const LauncherMinecraftConfig &cfg) noexcept {
            if (rules.isDemoUser && !cfg.isDemoUser)
                return false;
            if (rules.hasCustomResolution && !cfg.hasCustomResolution)
                return false;
            return true;
        };

        /// @throws ex::Parse if versionRegexStr invalid , an throw Parse exception
        bool matchOsVersion(const std::string &versionRegexStr) {
            try {
                std::regex versionRegex(versionRegexStr);
                return std::regex_search(system::getOsVersion(), versionRegex);
            } catch (const std::regex_error &e) {
                throw ex::Parse{
                    "Invalid OS version regex: " + versionRegexStr + ", system version: " + system::getOsVersion() + ", error: " + e.what(),
                    ex::ExceptionExtensionInfo{}};
            }
        }

        /// @throws ex::Parse if the OS version regex is invalid and tolerant mode is disabled
        bool checkOs(const RulesMap &rules, const LauncherMinecraftConfig &cfg) {
            if (!rules.osName.empty() && rules.osName != system::getOsName())
                return false;
            if (!rules.osArch.empty() && rules.osArch != system::getOsArch())
                return false;
            try {
                if (!rules.osVersion.empty() && !matchOsVersion(rules.osVersion))
                    return false;
            } catch (const ex::Parse &e) {
                if (!cfg.tolerantMode) {
                    throw;
                }
                log::Warn(log::SrcLoc::current(), "Failed to match OS version with regex '%s': %s", rules.osVersion.c_str(), e.what());
            }

            return true;
        }

        /// @brief Checks if the given object is allowed by the rules defined in the Minecraft version JSON.
        /// @param obj The JSON object to check, which can be any element in the args.jvm or libraries array.
        /// @param cfg The launcher configuration.
        /// @return True if the object is allowed by the rules, false otherwise.
        /// @throws ex::Parse if the OS version regex is invalid and tolerant mode is disabled
        bool isAllowedByRules(const nlohmann::json &obj, const LauncherMinecraftConfig &cfg) {
            if (!obj.contains("rules") || obj["rules"].empty())
                return true;

            bool allowed = false;
            // Check each rule in the rules array
            for (const auto &rules : obj["rules"]) {
                RulesMap rulesMap;
                rulesMap.action = rules.value("action", "");

                bool osOk = true,
                     featuresOk = true;

                if (rules.contains("os")) {
                    auto os = rules["os"];
                    rulesMap.osName = os.value("name", "");
                    rulesMap.osVersion = os.value("version", "");
                    rulesMap.osArch = os.value("arch", "");
                    osOk = checkOs(rulesMap, cfg);
                }

                if (rules.contains("features") && rules["features"].is_object()) {
                    auto features = rules["features"];
                    rulesMap.featuresIsDemoUser = features.value("isDemoUser", false);
                    rulesMap.featuresHasCustomResolution = features.value("hasCustomResolution", false);
                    featuresOk = checkFeatures(rulesMap, cfg);
                }

                if (osOk && featuresOk) {
                    allowed = (rulesMap.action == "allow");
                } else if (rulesMap.action == "disallow") {
                    // if any rule is disallowed, the whole object is disallowed. return false
                    allowed = false;
                    break;
                }
            }
            return allowed;
        }

        void uncompress(const std::string &zipFilePath, const std::string &destDir) {
            archive::ExtractConfig config{
                .destDir = destDir,
                .inputArchivePath = zipFilePath,
                .overwrite = true};
            try {
                archive::zip::extract(config);
            } catch (const ex::FileError &e) {
                throw;
            }
        }

        /**
         * @brief Checks file integrity and attempts to repair incomplete or missing files.
         * @param artifact The artifact map containing information about the archives.
         * @param maxRetries The maximum number of retries for each download attempt.
         * @throws ex::NetworkError if the download fails after the maximum number of retries.
         * @throws ex::FileError if the hash of the downloaded file does not match the expected SHA1.
         */
        void checkArchives(const ArtifactMap &artifact, int maxRetries = 5) {

            std::vector<Classifiers> SingleVector;

            if (!artifact.natives.empty()) {
                SingleVector.push_back({artifact.classifiers.path,
                                        artifact.classifiers.url,
                                        artifact.classifiers.sha1,
                                        artifact.classifiers.size});
            }
            SingleVector.push_back({artifact.artifact.path,
                                    artifact.artifact.url,
                                    artifact.artifact.sha1,
                                    artifact.artifact.size});

            for (const auto &it : SingleVector) {

                // auto retry task
                for (neko::uint32 i = 0; i < maxRetries; ++i) {

                    // download the file if it does not exist or is not a regular file
                    if (!std::filesystem::is_regular_file(it.path)) {
                        if (i == 0) {
                            log::Warn(log::SrcLoc::current(), "Archives not exists , path : %s , ready to download", it.path.c_str());
                        }

                        try {
                            downloadTask(it);
                        } catch (const ex::NetworkError &e) {
                            if (i >= maxRetries - 1) {
                                log::Error(log::SrcLoc::current(), "Archives download failed after multiple attempts, path : %s , sha1 : %s", it.path.c_str(), it.sha1.c_str());
                                throw;
                            }

                            log::Error(log::SrcLoc::current(), "Archives download failed, path: %s, sha1: %s, error: %s", it.path.c_str(), it.sha1.c_str(), e.what());
                            continue;
                        }
                    }

                    // check the file hash
                    auto hash = util::hash::hashFile(it.path, util::hash::Algorithm::sha1);
                    if (hash != it.sha1) {

                        // hash mismatch, try to remove the file
                        try {
                            std::filesystem::remove(it.path);
                        } catch (const std::filesystem::filesystem_error &e) {
                            if (i >= maxRetries - 1) {
                                throw ex::FileError{
                                    "Failed to remove file after multiple attempts, path: " + it.path + ", error: " + e.what(),
                                    ex::ExceptionExtensionInfo{}};
                            }
                            log::Error(log::SrcLoc::current(),
                                       "Failed to remove file, path: %s, error code: %d, error: %s. Will retry. (attempt %u/%u)",
                                       it.path.c_str(), e.code().value(), e.what(), i + 1, maxRetries);
                            continue;
                        }
                        // remove successful

                        // if failed to max retries, throw an error
                        if (i >= maxRetries - 1) {
                            log::Error(log::SrcLoc::current(), "Archives hash match failed after multiple attempts, path : %s , sha1 : %s", it.path.c_str(), it.sha1.c_str());
                            throw ex::FileError{
                                "Archives hash match failed after multiple attempts, ex sha1: " + it.sha1 + ", sha1: " + hash + ", path: " + it.path,
                                ex::ExceptionExtensionInfo{}};
                        }
                        // retry again
                        log::Warn(log::SrcLoc::current(), "Archives hash not match , try the download again, ex sha1 : %s , sha1 : %s , path : %s", it.sha1.c_str(), hash.c_str(), it.path.c_str());
                        continue;
                    }

                    // looks good, break the auto retry loop
                    log::Debug(log::SrcLoc::current(), "Archives exists and hash match , path : %s , sha1 : %s", it.path.c_str(), it.sha1.c_str());
                    break;
                }
            }
        };

        /// @throws ex::Parse if the OS version regex is invalid and tolerant mode is disabled
        std::vector<std::string> parseMinecraftVersionArguments(const nlohmann::json &arguments, const LauncherMinecraftConfig &cfg) {
            std::vector<std::string> result;
            for (const auto &it : arguments) {

                if (it.is_string()) {
                    log::Debug(log::SrcLoc::current(), "Push string : %s", it.get<std::string>().c_str());
                    result.push_back(it.get<std::string>());
                    continue;
                }
                if (!it.is_object()) {
                    log::Warn(log::SrcLoc::current(), "Unexpected type (not object and not string): %s", it.type_name());
                    continue;
                }
                if (!it.contains("value"))
                    continue;

                if (!it.contains("rules") || it["rules"].empty()) {
                    if (it["value"].is_string()) {
                        result.push_back(it["value"].get<std::string>());
                    } else if (it["value"].is_array()) {
                        for (const auto &pushArg : it["value"]) {
                            result.push_back(pushArg.get<std::string>());
                        }
                    }
                    continue;
                }

                if (isAllowedByRules(it, cfg)) {
                    if (it["value"].is_string()) {
                        result.push_back(it["value"].get<std::string>());
                    } else if (it["value"].is_array()) {
                        for (const auto &pushArg : it["value"]) {
                            result.push_back(pushArg.get<std::string>());
                        }
                    }
                }
            }
            return result;
        };

        /**
         * @brief Retrieves the paths of libraries based on the provided JSON configuration.
         * @param libraries The JSON object containing library information.
         * @param librariesPath The base path where libraries are located, e.g. "/path/to/.minecraft/libraries".
         * @param cfg The launcher configuration.
         */
        std::vector<std::string> getLibrariesPaths(const nlohmann::json &libraries, const std::string &librariesPath, const std::string &nativePath, const LauncherMinecraftConfig &cfg) {
            std::vector<std::string> librariesPaths;

            for (const auto &lib : libraries) {
                if (!isAllowedByRules(lib, cfg))
                    continue;

                if (!lib.contains("name")) {
                    log::Warn(log::SrcLoc::current(), "Library missing required 'name' field: %s", lib.dump().c_str());
                    continue;
                }

                std::string libNativePath;

                // check archive and repair if needed
                if (lib.contains("downloads") && lib["downloads"].contains("artifact")) {
                    ArtifactMap artifactMap;
                    const auto &artifactJson = lib["downloads"]["artifact"];
                    artifactMap.artifact.path = artifactJson.value("path", "");
                    artifactMap.artifact.url = artifactJson.value("url", "");
                    artifactMap.artifact.sha1 = artifactJson.value("sha1", "");
                    artifactMap.artifact.size = artifactJson.value("size", 0U);

                    if (artifactMap.artifact.path.empty() || artifactMap.artifact.url.empty() || artifactMap.artifact.sha1.empty()) {
                        log::Warn(log::SrcLoc::current(), "Library artifact missing required fields (path, url, sha1): %s", lib.dump().c_str());
                        continue;
                    }

                    artifactMap.artifact.path = librariesPath + "/" + artifactMap.artifact.path;

                    if (lib.contains("natives")) {
                        for (auto natives : lib["natives"].items()) {
                            if (natives.key() == system::getOsName()) {
                                artifactMap.natives = natives.value();
                                if (lib["downloads"]["classifiers"].contains(artifactMap.natives)) {
                                    const auto &classifiers = lib["downloads"]["classifiers"][artifactMap.natives];
                                    artifactMap.classifiers.path = librariesPath + "/" + classifiers.value("path", "");
                                    // add the native path to libNativePath
                                    libNativePath = artifactMap.classifiers.path;
                                    artifactMap.classifiers.url = classifiers.value("url", "");
                                    artifactMap.classifiers.sha1 = classifiers.value("sha1", "");
                                    artifactMap.classifiers.size = classifiers.value("size", 0U);
                                }
                            }
                        }
                    }
                    try {
                        checkArchives(artifactMap);
                    } catch (const ex::Exception &e) {
                        if (!cfg.tolerantMode) {
                            throw;
                        }
                        log::Error(log::SrcLoc::current(), "Failed to checkArchives , error : %s", e.what());
                    }
                }

                // If libNativePath is not empty and the check and repair pass, then decompress it.
                if (!libNativePath.empty() && std::filesystem::is_directory(nativePath)) {
                    uncompress(libNativePath, nativePath);
                }

                // Note: Forge may not include fields like "downloads", so it cannot be repaired; just try to add it directly
                std::string path = librariesPath + "/" + constructPath(lib.value("name", ""));
                nlog::Debug(log::SrcLoc::current(), "Push path : %s", path.c_str());
                librariesPaths.push_back(path);
            }
            return librariesPaths;
        }

        /**
         * @brief Downloads the Authlib Injector JAR file
         * @param authlibPath The path where the Authlib Injector JAR file should be saved.
         * @param configIni The configuration INI file to save the Authlib Injector SHA256 hash.
         * @throws ex::NetworkError if the download fails
         * @throws ex::Parse if the JSON response cannot be parsed correctly.
         * @throws ex::FileError if the downloaded file's SHA256 hash does not match the expected value.
         */
        void downloadAuthlibInjector(const std::string &authlibPath, CSimpleIniA &configIni) {
            log::autoLog log;

            network::Network net;
            network::RequestConfig reqConfig;

            auto url = net.buildUrl(network::NetworkBase::Api::Authlib::Injector::latest, network::NetworkBase::Api::Authlib::Injector::downloadHost);
            reqConfig.setUrl(url)
                .setMethod(network::RequestType::Get)
                .setRequestId("minecraft-authlib-injector-latest");

            auto res = net.executeWithRetry(reqConfig);
            if (!res.isSuccess() || !res.hasContent()) {
                throw ex::NetworkError{
                    "Failed to Get latest Authlib Injector version, error: " + res.errorMessage,
                    ex::ExceptionExtensionInfo{}};
            }

            nlohmann::json authlibVersionInfo;
            std::string
                downloadUrl,
                checksumSha256;
            try {
                authlibVersionInfo = nlohmann::json::parse(res.content);
                downloadUrl = authlibVersionInfo.at("download_url").get<std::string>();
                checksumSha256 = authlibVersionInfo.at("checksums").at("sha256").get<std::string>();
            } catch (const nlohmann::json::parse_error &e) {
                throw ex::Parse{
                    "Failed to parse Authlib Injector version info, error: " + std::string(e.what()),
                    ex::ExceptionExtensionInfo{}};
            } catch (const nlohmann::json::out_of_range &e) {
                throw ex::Parse{
                    "Authlib Injector version info does not contain 'download_url', error: " + std::string(e.what()),
                    ex::ExceptionExtensionInfo{}};
            }

            network::RequestConfig downloadConfig;
            downloadConfig.setUrl(downloadUrl)
                .setMethod(network::RequestType::DownloadFile)
                .setFileName(authlibPath)
                .setRequestId("minecraft-authlib-injector-download");

            auto downloadRes = net.executeWithRetry(downloadConfig);
            if (!downloadRes.isSuccess()) {
                throw ex::NetworkError{
                    "Failed to download Authlib Injector, error: " + downloadRes.errorMessage,
                    ex::ExceptionExtensionInfo{}};
            }

            auto hash = util::hash::hashFile(authlibPath, util::hash::Algorithm::sha256);
            if (hash != checksumSha256) {
                throw ex::FileError{
                    "Downloaded Authlib Injector hash does not match expected SHA256, expected: " + checksumSha256 + ", got: " + hash,
                    ex::ExceptionExtensionInfo{}};
            }

            // Save the checksum to the config file
            neko::ClientConfig cfg(configIni);
            cfg.minecraft.authlibSha256 = checksumSha256;
            cfg.save(configIni, info::app::getConfigFileName());
            log::Info(log::SrcLoc::current(), "Authlib Injector downloaded successfully: %s , hash sha256 : %s", authlibPath.c_str(), checksumSha256.c_str());
        }

        std::vector<std::string> getAuthlibVector(const std::string &minecraftDir, const LauncherMinecraftConfig &cfg, CSimpleIniA &configIni = core::getConfigObj()) {

            // authlib meta prefetched
            std::string authlibPrefetched = cfg.authlibPrefetched;
            // Since the config file may add escape backslashes, remove them before use
            authlibPrefetched.erase(std::remove(authlibPrefetched.begin(), authlibPrefetched.end(), '\\'), authlibPrefetched.end());

            // /path/to/.minecraft/<authlibName> (authlib-injector.jar)
            std::string authlibPath = minecraftDir + "/" + cfg.authlibName;

            auto hash = util::hash::hashFile(authlibPath, util::hash::Algorithm::sha256);

            if (!std::filesystem::exists(authlibPath)) {
                downloadAuthlibInjector(authlibPath, configIni);
            }

            if (!cfg.tolerantMode) {
                if (hash != cfg.authlib.sha256) {
                    try {
                        std::filesystem::remove(authlibPath);
                    } catch (const std::filesystem::filesystem_error &e) {
                        throw ex::FileError{
                            "Failed to remove file: " + authlibPath + ", error code: " + std::to_string(e.code()) ", error: " + e.what(),
                            ex::ExceptionExtensionInfo{}};
                    }
                    downloadAuthlibInjector(authlibPath, configIni);
                }
            }

            std::vector<std::string> result;
            result.push_back("-javaagent:" + authlibPath + "=" + network::NetworkBase::buildUrl(networkBase::Api::Authlib::root, networkBase::Api::Authlib::host));
            result.push_back("-Dauthlibinjector.side=client");
            result.push_back("-Dauthlibinjector.yggdrasil.prefetched=" + authlibPrefetched);
            return result;
        }

    } // namespace internal

    /// @throws ex::FileError if the minecraft folder is not a directory or does not exist
    /// @throws ex::Parse if the minecraft version json is invalid or does not contain required fields
    /// @throws ex::OutOfRange if the minecraft version json does not contain required keys
    /// @throws ex::NetworkError if the download fails or the file hash does not match
    /// @brief Launches Minecraft with the specified configuration.
    inline void launcherMinecraft(neko::ClientConfig cfg, std::function<void()> onStart = nullptr, std::function<void(int)> onExit = nullptr) {

        auto resolution = util::check::matchResolution(cfg.minecraft.customResolution);
        LauncherMinecraftConfig launcherCfg;
        launcherCfg.setMinecraftFolder(cfg.minecraft.minecraftFolder)
            .setJavaPath(cfg.minecraft.javaPath)
            .setPlayerName(cfg.minecraft.playerName)
            .setUuid(cfg.minecraft.uuid)
            .setAccessToken(cfg.minecraft.accessToken)
            .setTargetVersion(cfg.minecraft.targetVersion)
            .setMaxMemoryLimit(cfg.minecraft.maxMemoryLimit)
            .setMinMemoryLimit(cfg.minecraft.minMemoryLimit)
            .setNeedMemoryLimit(cfg.minecraft.needMemoryLimit)
            .setTolerantMode(cfg.minecraft.tolerantMode)
            .setHasCustomResolution(resolution.has_value())
            .setJoinServerAddress(cfg.minecraft.joinServerAddress)
            .setJoinServerPort(cfg.minecraft.joinServerPort);
        launcherCfg.authlib.setName(cfg.minecraft.authlibName)
            .setPrefetched(cfg.minecraft.authlibPrefetched)
            .setSha256(cfg.minecraft.authlib.sha256);

        if (resolution.has_value()) {
            launcherCfg.setCustomResolutionWidth(resolution.value().width)
                .setCustomResolutionHeight(resolution.value().height);
        }

        auto command = neko::minecraft::getLauncherMinecraftCommand(launcherCfg);
        core::launcherProcess(command, onStart, onExit, internal::getAbsoluteMinecraftPath(cfg.minecraft.minecraftFolder));
    }

    // may throw neko::ex FileError, Parse, OutOfRange , NetworkError
    inline std::string getLauncherMinecraftCommand(const LauncherMinecraftConfig &cfg) {
        nlog::autoLog log;

        // minecraft absolute path (e.g /path/to/.minecraft)
        const std::string minecraftDir = internal::getAbsoluteMinecraftPath(cfg.minecraftFolder);

        // a <version>
        const std::string minecraftVersionName = cfg.targetVersion.empty() ? internal::getMinecraftVersionName(minecraftDir + "/versions") : cfg.targetVersion;

        log::Info("minecraft version name : " + minecraftVersionName);

        // /path/to/.minecraft/versions/<version>
        const std::string minecraftVersionDir = internal::buildMinecraftVersionDir(minecraftDir + "/versions/", minecraftVersionName);

        internal::assertDirectoryExists(minecraftVersionDir, "minecraft version directory not exists: ");

        // /path/to/.minecraft/versions/<version>/<version>.json
        const std::string minecraftVersionJsonPath = internal::buildMinecraftVersionJsonPath(minecraftVersionDir, minecraftVersionName);

        const std::string minecraftVersionContent = internal::getMinecraftVersionJsonContent(minecraftVersionJsonPath);

        log::Info(log::SrcLoc::current(), "version file : %s ,content len : %zu", minecraftVersionJsonPath.c_str(), minecraftVersionContent.length());

        nlohmann::json minecraftVersionJsonObj;
        try {
            minecraftVersionJsonObj = nlohmann::json::parse(minecraftVersionContent);
        } catch (const nlohmann::json::parse_error &e) {
            throw ex::Parse{
                "Failed to parse minecraft version json: ", std::string(e.what()) + ", file : " + minecraftVersionJsonPath,
                ex::ExceptionExtensionInfo{}};
        }

        nlohmann::json
            baseArguments, // base "arguments" in version json
            jvmArguments,  // jvm args in "arguments"
            gameArguments, // game args in "arguments"
            libraries;     // "libraries" in version json
        try {
            baseArguments = minecraftVersionJsonObj.at("arguments");
            jvmArguments = baseArguments.at("jvm");
            gameArguments = baseArguments.at("game");
            libraries = minecraftVersionJsonObj.at("libraries");
        } catch (const nlohmann::json::out_of_range &e) {
            throw ex::OutOfRange{
                std::string("Required key not found in version json: ") + minecraftVersionJsonPath + ", error: " + e.what(),
                ex::ExceptionExtensionInfo{}};
        }

        // jvm
        const std::string
            javaPath = getAbsoluteFilePath(cfg.javaPath),
            mainClass = minecraftVersionJsonObj.value("mainClass", "net.minecraft.client.main.Main"),
            // /path/to/.minecraft/versions/<version>/<version>.jar
            clientJarPath = minecraftVersionDir + "/" + minecraftVersionJsonObj.value("jar", "") + ".jar",
            // /path/to/.minecraft/versions/<version>/natives
            nativesPath = minecraftVersionDir + "/natives",
            // /path/to/.minecraft/libraries
            librariesPath = minecraftDir + "/libraries";

        // game
        const std::string
            gameUsername = cfg.playerName,
            gameVersionName = "Neko Launcher",
            // /path/to/.minecraft/assets
            gameAssetsDir = minecraftDir + "/assets",
            gameUUID = cfg.uuid,
            gameAccessToken = cfg.accessToken,
            gameUserType = "mojang",
            gameVersionType = gameVersionName;

        // assets id , e.g 1.16
        std::string gameAssetsId;

        try {
            gameAssetsId = minecraftVersionJsonObj.at("assetIndex").at("id").get<std::string>();
        } catch (const nlohmann::json::out_of_range &e) {
            throw ex::OutOfRange{
                std::string("AssetIndex id not found in version json: ") + minecraftVersionJsonPath,
                ex::ExceptionExtensionInfo{}};
        }

        internal::assertDirectoryExists(librariesPath, "libraries directory not exists: ");

        const std::string nativePath = system::tempFolder() + "/NekoLc_natives_" + util::random::generateRandomString(8);

        // libraries paths, each string is a path
        const std::vector<std::string> librariesPaths = internal::getLibrariesPaths(libraries, librariesPath, nativePath, cfg);

        // All class path string, e.g  /path/to/.minecraft/libraries/<package>/<name>/<version>/<name>-<version>.jar; ... ; /path/to/.minecraft/version/<version>/<version>.jar
        const std::string classPath = internal::constructClassPath(librariesPaths, system::getOsName()) + ((system::getOsName() == std::string_view("windows")) ? ";" : ":") + clientJarPath;

        std::vector<std::string> jvmArgumentsVector = internal::parseMinecraftVersionArguments(jvmArguments, cfg);
        std::vector<std::string> gameArgumentsVector = internal::parseMinecraftVersionArguments(gameArguments, cfg);

        // jvm
        internal::applyPlaceholders(
            jvmArgumentsVector,
            {{"${natives_directory}", nativesPath},
             {"${library_directory}", librariesPath},
             {"${launcher_name}", "Neko Launcher"},
             {"${launcher_version}", info::app::getVersion()},
             {"${classpath}", classPath}});

        // game
        internal::applyPlaceholders(
            gameArgumentsVector,
            {{"${auth_player_name}", gameUsername},
             {"${version_name}", gameVersionName},
             {"${game_directory}", minecraftDir},
             {"${assets_root}", gameAssetsDir},
             {"${assets_index_name}", gameAssetsId},
             {"${auth_uuid}", gameUUID},
             {"${auth_access_token}", gameAccessToken},
             {"${user_type}", gameUserType},
             {"${version_type}", gameVersionType},
             {"${resolution_width}", cfg.resolutionWidth},
             {"${resolution_height}", cfg.resolutionHeight}});

        // memory limits calculation

        struct MemoryLimits {
            std::string maxMemory;
            std::string minMemory;
        };

        auto calcMemoryLimits = [](const LauncherMinecraftConfig &cfg) -> MemoryLimits {
            constexpr neko::uint64 oneGbyte = 1024 * 1024 * 1024; // 1 GB in bytes
            constexpr neko::uint64 oneMbyte = 1024 * 1024;        // 1 MB in bytes

            auto memoryInfo = system::getSystemMemoryInfo();

            if (memoryInfo.has_value()) {
                neko::uint64 totalBytes = memoryInfo.value().totalBytes;
                if (totalBytes < cfg.needMemoryLimit * oneGbyte) {
                    log::Err(log::SrcLoc::current(), "system memory is not enough , total memory : %zu GB (%zu MB) , need : %d GB", totalBytes / oneGbyte, totalBytes / oneMbyte, cfg.needMemoryLimit);
                    throw ex::Runtime(std::string("System memory is not enough , total memory : ") + std::to_string(totalBytes / oneGbyte) + " GB ( " + std::to_string(totalBytes / oneMbyte) + "MB ) , need : " + std::to_string(cfg.needMemoryLimit) + " GB",
                                      ex::ExceptionExtensionInfo{});
                }
            }

            neko::uint64 maxLimit = std::max(cfg.maxMemoryLimit, cfg.needMemoryLimit);
            neko::uint64 minLimit = std::min(cfg.minMemoryLimit, maxLimit);

            if (minLimit < 0 || maxLimit < 0) {
                throw ex::InvalidArgument{
                    "Memory limit cannot be negative, minLimit: " + std::to_string(minLimit) + ", maxLimit: " + std::to_string(maxLimit),
                    ex::ExceptionExtensionInfo{}};
            }

            return MemoryLimits{
                "-Xmx" + std::to_string(maxLimit) + "G",
                "-Xms" + std::to_string(minLimit) + "G"};
        };

        auto addJoinServer = [](std::vector<std::string> &gameArgs, const std::string &server, const std::string &port) -> void {
            if (!server.empty()) {
                gameArgs.push_back("--server");
                gameArgs.push_back(server);
                if (!port.empty()) {
                    gameArgs.push_back("--port");
                    gameArgs.push_back(port);
                }
            }
        };

        auto joinArgs = [](const std::vector<std::string> &list, const std::string &wrapper = "\"") -> std::string {
            std::string res;
            for (const auto &it : list) {
                res += wrapper + it + wrapper + " ";
            }
            return res;
        };

        auto [minMemory, maxMemory] = calcMemoryLimits(cfg);

        // jvm optimize args
        std::vector<std::string> jvmOptimizeArguments = {
            "-XX:+UnlockExperimentalVMOptions",
            "-XX:+UseG1GC",
            "-XX:G1NewSizePercent=20",
            "-XX:G1ReservePercent=20",
            "-XX:MaxGCPauseMillis=50",
            // "-XX:-OmitStackTraceInFastThrow", // if you want to see the stack trace in fast throw
            "-Dfml.ignoreInvalidMinecraftCertificates=true",
            "-Dfml.ignorePatchDiscrepancies=true",
            // "-Xlog:gc*:file=gc.log:time,level,tags", // java9+
            // "-XX:+PrintGCDetails", "-XX:+PrintGCDateStamps", "-Xloggc:gc.log" // java8
            minMemory,
            maxMemory};

        // join server if needed
        addJoinServer(gameArgumentsVector, cfg.joinServerAddress, cfg.joinServerPort);

        // authlib Injector
        std::vector<std::string> authlibInjectorVector;
        if (cfg.authlib.enabled)
            authlibInjectorVector = internal::getAuthlibVector(minecraftDir, cfg);

        const std::string command = joinArgs({javaPath}) + joinArgs(jvmOptimizeArguments) + joinArgs(jvmArgumentsVector) + joinArgs(authlibInjectorVector) + joinArgs({mainClass}) + joinArgs(gameArgumentsVector);

        // Mask the game token before logging to avoid security issues.
        internal::applyPlaceholders(gameArgumentsVector, {{gameAccessToken, "***********"}});
        log::Debug(log::SrcLoc::current(), "command len : %zu", command.size());
        log::Debug(log::SrcLoc::current(), "jvm optimize arguments : %s", joinArgs(jvmOptimizeArguments).c_str());
        log::Debug(log::SrcLoc::current(), "jvm arguments : %s", joinArgs(jvmArgumentsVector).c_str());
        log::Debug(log::SrcLoc::current(), "game arguments : %s", joinArgs(gameArgumentsVector).c_str());
        log::Debug(log::SrcLoc::current(), "authlib injector arguments : %s", joinArgs(authlibInjectorVector).c_str());

        return command;
    }
} // namespace neko::minecraft
