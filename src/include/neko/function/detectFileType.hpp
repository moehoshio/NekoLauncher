#pragma once

#include "neko/function/utilities.hpp"
#include "neko/schema/exception.hpp"
#include "neko/schema/types.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @namespace neko::util::detect
 * @brief Contains functions and types for file type detection.
 */
namespace neko::util::detect {

    /**
     * @namespace neko::util::detect::detail
     * @brief Contains implementation details for file type detection.
     */
    namespace detail {

        /**
         * @brief Represents a file signature for type detection.
         */
        struct FileSignature {
            /**
             * @brief The possible types for the file.
             * @example "TXT", "EXE", "ZIP", "7Z" , etc.
             */
            std::vector<std::string> types;
            /**
             * @brief The magic number (file signature) for the file.
             */
            std::vector<neko::uchar> magic;
            /**
             * @brief The possible file extensions for the file.
             * @note This is used to match the file extension after the magic number is detected.
             *       If the magic number matches but the extension does not, it will not be considered a match.
             *       This helps avoid false positives for files with the same magic number but different extensions.
             */
            std::vector<std::string> possibleExtensions;
        };

        const std::vector<FileSignature> signatures = {
            // text files
            {{"TXT"}, {'T', 'E', 'X', 'T'}, {"txt"}},
            {{"CSV"}, {'C', 'S', 'V'}, {"csv"}},
            {{"JSON"}, {'{', '"'}, {"json"}},
            {{"XML"}, {'<', '?', 'x', 'm', 'l'}, {"xml"}},
            {{"HTML"}, {'<', '!', 'D', 'O', 'C', 'T', 'Y', 'P', 'E'}, {"html", "htm"}},

            // image files
            {{"BMP"}, {'B', 'M'}, {"bmp"}},
            {{"GIF"}, {'G', 'I', 'F', '8'}, {"gif"}},
            {{"JPEG"}, {0xFF, 0xD8, 0xFF}, {"jpg", "jpeg"}},
            {{"PNG"}, {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A}, {"png"}},

            // audio files
            {{"MP3"}, {0x49, 0x44, 0x33}, {"mp3"}},
            {{"WAV", "AVI"}, {'R', 'I', 'F', 'F'}, {"wav", "avi"}}, // merged
            {{"FLAC"}, {'f', 'L', 'a', 'C'}, {"flac"}},
            {{"OGG"}, {'O', 'g', 'g', 'S'}, {"ogg"}},

            // video files
            {{"MP4"}, {0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70}, {"mp4"}},
            {{"MOV"}, {0x00, 0x00, 0x00, 0x14, 0x66, 0x74, 0x79, 0x70, 0x71, 0x74}, {"mov"}},
            {{"MKV"}, {'1', 'A', 'V', 'I'}, {"mkv"}},

            // archive files
            {{"ZIP", "DOCX", "XLSX", "PPTX", "APK"}, {0x50, 0x4B, 0x03, 0x04}, {"zip", "docx", "xlsx", "pptx", "apk"}},
            {{"RAR"}, {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00}, {"rar"}},
            {{"7Z"}, {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C}, {"7z"}},
            {{"TAR"}, {0x75, 'S', 'T', 'A', 'R'}, {"tar"}},
            {{"GZ"}, {0x1F, 0x8B}, {"gz"}},
            {{"BZ2"}, {0x42, 'Z', 'h'}, {"bz2"}},
            {{"LZMA"}, {0x5D, '0', '0', 'L', 'Z', 'M', 'A'}, {"lzma"}},
            {{"XZ"}, {0xFD, '7', 'z', 'X', 'Z', 0x00}, {"xz"}},
            {{"ZST"}, {0x28, 0xB5, 0x2F, 0xFD}, {"zst"}},
            {{"LZO"}, {'L', 'Z', 'O', '1'}, {"lzo"}},
            {{"LZ4"}, {'L', 'Z', '4', 'F'}, {"lz4"}},

            // executable files
            {{"EXE", "PE", "DLL", "SYS", "COM"}, {0x4D, 0x5A}, {"exe", "pe", "dll", "sys", "com"}},
            {{"ELF"}, {0x7F, 'E', 'L', 'F'}, {"elf"}},
            {{"MSI"}, {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1}, {"msi", "doc", "xls", "ppt"}},
            {{"MACHO"}, {0xFE, 0xED, 0xFA, 0xCE}, {"macho"}},
            {{"MACHO"}, {0xFE, 0xED, 0xFA, 0xCF}, {"macho"}},
            {{"MACHO"}, {0xCA, 0xFE, 0xBA, 0xBE}, {"macho"}},
            {{"MACHO"}, {0xCE, 0xFA, 0xED, 0xFE}, {"macho"}},
            {{"MACHO"}, {0xCF, 0xFA, 0xED, 0xFE}, {"macho"}},

            // document files
            {{"PDF"}, {0x25, 0x50, 0x44, 0x46, 0x2D}, {"pdf"}},
            {{"ISO"}, {0x43, 'D', 'F', 'I', 'M', 'I', 'S', 'O'}, {"iso"}},
            {{"PSD"}, {'8', 'B', 'P', 'S'}, {"psd"}},
            {{"SWF"}, {'F', 'W', 'S'}, {"swf"}},
            {{"SWF"}, {'C', 'W', 'S'}, {"swf"}},
            {{"SWF"}, {'Z', 'W', 'S'}, {"swf"}}};

