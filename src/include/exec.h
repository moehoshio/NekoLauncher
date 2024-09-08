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

#include <chrono>
#include <random>
#include <regex>
#include <string>
#include <filesystem>

// NekoLc Project Customization
#include "SimpleIni/SimpleIni.h"
#include "threadpool.h"

// hash
#include <fstream>
#include <iomanip>
#include <sstream>
#include <openssl/md5.h>
#include <openssl/sha.h>


namespace neko {

    constexpr inline decltype(auto) operator|(auto &&val, auto &&func) {
        return func(std::forward<decltype(val)>(val));
    }

    template <class T, class F>
    constexpr inline decltype(auto) operator|(const std::initializer_list<T> &val, F &&func) {
        return func(std::forward<std::initializer_list<T>>(val));
    }
    namespace range {

        template <typename T>
        concept Range = std::ranges::range<T>;
        constexpr inline decltype(auto) operator|(Range auto &&val, auto &&func) {
            for (const auto &it : val) {
                func(val);
            }
            return val;
        }
    } // namespace range

} // namespace neko

namespace exec {

    // NekoLc Project Customization
    neko::ThreadPool &getThreadObj();

    CSimpleIniA &getConfigObj();

    namespace hashs {
        enum class Algorithm {
            md5,
            sha1,
            sha256,
            sha512
        };
    } // namespace hashs

    inline std::string hashStr(const std::string str, hashs::Algorithm algorithm = hashs::Algorithm::sha256) {
        const unsigned char *unsignedData = reinterpret_cast<const unsigned char *>(str.c_str());
        unsigned char outBuf[SHA256_DIGEST_LENGTH];
        switch (algorithm) {
            case hashs::Algorithm::sha1:
                SHA1(unsignedData, str.size(), outBuf);
                break;
            case hashs::Algorithm::sha256:
                SHA256(unsignedData, str.size(), outBuf);
                break;
            case hashs::Algorithm::sha512:
                SHA512(unsignedData, str.size(), outBuf);
                break;
            case hashs::Algorithm::md5:
                MD5(unsignedData, str.size(), outBuf);
                break;
            default:
                break;
        }
        std::stringstream ssRes;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
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

    // Nekolc end

    inline void execf(const char *name) {
        if (!std::filesystem::exists(name))
            return;
        std::system(name);
    }

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

    inline std::string getTimeString(const char *Format = "%Y-%m-%d-%H-%M-%S") {
        auto currentTimePoint = std::chrono::system_clock::now();

        std::time_t currentTime = std::chrono::system_clock::to_time_t(currentTimePoint);
        std::tm *currentTm = std::localtime(&currentTime);

        char timeString[128];
        std::strftime(timeString, sizeof(timeString), Format, currentTm);
        return std::string(timeString);
    };

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
