#include "neko/schema/types.hpp"
#include "neko/system/memoryinfo.hpp"

// MacOS specific includes
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <optional>

namespace neko::system {
    
    std::optional<MemoryInfo> getSystemMemoryInfo() {
        neko::uint64 memsize = 0;
        size_t len = sizeof(memsize);
        if (sysctlbyname("hw.memsize", &memsize, &len, nullptr, 0) != 0) {
            return std::nullopt;
        }

        mach_port_t hostPort = mach_host_self();
        vm_size_t pageSize = 0;
        vm_statistics64_data_t vmStats;
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

        if (host_page_size(hostPort, &pageSize) != KERN_SUCCESS ||
            host_statistics64(hostPort, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vmStats), &count) != KERN_SUCCESS) {
            return std::nullopt;
        }

        neko::uint64 freePages = vmStats.free_count + vmStats.inactive_count;
        neko::uint64 freeBytes = static_cast<neko::uint64>(freePages) * pageSize;

        return MemoryInfo{memsize, freeBytes};
    }

} // namespace neko::system