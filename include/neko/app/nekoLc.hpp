/**
 * @brief NekoLc configuration and constants.
 * @file nekoLc.hpp
 */

#pragma once

#include <neko/schema/types.hpp>

#include <array>
#include <string_view>

namespace neko::lc {

    /*******************/
    /*** Application ***/
    /*******************/

    // In this project, `App` refers to NekoLc; in contrast, `resource` refers to the managed/maintained content.
    constexpr neko::strview AppName = "NekoLauncher";

    constexpr neko::strview AppVersion = "v0.0.1";

    // Options: "custom", "minecraft"
    constexpr neko::strview LauncherMode = "minecraft";

    constexpr neko::strview ClientConfigFileName = "config.ini";

    /***************/
    /*** Network ***/
    /***************/

    constexpr neko::strview NetworkHostList[] = {"api.example.com", "www.example.org"};

    // Authlib-Injector for Minecraft
    constexpr neko::strview NetworkAuthlibHost = "skin.example.org";

    constexpr neko::uint64 NetworkHostListSize = sizeof(NetworkHostList) / sizeof(NetworkHostList[0]);

    // Whether to use authentication features
    constexpr bool EnableAuthentication = true;

    // When true, EnableStaticRemoteConfig is always true as well
    constexpr bool EnableStaticDeployment = false;

    constexpr bool EnableWebSocket = true;

    // If only static deployment configuration
    constexpr bool EnableStaticRemoteConfig = false;
    // URL for the static remote configuration file
    constexpr neko::strview NetworkStaticRemoteConfigUrl = "https://static.example.com/NekoLc/v0/remote-config.json";

    /****************/
    /*** Build ID ***/
    /****************/

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
            for (neko::uint64 i = 0; i < N1; ++i) {
                arr[idx++] = version[i];
            }
            arr[idx++] = '-';
            for (neko::uint64 i = 0; i < N2; ++i) {
                arr[idx++] = time[i];
            }
            arr[idx++] = '-';
            for (neko::uint64 i = 0; i < N3; ++i) {
                arr[idx++] = id[i];
            }
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

        constexpr auto build_id_full_array = concat3<AppVersion.size(), consteval_strlen(NEKO_LC_BUILD_TIME), build_id_array.size()>(AppVersion.data(), NEKO_LC_BUILD_TIME, build_id_array.data());

    } // namespace constexprBuildId

    /**
     * @brief This is the build ID, e.g the "v0.0.1-20250710184724-githash" which is a combination of the core version, build time, and a unique identifier.
     */
    constexpr std::string_view buildID(constexprBuildId::build_id_full_array.data(), constexprBuildId::build_id_full_array.size());

    
    /*********************/
    /*** API Endpoints ***/
    /*********************/

    namespace api {

        // NekoLc API endpoints
        constexpr neko::cstr testing = "/v0/testing/ping";
        constexpr neko::cstr echo = "/v0/testing/echo";

        constexpr neko::cstr login = "/v0/api/auth/login";
        constexpr neko::cstr refresh = "/v0/api/auth/refresh";
        constexpr neko::cstr validate = "/v0/api/auth/validate";
        constexpr neko::cstr logout = "/v0/api/auth/logout";

        constexpr neko::cstr launcherConfig = "/v0/api/launcherConfig";
        constexpr neko::cstr maintenance = "/v0/api/maintenance";
        constexpr neko::cstr checkUpdates = "/v0/api/checkUpdates";
        constexpr neko::cstr feedbackLog = "/v0/api/feedbackLog";

        namespace authlib {
            constexpr neko::cstr host = NetworkAuthlibHost.data();
            constexpr neko::cstr root = "/api/yggdrasil";
            constexpr neko::cstr authenticate = "/api/yggdrasil/authserver/authenticate";
            constexpr neko::cstr refresh = "/api/yggdrasil/authserver/refresh";
            constexpr neko::cstr validate = "/api/yggdrasil/authserver/validate";
            constexpr neko::cstr invalidate = "/api/yggdrasil/authserver/invalidate";
            constexpr neko::cstr signout = "/api/yggdrasil/authserver/signout";

            namespace injector {
                constexpr neko::cstr downloadHost = "authlib-injector.yushi.moe";
                constexpr neko::cstr bmclapiDownloadHost = "bmclapi2.bangbang93.com/mirrors/authlib-injector";
                constexpr neko::cstr getVersionsList = "/artifacts.json";
                constexpr neko::cstr latest = "/artifact/latest.json";
                constexpr neko::cstr baseRoot = "/artifact"; // + /{build_number}.json
            } // namespace injector

        } // namespace authlib
    } // namespace api
} // namespace neko::lc