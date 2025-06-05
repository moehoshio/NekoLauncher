#pragma once

#include "neko/schema/types.hpp"

#include <array>
#include <string_view>

namespace neko::schema {

    /**
     * @brief This namespace contains global definitions and constants used throughout the Neko framework.
     * @namespace neko::schema
     * @ingroup schema
     */
    namespace definitions {

        constexpr neko::cstr NekoLcCoreVersion = "v0.0.1";

        constexpr neko::cstr NetWorkHostList[] = {"api.example.com", "www.example.org"};

        constexpr neko::cstr NetWorkAuthlibHost = "skin.example.org";

        constexpr neko::cstr launcherMode = "minecraft";

        constexpr neko::cstr clientConfigFileName = "config.ini";

        // Unique identifier for the build
#if defined(GIT_BUILD_ID)
        constexpr inline const std::string_view build_id = GIT_BUILD_ID;
#else

        // A constexpr function to generate a unique identifier based on the current time, date, and file.
        namespace constexprBuildId {

            constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            constexpr neko::uint64 charset_size = sizeof(charset) - 1;

            constexpr neko::uint32 constexpr_hash(neko::cstr str, int h = 0) {
                return !str[h] ? 5381 : (constexpr_hash(str, h + 1) * 33) ^ str[h];
            }

            constexpr neko::uint32 combine_hashes(neko::uint32 a, neko::uint32 b) {
                return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
            }

            constexpr char pick_char(neko::uint32 &seed) {
                seed = seed * 1664525u + 1013904223u;
                return charset[seed % charset_size];
            }

            template <neko::uint64 N>
            constexpr auto make_identifier(neko::cstr time_str, neko::cstr date_str, neko::cstr file_str) {
                std::array<char, N + 1> arr{};
                neko::uint32 seed = combine_hashes(
                    combine_hashes(constexpr_hash(time_str), constexpr_hash(date_str)),
                    constexpr_hash(file_str));
                for (neko::uint64 i = 0; i < N; ++i) {
                    arr[i] = pick_char(seed);
                }
                arr[N] = '\0';
                return arr;
            }

            constexpr auto build_id_array = make_identifier<10>(__TIME__, __DATE__, __FILE__);

        } // namespace constexprBuildId

        constexpr std::string_view build_id(constexprBuildId::build_id_array.data(), 10);

#endif // Git_BUILD_ID

    } // namespace definitions

} // namespace neko::schema

// Import the logging module for nerr exceptions , in nerr.hpp
#define nerrImpLoggerModeDefine true