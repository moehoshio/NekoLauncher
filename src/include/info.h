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
            resVersion
        };
    private:
        struct Data {
            static std::string home;
            constexpr static const char *version =
#include "../data/version"
                ;
            static std::string temp;
            static std::string resVersion;
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
        inline static std::string getResVersion(){
            return Data::resVersion;
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
            case getType::resVersion :
                return Data::resVersion;
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

            if (std::string temp = exec::getConfigObj().GetValue("more", "temp", "");
                std::filesystem::is_directory(temp)
            )
                Data::temp = temp | exec::move;
            else
                Data::temp = std::filesystem::temp_directory_path().string();

            Data::resVersion = exec::getConfigObj().GetValue("more","resVersion","");
            if (Data::resVersion.empty())            
                nlog::Err(FI,LI,"%s : resVersion is empty !",FN);
            
        };
    }; //class info

} // namespace neko