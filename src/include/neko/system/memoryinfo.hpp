#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <sstream>

namespace neko::system {

        struct MemoryInfo {
            uint64_t totalBytes; // total memory , bytes
            uint64_t freeBytes;  // free memory ，bytes
            std::string toString() const {
                std::ostringstream oss;
                oss << "Total: " << totalBytes / (1024 * 1024) << " MB, "
                    << "Free: " << freeBytes / (1024 * 1024) << " MB";
                return oss.str();
            }
        };

        // Get system memory information, return std::nullopt on failure
        std::optional<MemoryInfo> getSystemMemoryInfo();

} // namespace neko::system
