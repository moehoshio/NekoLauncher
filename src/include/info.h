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
            version
        };
    private:
        struct Data {
            static std::string home;
            constexpr static const char *version =
#include "../data/version"
                ;
            static std::string temp;
        };


    public:
        inline static void setTemp(const char * tmp){
            if (std::filesystem::is_directory(tmp))
                Data::temp = tmp;
        }
        constexpr inline static std::string getVersion() {
            return Data::version;
        };
        inline static std::string getTemp(){
            return Data::temp;
        }
        inline static std::string getHome(){
            return Data::home;
        }
        
        constexpr inline static std::string get(getType o){
            switch (o)
            {
            case getType::home :
                return Data::home;
                break;
            case getType::temp :
                return Data::temp;
                break;
            case getType::version :
                return Data::version;
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
                Data::home = std::string(path);

            if (std::string t = exec::getConfigObj().GetValue("more", "temp", "");
                std::filesystem::is_directory(t)
            )
                Data::temp = t | exec::move;
            else
                Data::temp = std::filesystem::temp_directory_path().string();
        };
    }; //class info

} // namespace neko