/**
 * @file memoryinfo.hpp
 * @brief Header file defining structures and functions for system memory information.
 */

#pragma once

#include "neko/schema/types.hpp"

#include <optional>
#include <string>

namespace neko::system {

    /**
     * @brief Structure containing system memory information.
     */
    struct MemoryInfo {
        /**
         * @brief Total physical memory in bytes.
         */
        neko::uint64 totalBytes;

        /**
         * @brief Free physical memory in bytes.
         */
        neko::uint64 freeBytes;

        /**
         * @brief Convert MemoryInfo to string representation.
         * @return String describing total and free memory.
         */
        std::string toString() const;
    };

    /**
     * @brief Get system memory information.
     * @return MemoryInfo structure if successful, std::nullopt on failure.
     */
    std::optional<MemoryInfo> getSystemMemoryInfo();

} // namespace neko::system
