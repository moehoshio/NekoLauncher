#include "neko/system/memoryinfo.hpp"

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#elif defined(__linux__) || defined(__unix__)
    #include <sys/sysinfo.h>
#elif defined(__APPLE__)
    #include <mach/mach.h>
    #include <sys/sysctl.h>
    #include <sys/types.h>
#endif

namespace neko {
    namespace system {

        std::optional<MemoryInfo> getSystemMemoryInfo() {
#ifdef _WIN32
            MEMORYSTATUSEX statex;
            statex.dwLength = sizeof(statex);
            if (GlobalMemoryStatusEx(&statex)) {
                return MemoryInfo{
                    statex.ullTotalPhys,
                    statex.ullAvailPhys};
            }
            return std::nullopt;
#elif defined(__linux__) || defined(__unix__)
            struct ::sysinfo sys_info;
            if (::sysinfo(&sys_info) == 0) {
                uint64_t total = sys_info.totalram;
                uint64_t free = sys_info.freeram;
#ifdef __linux__
                total *= sys_info.mem_unit;
                free *= sys_info.mem_unit;
#endif
                return MemoryInfo{total, free};
            }
            return std::nullopt;
#elif defined(__APPLE__)
            uint64_t memsize = 0;
            size_t len = sizeof(memsize);
            if (sysctlbyname("hw.memsize", &memsize, &len, nullptr, 0) != 0) {
                return std::nullopt;
            }

            mach_port_t hostPort = mach_host_self();
            vm_size_t pageSize = 0;
            vm_statistics64_data_t vmStats;
            mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
            if (host_page_size(hostPort, &pageSize) != KERN_SUCCESS) {
                return std::nullopt;
            }
            if (host_statistics64(hostPort, HOST_VM_INFO64, (host_info64_t)&vmStats, &count) != KERN_SUCCESS) {
                return std::nullopt;
            }

            uint64_t freePages = vmStats.free_count + vmStats.inactive_count;
            uint64_t freeBytes = static_cast<uint64_t>(freePages) * pageSize;

            return MemoryInfo{memsize, freeBytes};
#else
            return std::nullopt;
#endif
        }

    } // namespace system
} // namespace neko