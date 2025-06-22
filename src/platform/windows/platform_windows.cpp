#include "neko/system/platform.hpp"

#include <sysinfoapi.h>

namespace neko::system {
    std::string getOsVersion() {
        OSVERSIONINFOEX info = {0};
        info.dwOSVersionInfoSize = sizeof(info);
        if (GetVersionEx((OSVERSIONINFO *)&info)) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%lu.%lu.%lu",
                     info.dwMajorVersion, info.dwMinorVersion, info.dwBuildNumber);
            return buf;
        }
        return "";
    }
} // namespace neko::system
