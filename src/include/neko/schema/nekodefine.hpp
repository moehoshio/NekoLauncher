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

        constexpr neko::strview AppName = "NekoLauncher";

        constexpr neko::strview NekoLcCoreVersion = "v0.0.1";

        constexpr neko::strview NetworkHostList[] = {"api.example.com", "www.example.org"};

        constexpr neko::strview NetworkAuthlibHost = "skin.example.org";

        constexpr bool useAuthentication = true;

        constexpr bool useStaticDeployment = false;

        constexpr bool useWebSocket = true;

        // If only static deployment configuration . always enabled when useStaticDeployment is true
        constexpr bool useStaticRemoteConfig = false;
        // URL for the remote configuration file
        constexpr neko::strview NetworkRemoteConfigUrl = "https://static.example.com/NekoLc/v0/remote-config.json";

        constexpr neko::strview launcherMode = "minecraft";

        constexpr neko::strview clientConfigFileName = "config.ini";

        constexpr neko::uint64 NetworkHostListSize = sizeof(NetworkHostList) / sizeof(NetworkHostList[0]);


        // Helper namespace for constexpr build ID generation
        namespace constexprBuildId {

            constexpr char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
            constexpr neko::uint64 charset_size = sizeof(charset) - 1;

            consteval neko::uint64 consteval_strlen(neko::cstr str) {
                neko::uint64 len = 0;
                while (str[len] != '\0') {
                    ++len;
                }
                return len;
            }

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

            template <neko::uint64 N1, neko::uint64 N2, neko::uint64 N3>
            constexpr auto concat3(neko::cstr version, neko::cstr time, neko::cstr id) {
                std::array<char, N1 + 1 + N2 + 1 + N3 + 1> arr{};
                neko::uint64 idx = 0;
                for (neko::uint64 i = 0; i < N1; ++i) { arr[idx++] = version[i]; }
                arr[idx++] = '-';
                for (neko::uint64 i = 0; i < N2; ++i) { arr[idx++] = time[i]; }
                arr[idx++] = '-';
                for (neko::uint64 i = 0; i < N3; ++i) { arr[idx++] = id[i]; }
                arr[idx] = '\0';
                return arr;
            }

            // If GIT_COMMIT_ID is defined, use it as the identifier; otherwise, use make_identifier to generate a seemingly random identifier
            constexpr auto build_id_array =
#if defined(GIT_COMMIT_ID)
                std::array<char, sizeof(GIT_COMMIT_ID)>{GIT_COMMIT_ID};
#else
                make_identifier<8>(__TIME__, __DATE__, __FILE__);
#endif

            // The complete identifier is composed in the form of version-time-unique_identifier
            constexpr auto build_id_full_array = concat3<NekoLcCoreVersion.size(), consteval_strlen(BUILD_TIME), build_id_array.size()>(NekoLcCoreVersion.data(), BUILD_TIME, build_id_array.data());

        } // namespace constexprBuildId

        /**
         * @brief // This is the build ID, which is a combination of the core version, build time, and a unique identifier.
         * @var buildID
         * @example "v0.0.1-20250710184724-githash"
         */
        constexpr std::string_view buildID(constexprBuildId::build_id_full_array.data(), constexprBuildId::build_id_full_array.size());

    } // namespace definitions

} // namespace neko::schema