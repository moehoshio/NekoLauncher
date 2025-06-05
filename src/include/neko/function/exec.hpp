/**
 * @file exec.hpp
 * @brief Utilities for execution, path handling, string manipulation, and hash computation.
 *
 * This header provides a collection of utilities:
 * - Functional programming operators
 * - Hash computation (MD5, SHA1, SHA256, SHA512)
 * - Path manipulation and normalization
 * - UUID generation and manipulation
 * - Base64 encoding/decoding
 * - String utilities
 * - Random value generators
 * - File extension matching
 *
 * @dependencies
 * - SimpleIni: For configuration handling
 * - threadpool.hpp: For thread pool operations
 * - OpenSSL (optional): For hash computations
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <random>
#include <regex>
#include <string>

// Enable hash support if OpenSSL is available
#if __has_include("openssl/md5.h") && __has_include("openssl/sha.h")
#define _USE_OPENSSL
#endif

// hash support
#if defined(_USE_OPENSSL)
#include <fstream>
#include <iomanip>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sstream>
#include <unordered_map>
#endif

/**
 * @namespace neko::operator
 * @brief Contains operator overloads for functional programming style operations.
 * @note This conflicts with the syntax of std::ranges::views. Use namespace qualification if necessary.
 */
namespace neko::operator{

    /**
     * @brief Pipe operator for functional composition.
     * @tparam T Type of the value
     * @tparam F Type of the function
     * @param val Value to be passed to the function
     * @param func Function to be applied to the value
     * @return Result of applying the function to the value
     */
    template <typename T, typename F>
        requires requires(T &&v, F &&f) { { std::forward<F>(f)(std::forward<T>(v)) }; }
    constexpr decltype(auto) operator|(T &&val, F &&func) {
        return std::forward<F>(func)(std::forward<T>(val));
    }

    /**
     * @brief Pipe operator overload for initializer lists.
     * @tparam T Type of elements in the initializer list
     * @tparam F Type of the function
     * @param val Initializer list to be passed to the function
     * @param func Function to be applied to the initializer list
     * @return Result of applying the function to the initializer list
     */
    template <typename T, typename F>
        requires requires(const std::initializer_list<T> &val, F &&f) { { std::forward<F>(f)(val) }; }
    constexpr decltype(auto) operator|(const std::initializer_list<T> &val, F &&func) {
        return std::forward<F>(func)(val);
    }

} // namespace neko::operator

/**
 * @namespace neko::exec
 * @brief Execution and utility functions for the Neko launcher.
 */
namespace neko::exec {

#if defined(_USE_OPENSSL)
    /**
     * @namespace neko::exec::hash
     * @brief Hash computation utilities using OpenSSL.
     */
    namespace hash {
        /**
         * @enum Algorithm
         * @brief Supported hash algorithms.
         */
        enum class Algorithm {
            none,   ///< No algorithm
            md5,    ///< MD5 algorithm
            sha1,   ///< SHA-1 algorithm
            sha256, ///< SHA-256 algorithm
            sha512  ///< SHA-512 algorithm
        };

        /**
         * @brief Mapping between hash algorithms and their string representations.
         */
        inline std::unordered_map<hash::Algorithm, std::string> hashAlgorithmMap = {
            {hash::Algorithm::md5, "md5"},
            {hash::Algorithm::sha1, "sha1"},
            {hash::Algorithm::sha256, "sha256"},
            {hash::Algorithm::sha512, "sha512"}};

        /**
         * @brief Maps a string to a hash algorithm.
         * @param str String representation of a hash algorithm
         * @return Corresponding hash algorithm enum value
         */
        inline auto mapAlgorithm(const std::string &str) {
            for (auto it : hashAlgorithmMap) {
                if (it.second == str) {
                    return it.first;
                }
            }
            return hash::Algorithm::none;
        }

        /**
         * @brief Maps a hash algorithm to its string representation.
         * @param algorithm Hash algorithm enum value
         * @return String representation of the hash algorithm
         */
        inline auto mapAlgorithm(hash::Algorithm algorithm) {
            for (auto it : hashAlgorithmMap) {
                if (it.first == algorithm) {
                    return it.second;
                }
            }
            return std::string("unknown");
        }