        /**
         * @brief Serialize magic number to string for map key.
         */
        inline std::string magicToKey(const std::vector<neko::uchar> &magic) {
            std::ostringstream oss;
            for (auto c : magic)
                oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<neko::uint32>(c);
            return oss.str();
        }

        /**
         * @brief Get the extension to type mapping.
         */
        inline const std::unordered_map<std::string, std::string> &getExtensionTypeMap() {
            static std::unordered_map<std::string, std::string> extTypeMap;
            static bool initialized = false;
            if (!initialized) {
                for (const auto &sig : signatures) {
                    for (size_t i = 0; i < sig.possibleExtensions.size(); ++i) {
                        std::string ext = sig.possibleExtensions[i];
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        std::string type = (i < sig.types.size()) ? sig.types[i] : sig.types[0];
                        extTypeMap[ext] = type;
                    }
                }
                initialized = true;
            }
            return extTypeMap;
        }

        /**
         * @brief Get the magic number to type mapping.
         */
        inline const std::unordered_map<std::string, std::string> &getMagicTypeMap() {
            static std::unordered_map<std::string, std::string> magicTypeMap;
            static bool initialized = false;
            if (!initialized) {
                for (const auto &sig : signatures) {
                    if (!sig.magic.empty()) {
                        magicTypeMap[magicToKey(sig.magic)] = sig.types[0];
                    }
                }
                initialized = true;
            }
            return magicTypeMap;
        }

        /**
         * @brief Find type by file extension.
         * This function checks the extension of a file against a static map of known extensions and their types.
         * If the extension is found, it returns the corresponding type; otherwise, it returns "Unknown".
         * @param ext The file extension to check, in lowercase.
         * @return The type associated with the extension, or "Unknown" if not found.
         * @note The extension should be in lowercase for case-insensitive matching.
         * @example For example, "txt" returns "TXT", "jpg" returns "JPEG", etc.
         * @see getExtensionTypeMap for the map of extensions to types.
         */
        inline std::string typeByExtension(const std::string &ext) {
            auto &extTypeMap = getExtensionTypeMap();
            auto it = extTypeMap.find(ext);
            if (it != extTypeMap.end())
                return it->second;
            return "Unknown";
        }

        /**
         * @brief Find type by magic number.
         * This function checks the magic number of a file against a static map of known magic numbers and their types.
         * If the magic number is found, it returns the corresponding type; otherwise, it returns "Unknown".
         * @param buffer Pointer to the buffer containing the file data.
         * @param size Size of the buffer in bytes.
         * @return The type associated with the magic number, or "Unknown" if not found.
         * @note The buffer should contain at least the first few bytes of the file to match the magic number.
         * @example For example, a buffer starting with {0xFF, 0xD8, 0xFF} returns "JPEG", while a buffer starting with
         */
        inline std::string typeByMagic(const neko::uchar *buffer, size_t size) {
            auto &magicTypeMap = getMagicTypeMap();
            for (const auto &sig : signatures) {
                if (sig.magic.empty() || size < sig.magic.size())
                    continue;
                if (std::equal(sig.magic.begin(), sig.magic.end(), buffer)) {
                    return sig.types[0];
                }
            }
            return "Unknown";
        }

    } // namespace detail

    /**
     * @brief Detect the file type based on its content and extension.
     * This function reads the first few bytes of a file to determine its type using magic numbers.
     * If a magic number has multiple possible types (e.g., exe, dll), the function will try to determine the type using the file extension. Otherwise, it returns the first type.
     * @param filename The name of the file to check.
     * @param noex If true, returns "Unknown" instead of throwing an exception if the file cannot be opened or no magic number matches.
     * @return The detected file type as a string. e.g. "TXT", "PNG", "ZIP", "7Z", etc.
     * @throws ex::FileError if the file cannot be opened or if no magic number matches
     * @note The function reads up to 32 bytes from the file to check for magic numbers.
     */
    inline std::string detectFileType(const std::string &filename, bool noex = false) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            if (noex)
                return "Unknown";
            throw ex::FileError("Cannot open file: " + filename, ex::ExceptionExtensionInfo{});
        }

        neko::uchar buffer[32] = {0};
        file.read(reinterpret_cast<char *>(buffer), sizeof(buffer));
        size_t bytesRead = file.gcount();

        std::string ext = util::string::getExtensionName(filename);

        // 1. Try magic number map
        std::string type = detail::typeByMagic(buffer, bytesRead);
        if (type != "Unknown")
            return type;

        // 2. Try extension map
        type = detail::typeByExtension(ext);
        if (type != "Unknown")
            return type;

        if (noex)
            return "Unknown";

        std::ostringstream oss;
        oss << "Unknown file type: " << std::uppercase << std::hex;
        for (size_t i = 0; i < std::min(bytesRead, size_t(16)); ++i) {
            oss << std::setw(2) << std::setfill('0') << static_cast<neko::uint>(buffer[i]) << ' ';
        }

        throw ex::FileError(oss.str(), ex::ExceptionExtensionInfo{});
    }

} // namespace neko::util::detect
