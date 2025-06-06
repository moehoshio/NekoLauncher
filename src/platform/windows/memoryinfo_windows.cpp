#include "neko/system/memoryinfo.hpp"

#include <optional>

// Windows specific includes
#define NOMINMAX
#include <windows.h>

namespace neko::system {

    std::optional<MemoryInfo> getSystemMemoryInfo() {

        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        if (GlobalMemoryStatusEx(&statex)) {
            return MemoryInfo{
                statex.ullTotalPhys,
                statex.ullAvailPhys};
        }
        return std::nullopt;
    }

} // namespace neko::system