        /**
         * @brief Computes the hash of a string.
         * @param str String to hash
         * @param algorithm Hash algorithm to use
         * @return Hexadecimal string representation of the hash
         */
        inline std::string hashString(const std::string str, hash::Algorithm algorithm = hash::Algorithm::sha256) {
            const unsigned char *unsignedData = reinterpret_cast<const unsigned char *>(str.c_str());
            unsigned char outBuf[128];
            int condLen = 0;
            switch (algorithm) {
                case hash::Algorithm::sha1:
                    SHA1(unsignedData, str.size(), outBuf);
                    condLen = SHA_DIGEST_LENGTH;
                    break;
                case hash::Algorithm::sha256:
                    SHA256(unsignedData, str.size(), outBuf);
                    condLen = SHA256_DIGEST_LENGTH;
                    break;
                case hash::Algorithm::sha512:
                    SHA512(unsignedData, str.size(), outBuf);
                    condLen = SHA512_DIGEST_LENGTH;
                    break;
                case hash::Algorithm::md5:
                    MD5(unsignedData, str.size(), outBuf);
                    condLen = MD5_DIGEST_LENGTH;
                    break;
                default:
                    break;
            }
            std::stringstream ssRes;
            for (int i = 0; i < condLen; ++i) {
                ssRes << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(outBuf[i]);
            }
            return ssRes.str();
        }

        /**
         * @brief Computes the hash of a file.
         * @param name Path to the file
         * @param algorithm Hash algorithm to use
         * @return Hexadecimal string representation of the hash
         */
        inline std::string hashFile(const std::string &name, hash::Algorithm algorithm = hash::Algorithm::sha256) {
            std::ifstream file(name, std::ios::binary);
            std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            return hashString(raw, algorithm);
        }

        /**
         * @brief Computes the hash of a string or file.
         * @param hash String to hash or path to file
         * @param isFileName If true, hash is treated as a file path; otherwise as a string
         * @param algorithm Hash algorithm to use
         * @return Hexadecimal string representation of the hash
         */
        inline std::string hash(const std::string &hash, bool isFileName = false, hash::Algorithm algorithm = hash::Algorithm::sha256) {
            return (isFileName) ? hashFile(hash, algorithm) : hashString(hash, algorithm);
        }

    } // namespace hash

    /**
     * @brief Converts a UUID string to bytes.
     * @param uuid UUID string in standard format
     * @return Array of 16 bytes representing the UUID
     */
    inline std::array<uint8_t, 16> uuidStringToBytes(const std::string &uuid) {
        std::array<uint8_t, 16> bytes{};
        int idx = 0;
        for (size_t i = 0; i < uuid.length() && idx < 16; ++i) {
            if (uuid[i] == '-')
                continue;
            uint8_t value = 0;
            if (uuid[i] >= '0' && uuid[i] <= '9')
                value = uuid[i] - '0';
            else if (uuid[i] >= 'a' && uuid[i] <= 'f')
                value = uuid[i] - 'a' + 10;
            else if (uuid[i] >= 'A' && uuid[i] <= 'F')
                value = uuid[i] - 'A' + 10;
            else
                continue;
            value <<= 4;
            ++i;
            if (i < uuid.length() && uuid[i] != '-') {
                if (uuid[i] >= '0' && uuid[i] <= '9')
                    value |= uuid[i] - '0';
                else if (uuid[i] >= 'a' && uuid[i] <= 'f')
                    value |= uuid[i] - 'a' + 10;
                else if (uuid[i] >= 'A' && uuid[i] <= 'F')
                    value |= uuid[i] - 'A' + 10;
            } else {
                --i;
            }
            bytes[idx++] = value;
        }
        return bytes;
    }

