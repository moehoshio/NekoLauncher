#pragma once

// core version , any string
#define NekoLcCoreVersionDefine "v0.0.1"

// server list ,  e.g "api.example.com","api.example.org","" ...
#define NetWorkHostListDefine "api.example.com","www.example.org"

// Option: `minecraft`. Used in the `launcher` function in core.hpp
constexpr const char *launcherMode = "minecraft"; 




// authlib host , e.g "auth.example.com"
#define NetWorkAuthlibHostDefine "skin.example.org"

// Import the logging module for nerr exceptions , in nerr.hpp
#define nerrImpLoggerModeDefine true




// unique identifier for the build
#include <string_view>

#if defined(GIT_BUILD_ID)
constexpr inline const std::string_view build_id = GIT_BUILD_ID;
#else

#include <array>

namespace constexprBuildId {

    // A constexpr function to generate a unique identifier based on the current time, date, and file.
    
    constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    constexpr std::size_t charset_size = sizeof(charset) - 1;

    constexpr unsigned int constexpr_hash(const char *str, int h = 0) {
        return !str[h] ? 5381 : (constexpr_hash(str, h + 1) * 33) ^ str[h];
    }

    constexpr unsigned int combine_hashes(unsigned int a, unsigned int b) {
        return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
    }

    constexpr char pick_char(unsigned int &seed) {
        seed = seed * 1664525u + 1013904223u;
        return charset[seed % charset_size];
    }

    template <std::size_t N>
    constexpr auto make_identifier(const char *time_str, const char *date_str, const char *file_str) {
        std::array<char, N + 1> arr{};
        unsigned int seed = combine_hashes(
            combine_hashes(constexpr_hash(time_str), constexpr_hash(date_str)),
            constexpr_hash(file_str));
        for (std::size_t i = 0; i < N; ++i) {
            arr[i] = pick_char(seed);
        }
        arr[N] = '\0';
        return arr;
    }
    
    constexpr auto build_id_array = make_identifier<10>(__TIME__, __DATE__, __FILE__);

} //namespace constexprBuildId

constexpr std::string_view build_id(constexprBuildId::build_id_array.data(), 10);

#endif //Git_BUILD_ID


using cstr = const char *;

using uint_64 =  unsigned long long;
using uint_32 = unsigned int;
using uint_16 = unsigned short;

using int_64 = long long;
using int_32 = int;
using int_16 = short;

using uint = uint_32;

// Define a convenient macro for printing logs
#define FN __FUNCTION__

#define FI __FILE__

#define LI __LINE__