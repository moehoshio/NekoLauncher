#include "neko/schema/types.hpp"
#include "neko/system/memoryinfo.hpp"

#include <optional>

// unix specific includes
#include <sys/sysinfo.h>

namespace neko::system {
    std::optional<MemoryInfo> getSystemMemoryInfo() {
        struct ::sysinfo sys_info;
        if (::sysinfo(&sys_info) == 0) {
            neko::uint64 total = sys_info.totalram;
            neko::uint64 free = sys_info.freeram;
#ifdef __linux__
            total *= sys_info.mem_unit;
            free *= sys_info.mem_unit;
#endif
            return MemoryInfo{total, free};
        }
        return std::nullopt;
    }

} // namespace neko::system