    /**
     * @brief Generates a version 3 UUID based on a namespace UUID and a name.
     * @param namespace_uuid Namespace UUID in string format
     * @param name Name to use for UUID generation
     * @return Version 3 UUID string
     * @example uuidV3("6ba7b810-9dad-11d1-80b4-00c04fd430c8", "example.com")
     * Example Namespace UUID: "6ba7b810-9dad-11d1-80b4-00c04fd430c8"
     *
     * @note This requires using hash values, so OpenSSL support is needed
     */
    inline std::string uuidV3(const std::string &namespace_uuid, const std::string &name) {
        auto ns_bytes = uuidStringToBytes(namespace_uuid);

        std::string to_hash(reinterpret_cast<const char *>(ns_bytes.data()), ns_bytes.size());
        to_hash += name;

        std::string md5hex = hashString(to_hash, hash::Algorithm::md5);

        std::array<uint8_t, 16> hash_bytes{};
        for (int i = 0; i < 16; ++i) {
            hash_bytes[i] = static_cast<uint8_t>(std::stoi(md5hex.substr(i * 2, 2), nullptr, 16));
        }

        hash_bytes[6] = (hash_bytes[6] & 0x0F) | 0x30; // version 3
        hash_bytes[8] = (hash_bytes[8] & 0x3F) | 0x80; // RFC4122 variant

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (int i = 0; i < 16; ++i) {
            oss << std::setw(2) << static_cast<int>(hash_bytes[i]);
            if (i == 3 || i == 5 || i == 7 || i == 9)
                oss << '-';
        }
        return oss.str();
    }
#endif // use openssl

    /**
     * @brief Characters used for Base64 encoding.
     */
    constexpr inline std::string_view base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    /**
     * @brief Encodes a string using Base64.
     * @param input String to encode
     * @return Base64 encoded string
     */
    inline std::string base64Encode(const std::string &input) {
        std::string encodedString;
        int val = 0;
        int bits = -6;
        const unsigned int b63 = 0x3F;

        for (unsigned char c : input) {
            val = (val << 8) + c;
            bits += 8;
            while (bits >= 0) {
                encodedString.push_back(base64Chars[(val >> bits) & b63]);
                bits -= 6;
            }
        }

        if (bits > -6) {
            encodedString.push_back(base64Chars[((val << 8) >> (bits + 8)) & b63]);
        }

        while (encodedString.size() % 4) {
            encodedString.push_back('=');
        }

        return encodedString;
    }

    /**
     * @brief Decodes a Base64 encoded string.
     * @param input Base64 encoded string
     * @return Decoded string
     */
    inline std::string base64Decode(const std::string &input) {
        std::string decodedString;
        std::vector<int> base64Index(256, -1);
        for (int i = 0; i < 64; i++) {
            base64Index[base64Chars[i]] = i;
        }

        int val = 0;
        int bits = -8;
        for (unsigned char c : input) {
            if (base64Index[c] == -1)
                break;
            val = (val << 6) + base64Index[c];
            bits += 6;

            if (bits >= 0) {
                decodedString.push_back(char((val >> bits) & 0xFF));
                bits -= 8;
            }
        }

        return decodedString;
    }

    /**
     * @brief Adds single quotes around a string.
     * @tparam T String type
     * @param str String to quote
     * @return String with single quotes
     */
    template <typename T = std::string>
    inline T plusSingleQuotes(const T &str) {
        return "'" + str + "'";
    }

    /**
     * @brief Adds double quotes around a string.
     * @tparam T String type
     * @param str String to quote
     * @return String with double quotes
     */
    template <typename T = std::string>
    inline T plusDoubleQuotes(const T &str) {
        return "\"" + str + "\"";
    }

    /**
     * @brief Normalizes path separators to forward slashes.
     * @tparam T String type
     * @param inPath Path to normalize
     * @return Normalized path with forward slashes
     */
    template <typename T = std::string>
    inline T unifiedThePaths(const T &inPath) {
        T res = inPath;
        std::replace(res.begin(), res.end(), '\\', '/');
        return res;
    }

    /**
     * @brief Lambda for adding double quotes to a value.
     */
    constexpr auto plusDoubleQuote = [](auto &&val) { return exec::plusDoubleQuotes(val); };

    /**
     * @brief Lambda for adding single quotes to a value.
     */
    constexpr auto plusSingleQuote = [](auto &&val) { return exec::plusSingleQuotes(val); };

    /**
     * @brief Lambda for normalizing path separators.
     */
    constexpr auto unifiedPaths = [](auto &&val) { return exec::unifiedThePaths(val); };

    /**
     * @brief Lambda for moving a value.
     */
    constexpr auto move = [](auto &&val) -> auto && { return std::move(val); };

