#include "neko/system/platform.hpp"

#include <sysinfoapi.h>

namespace neko::system {
    std::string getOsVersion() {
        OSVERSIONINFOEX info = {0};
        info.dwOSVersionInfoSize = sizeof(info);
        if (GetVersionEx((OSVERSIONINFO *)&info)) {
            std::vector<char> buf(64);
            snprintf(buf.data(), buf.size(), "%lu.%lu.%lu",
                     info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);
            return std::string(buf.data());
        }
        return "";
    }
} // namespace neko::system
