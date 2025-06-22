#pragma once

#include "neko/function/detectFileType.hpp"
#include "neko/schema/exception.hpp"

#include <memory>
#include <string>
#include <vector>

namespace neko::archive {

    enum class CompressionLevel {
        none,    // No compression
        fast,    // Fast compression
        normal,  // Normal compression
        maximum, // Maximum compression
        ultra    // Ultra compression
    };

    enum class ZipEncryption {
        ZipCrypto,
        AES256
    };
    /**
     * @struct CreateConfig
     * @brief Configuration for creating an archive.
     *
     * This structure holds the configuration options for creating an archive,
     * including output path, password, input and exclusion rules, compression, and encryption.
    */
    struct CreateConfig {
        std::string
            outputArchivePath, // Path to the output archive file
            password; // Password for encryption (optional)

        /**
         * @brief List of input paths to include in the archive.
         * Supports various matching rules:
         * 1. File name (e.g., "file.txt"): Matches files with the exact name.
         * 2. Relative path (e.g., "user/abc.txt"):
         *    Matches any file whose path ends with this relative path.
         * 3. Absolute path (e.g., "/path/to/user/abc.txt"):
         *    Matches only if the full path is exactly the same.
         * 4. Folder (e.g., "logs/", "/path/to/user/logs/"):
         *    - Absolute path starting with '/' and ending with '/': Matches only if the full path is exactly the same.
         *    - Relative path (e.g., "logs/"): Matches any folder named "logs" at any level in the path.
         * 5. Wildcards (e.g., "*.txt", "logs/*.log"):
         *    Supports '*' as a wildcard for any characters.
         *    - Example: "logs/*.log" matches all ".log" files under the "logs/" folder.
         * 6. Regular expressions (e.g., "^logs/.*\\.log$"):
         *    Supports regex matching.
         *    - Example: "^logs/.*\\.log$" matches all ".log" files under the "logs/" folder.
         */
        std::vector<std::string> inputPaths;
        /**
         * @brief List of paths to exclude from the archive.
         * @var excludePaths
         *      List of paths to exclude from the archive.
         *      Supports the same matching rules as inputPaths.
         */
        std::vector<std::string> excludePaths;

        CompressionLevel compressionLevel = CompressionLevel::normal;
        ZipEncryption encryption = ZipEncryption::AES256;
    };

    struct ExtractConfig {
        std::string
            inputArchivePath, // Extract data from this archive file
            destDir,          // Extract the archive to this directory
            password;         // Password for extraction (optional)
            
        /**
         * @brief List of paths to include in the extraction (if empty, extract all).
         * Supports various matching rules:
         * 1. File name (e.g., "file.txt"): Matches files with the exact name.
         * 2. Relative path (e.g., "user/abc.txt"):
         *   Matches any file whose path ends with this relative path.
         * 3. Absolute path (e.g., "/path/to/user/abc.txt"):
         *   Matches only if the full path is exactly the same.
         * 4. Folder (e.g., "logs/", "/path/to/user/logs/"):
         *   - Absolute path starting with '/' and ending with '/': Matches only if the full path is exactly the same.
         *   - Relative path (e.g., "logs/"): Matches any folder named "logs" at any level in the path.
         * 5. Wildcards (e.g., "*.txt", "logs/*.log"):
         *   Supports '*' as a wildcard for any characters.
         *   - Example: "logs/*.log" matches all ".log" files under the "logs/" folder.
         * 6. Regular expressions (e.g., "^logs/.*\\.log$"):
         *   Supports regex matching.
         *   - Example: "^logs/.*\\.log$" matches all ".log" files under the "logs/" folder.
         */
        std::vector<std::string> includePaths;
        
        /**
         * @brief List of paths to exclude from the extraction.
         * @var excludePaths
         *      List of paths to exclude from the extraction.
         *      Supports the same matching rules as includePaths.
         */
        std::vector<std::string> excludePaths;

        /**
         * @brief Whether to overwrite existing files during extraction.
         * If true, existing files will be overwritten.
         * If false, existing files will be skipped.
         */
        bool overwrite = true;
    };

    inline bool isArchiveFile(const std::string &filePath) {
        return util::detect::isTargetFileType(
            filePath,
            {"zip", "tar", "gz", "bz2", "xz", "rar", "7z"},
            false);
    }

    namespace zip {

        /**
         * @brief Extracts the contents of a ZIP archive.
         * @param config Configuration for the extraction process.
         * @throws ex::FileError if the extraction fails.
         */
        void extract(const ExtractConfig &config);
        /**
         * @brief Creates a ZIP archive.
         * @param config Configuration for the creation process.
         * @throws ex::FileError if the creation fails.
         */
        void create(const CreateConfig &config);
        /**
         * @brief Checks if the given file is a ZIP archive.
         * @param filePath The path to the file to check.
         * @return True if the file is a ZIP archive, false otherwise.
         */
        inline bool isZipArchiveFile(const std::string &filePath) {
            return util::detect::isTargetFileType(
                filePath,
                {"zip", "jar", "apk", "xpi"},
                false);
        }

    } // namespace zip

} // namespace neko::archive