    /**
     * @brief Lambda for creating a shared pointer.
     */
    constexpr auto makeShared = [](auto &&val) -> decltype(auto) { return std::make_shared<std::remove_reference_t<decltype(val)>>(std::forward<decltype(val)>(val)); };

    /**
     * @brief Creates a copy of a value.
     * @param d Value to copy
     * @return Copy of the value
     */
    constexpr inline auto copy(const auto &d) {
        return std::decay_t<decltype(d)>(d);
    };

    /**
     * @brief Converts a boolean to a string or other type.
     * @tparam T Return type
     * @param v Boolean value to convert
     * @param rTrue Value to return if v is true
     * @param rFalse Value to return if v is false
     * @return rTrue if v is true, rFalse otherwise
     */
    template <typename T = std::string_view>
    constexpr T boolTo(bool v, const T &rTrue = T("true"), const T &rFalse = T("false")) {
        return ((v) ? rTrue : rFalse);
    };

    /**
     * @brief Computes the sum of all arguments.
     * @param args Values to sum
     * @return Sum of all arguments
     */
    constexpr inline decltype(auto) sum(auto &&...args) {
        static_assert(sizeof...(args) > 0, "sum requires at least one argument");
        return (args + ...);
    };

    /**
     * @brief Computes the product of all arguments.
     * @param args Values to multiply
     * @return Product of all arguments
     */
    constexpr inline decltype(auto) product(auto &&...args) {
        static_assert(sizeof...(args) > 0, "product requires at least one argument");
        return (args * ...);
    };

    /**
     * @brief Checks if all arguments are true.
     * @param args Boolean values to check
     * @return true if all arguments are true, false otherwise
     */
    constexpr inline bool allTrue(auto &&...args) {
        return (true && ... && args);
    };

    /**
     * @brief Checks if any argument is true.
     * @param args Boolean values to check
     * @return true if any argument is true, false otherwise
     */
    constexpr inline bool anyTrue(auto &&...args) {
        return (false || ... || args);
    };

