// exec.h
// cpp Lang
// Version 0.0.2
// https://github.com/moehoshio/exec.h
// MIT License
// Welcome to  submit questions, light up star , error corrections (even just for better translations), and feature suggestions/construction.
// :D
/*
MIT License
Copyright (c) 2024 Hoshi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

// NekoLc Project Customization
#include "library/SimpleIni/SimpleIni.h"
#include "library/threadpool.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <random>
#include <regex>
#include <string>

#if __has_include("openssl/md5.h") && __has_include("openssl/sha.h")
#define _USE_OPENSSL
#endif

// hash
#if defined(_USE_OPENSSL)
#include <fstream>
#include <iomanip>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sstream>
#include <unordered_map>
#endif

namespace neko {

    constexpr inline decltype(auto) operator|(auto &&val, auto &&func) {
        return func(std::forward<decltype(val)>(val));
    }

    template <class T, class F>
    constexpr inline decltype(auto) operator|(const std::initializer_list<T> &val, F &&func) {
        return func(std::forward<std::initializer_list<T>>(val));
    }

} // namespace neko

namespace exec {

    // NekoLc Project Customization
    inline neko::ThreadPool &getThreadObj() {
        static neko::ThreadPool obj;
        return obj;
    };

    inline CSimpleIniA &getConfigObj() {
        static CSimpleIniA obj;
        return obj;
    }
    // Nekolc end

#if defined(_USE_OPENSSL)

    namespace hashs {
        enum class Algorithm {
            none,
            md5,
            sha1,
            sha256,
            sha512
        };
    } // namespace hashs

    using namespace std::literals;
    inline std::unordered_map<hashs::Algorithm, std::string> hashAlgorithmMap = {
        {hashs::Algorithm::md5, "md5"s},
        {hashs::Algorithm::sha1, "sha1"s},
        {hashs::Algorithm::sha256, "sha256"s},
        {hashs::Algorithm::sha512, "sha512"s}};

    inline auto mapAlgorithm(const std::string &str) {
        for (auto it : hashAlgorithmMap) {
            if (it.second == str) {
                return it.first;
            }
        }
        return hashs::Algorithm::none;
    }
    inline auto mapAlgorithm(hashs::Algorithm algortihm) {
        for (auto it : hashAlgorithmMap) {
            if (it.first == algortihm) {
                return it.second;
            }
        }
        return std::string("unknown");
    }

    inline std::string hashStr(const std::string str, hashs::Algorithm algorithm = hashs::Algorithm::sha256) {
        const unsigned char *unsignedData = reinterpret_cast<const unsigned char *>(str.c_str());
        unsigned char outBuf[128];
        int condLen = 0;
        switch (algorithm) {
            case hashs::Algorithm::sha1:
                SHA1(unsignedData, str.size(), outBuf);
                condLen = SHA_DIGEST_LENGTH;
                break;
            case hashs::Algorithm::sha256:
                SHA256(unsignedData, str.size(), outBuf);
                condLen = SHA256_DIGEST_LENGTH;
                break;
            case hashs::Algorithm::sha512:
                SHA512(unsignedData, str.size(), outBuf);
                condLen = SHA512_DIGEST_LENGTH;
                break;
            case hashs::Algorithm::md5:
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

    inline std::string hashFile(const std::string &name, hashs::Algorithm algorithm = hashs::Algorithm::sha256) {
        std::ifstream file(name, std::ios::binary);
        std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return hashStr(raw, algorithm);
    }

    inline std::string hash(const std::string &hash, bool isFileName = false, hashs::Algorithm algorithm = hashs::Algorithm::sha256) {
        return (isFileName) ? hashFile(hash, algorithm) : hashStr(hash, algorithm);
    }

#endif // use openssl

    inline std::string base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

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

    template <typename T = std::string>
    inline T plusSingleQuotes(const T &str) {
        return "'" + str + "'";
    }

    template <typename T = std::string>
    inline T plusDoubleQuotes(const T &str) {
        return "\"" + str + "\"";
    }

    template <typename T = std::string>
    inline T unifiedThePaths(const T &inPath) {
        T res = inPath;
        std::replace(res.begin(), res.end(), '\\', '/');
        return res;
    }

    inline void execfs(const std::string &name, auto &&...args) {
        if (!std::filesystem::exists(name))
            return;
        std::system((name + (args + ...)).c_str());
    }

    inline void execfe(const char *name) {
        if (!std::filesystem::exists(name))
            return;

        std::system((name + std::filesystem::current_path().string()).c_str());
    }

    inline void execf(const char *name) {
        if (!std::filesystem::exists(name))
            return;

        std::system(name);
    }
    constexpr auto plusDoubleQuote = [](auto &&val) { return exec::plusDoubleQuotes(val); };
    constexpr auto plusSingleQuote = [](auto &&val) { return exec::plusSingleQuotes(val); };
    constexpr auto unifiedPaths = [](auto &&val) { return exec::unifiedThePaths(val); };
    constexpr auto move = [](auto &&val) -> auto && { return std::move(val); };
    constexpr auto make_shared = [](auto &&val) -> decltype(auto) { return std::make_shared<std::remove_reference_t<decltype(val)>>(val); };

    constexpr inline auto copy(const auto &d) {
        return d;
    };

    template <typename T = std::string>
    constexpr inline T boolTo(bool v, const T &rTrue = "true", const T &rFalse = "false") {
        return ((v) ? rTrue : rFalse);
    };

    constexpr inline decltype(auto) sum(auto &&...args) {
        return (args + ...);
    };

    constexpr inline decltype(auto) product(auto &&...args) {
        return (args * ...);
    };

    constexpr inline bool allTrue(auto &&...args) {
        return (true && ... && args);
    };

    constexpr inline bool anyTrue(auto &&...args) {
        return (false || ... || args);
    };

    inline auto getTime() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::system_clock::to_time_t(now);
    }

    inline std::string getTimeString(const char *Format = "%Y-%m-%d-%H-%M-%S") {
        auto currentTime = getTime();
        std::tm *currentTm = std::localtime(&currentTime);

        char timeString[128];
        std::strftime(timeString, sizeof(timeString), Format, currentTm);
        return std::string(timeString);
    };

    inline std::string getTimestamp() {
        return std::to_string(getTime());
    }

    inline std::string generateUUID(int Digits = 32) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);

        const char *hex_chars = "0123456789abcdef";
        std::string uuid;

        for (int i = 0; i < Digits; ++i) {
            if (i == 8 || i == 12 || i == 16 || i == 20) {
                uuid += '-';
            } else {
                uuid += hex_chars[dis(gen)];
            }
        }
        return uuid;
    };

    // A random number , default with a maximum of 9 digits, it could be 0.
    inline size_t randNums(std::pair<int, int> digits = {1, 9}) {
        std::random_device rd;
        std::mt19937 rng(rd());

        std::uniform_int_distribution<int> distribution(0, 9);
        std::uniform_int_distribution<int> numsD2(digits.first, digits.second);
        std::string randomString;
        std::string numsList("0123456789");
        for (int i = 0; i < numsD2(rng); ++i) {
            randomString += numsList[distribution(rng)];
        }
        return std::stoull(randomString);
    }

    inline std::string generateRandomString(int length, const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") {

        std::random_device rd;
        std::mt19937 rng(rd());

        std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

        std::string randomString;
        for (int i = 0; i < length; ++i) {
            randomString += characters[distribution(rng)];
        }
        return randomString;
    };

    inline bool isUrl(const std::string &str) {
        std::regex url_regex("(http|https)://[a-zA-Z0-9\\-\\.]+\\.[a-zA-Z]{2,3}(/\\S*)?");
        return std::regex_match(str, url_regex);
    };

    inline bool isProxyAddress(const std::string &address) {
        std::regex proxyRegex(R"((http|https|socks5|socks4)://([\w.-]+)(:\d+))");
        return std::regex_match(address, proxyRegex);
    };

    inline bool matchExtName(const std::string &name, const std::string &targetExtension) {
        if (name.empty() || targetExtension.empty())
            return false;

        size_t lastDotIndex = name.find_last_of('.');

        if (lastDotIndex != std::string::npos) {
            std::string fileExtension = name.substr(lastDotIndex + 1);

#ifdef _WIN32
            for (auto &c : fileExtension) {
                c = tolower(c);
            }
#endif

            return fileExtension == targetExtension;
        }

        return false;
    }
    inline bool matchExtNames(const std::string &name, const std::vector<std::string> &targetExtensions) {
        for (const auto &it : targetExtensions) {
            if (matchExtName(name, it)) {
                return true;
            }
        }
        return false;
    }
    inline auto matchSizes(const std::string &s) {
        std::regex r("\\b(\\d+)x(\\d+)\\b");
        std::smatch match;
        std::regex_search(s, match, r);
        return match;
    }
    inline std::vector<std::string> matchSizesV(const std::string &s) {
        auto res = matchSizes(s);
        std::vector<std::string> v;
        for (const auto &it : res) {
            v.push_back(it);
        }
        return v;
    }
} // namespace exec
