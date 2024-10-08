#pragma once
#include "exec.h"
#include <filesystem>
#include <string>
namespace neko {

    class info {
    public:
        enum class getType {
            home,
            temp,
            version,
            resVersion,
            workDir,
            osName
        };

    private:
        struct Data {

            constexpr static const char *version =
#include "../data/version"
                ;
            constexpr static const char *website =
#include "../data/website"
                ;
            static std::string home;
            static std::string temp;
            constexpr static const char *osName =
#if _WIN32
                "windows";
#elif __APPLE__
                "osx";
#elif __linux__
                "linux";
#else
                "unknown";
#endif
        };

    public:
        inline static void setTemp(const std::string &tmp) {
            if (std::filesystem::is_directory(tmp))
                Data::temp = tmp | exec::unifiedPaths;
        }
        constexpr inline static std::string getVersion() {
            return Data::version;
        };
        inline static std::string getTemp() {
            return Data::temp;
        }
        inline static std::string getHome() {
            return Data::home;
        }
        inline static std::string getResVersion() {
            return exec::getConfigObj().GetValue("more", "resVersion", "");
        }
        inline static std::string getWorkDir() {
            return std::filesystem::current_path().string() | exec::unifiedPaths;
        }
        constexpr inline static std::string getOsName(){
            return Data::osName;
        }

        constexpr inline static std::string get(getType o) {
            switch (o) {
                case getType::home:
                    return Data::home;
                    break;
                case getType::temp:
                    return Data::temp;
                    break;
                case getType::version:
                    return Data::version;
                    break;
                case getType::resVersion:
                    return exec::getConfigObj().GetValue("more", "resVersion", "");
                    break;
                case getType::workDir:
                    return std::filesystem::current_path().string() | exec::unifiedPaths;
                    break;
                case getType::osName:
                    return Data::osName;
                    break;
                default:
                    return "unknown";
                    break;
            }
        }

        inline static void init() {

            const char *path = std::getenv(

#ifdef _WIN32
                "USERPROFILE"
#else
                "HOME"
#endif
            );

            if (path)
                Data::home = exec::unifiedThePaths<std::string>(path);

            if (std::string temp = exec::getConfigObj().GetValue("more", "temp", "");
                std::filesystem::is_directory(temp))
                Data::temp = temp | exec::unifiedPaths | exec::move;
            else
                Data::temp = (std::filesystem::temp_directory_path().string() + "Nekolc/") | exec::unifiedPaths;

            if (!std::filesystem::exists(Data::temp))
                std::filesystem::create_directory(Data::temp);
        };
    }; // class info

} // namespace neko