    /**
     * @brief Gets the current system time.
     * @return Current time as a time_t value
     */
    inline std::time_t getTime() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::system_clock::to_time_t(now);
    }

    /**
     * @brief Formats the current time as a string.
     * @param format Format string for strftime
     * @return Formatted time string
     */
    inline std::string getTimeString(std::string_view format = "%Y-%m-%d-%H-%M-%S") {
        auto currentTime = getTime();
        std::tm tmResult;

#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tmResult, &currentTime);
#else
        localtime_r(&currentTime, &tmResult);
#endif

        char timeString[128];
        if (std::strftime(timeString, sizeof(timeString), format.data(), &tmResult) == 0) {
            return std::string();
        }
        return std::string(timeString);
    }

    /**
     * @brief Gets the current time as a timestamp string.
     * @return Current time as a string of seconds since epoch
     */
    inline std::string getTimestamp() {
        return std::to_string(getTime());
    }

    /**
     * @brief Generates a version 4 (random) UUID.
     * @return UUID string in standard format
     */
    inline std::string uuidV4() {
        static thread_local std::mt19937 gen{std::random_device{}()};
        static thread_local std::uniform_int_distribution<uint32_t> dis(0, 0xffffffff);

        uint32_t data[4];
        for (auto &d : data)
            d = dis(gen);

        // Set the version (4) and variant (10xx)
        data[1] = (data[1] & 0xFFFF0FFF) | 0x00004000;
        data[2] = (data[2] & 0x3FFFFFFF) | 0x80000000;

        char buf[37];
        std::snprintf(
            buf, sizeof(buf),
            "%08x-%04x-%04x-%04x-%04x%08x",
            data[0],
            (data[1] >> 16) & 0xFFFF, data[1] & 0xFFFF,
            (data[2] >> 16) & 0xFFFF, data[2] & 0xFFFF,
            data[3]);
        return std::string(buf);
    }

    /**
     * @brief Generates a random hexadecimal string.
     * @param digits Number of hexadecimal digits to generate
     * @return Random hexadecimal string
     */
    inline std::string randomHex(int digits = 32) {
        static thread_local std::mt19937 gen{std::random_device{}()};
        static thread_local std::uniform_int_distribution<> dis(0, 15);
        const char *hex_chars = "0123456789abcdef";
        std::string result;
        result.reserve(digits);
        for (int i = 0; i < digits; ++i)
            result += hex_chars[dis(gen)];
        return result;
    }

    /**
     * @brief Generates a random number with a specified number of digits.
     * @param digits Pair specifying minimum and maximum number of digits
     * @return Random number
     */
    inline uint64_t randomNDigitNumber(std::pair<int, int> digits = {1, 9}) {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> lenDist(digits.first, digits.second);
        int len = lenDist(rng);

        uint64_t min = static_cast<uint64_t>(std::pow(10, len - 1));
        uint64_t max = static_cast<uint64_t>(std::pow(10, len)) - 1;

        std::uniform_int_distribution<uint64_t> numDist(min, max);
        return numDist(rng);
    }

    /**
     * @brief Generates a random string of specified length.
     * @param length Length of the string to generate
     * @param characters Character set to use for generation
     * @return Random string
     *
     * @throw std::invalid_argument if characters is empty or length is negative
     */
    inline std::string generateRandomString(
        int length,
        std::string_view characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") {
        if (characters.empty()) {
            throw std::invalid_argument("characters list must not be empty");
        }
        if (length < 0) {
            throw std::invalid_argument("length must be non-negative");
        }

        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<size_t> distribution(0, characters.size() - 1);

        std::string randomString;
        randomString.reserve(length);

        for (int i = 0; i < length; ++i) {
            randomString += characters[distribution(rng)];
        }
        return randomString;
    }

    /**
     * @brief Checks if a string is a valid URL.
     * @param str String to check
     * @return true if the string is a valid URL, false otherwise
     */
    inline bool isUrl(std::string_view str) {
        static const std::regex url_regex("(http|https)://[a-zA-Z0-9\\-\\.]+\\.[a-zA-Z]{2,3}(/\\S*)?");
        return std::regex_match(str, url_regex);
    };

    /**
     * @brief Checks if a string is a valid proxy address.
     * @param address String to check
     * @return true if the string is a valid proxy address, false otherwise
     */
    inline bool isProxyAddress(std::string_view address) {
        static const std::regex proxyRegex(R"((http|https|socks5|socks4)://([\w.-]+)(:\d+))");
        return std::regex_match(address, proxyRegex);
    };

    /**
     * @brief Checks if a file has a specific extension.
     * @param name Filename to check
     * @param targetExtension Extension to match
     * @param caseSensitive Whether to perform case-sensitive matching
     * @return true if the file has the specified extension, false otherwise
     */
    inline bool matchExtName(std::string_view name, std::string_view targetExtension, bool caseSensitive = false) {
        if (name.empty() || targetExtension.empty())
            return false;
        auto lastDot = name.find_last_of('.');
        if (lastDot == std::string_view::npos)
            return false;
        auto fileExtension = name.substr(lastDot + 1);

        auto normalize = [](std::string_view sv, bool lower) {
            std::string s(sv);
            if (lower)
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };

        if (caseSensitive)
            return normalize(fileExtension, false) == normalize(targetExtension, false);
        else
            return normalize(fileExtension, true) == normalize(targetExtension, true);
    }

    /**
     * @brief Checks if a file has any of the specified extensions.
     * @param name Filename to check
     * @param targetExtensions Vector of extensions to match
     * @return true if the file has any of the specified extensions, false otherwise
     */
    inline bool matchExtNames(const std::string &name, const std::vector<std::string> &targetExtensions) {
        for (const auto &it : targetExtensions) {
            if (matchExtName(name, it)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Extracts size specifications (e.g., "800x600") from a string.
     * @param str String to parse
     * @return Vector of matched strings
     */
    inline std::vector<std::string> matchSizes(const std::string &str) {
        static const std::regex r(R"((\d+)x(\d+))");
        std::smatch match;
        std::vector<std::string> v;
        if (std::regex_search(str, match, r)) {
            for (size_t i = 0; i < match.size(); ++i) {
                v.push_back(match[i]);
            }
        }
        return v;
    }
} // namespace neko::exec
