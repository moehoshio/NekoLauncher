#pragma once

#include <cstdint>
#include <string_view>

namespace neko {

    // Basic types should use all-lowercase style
    inline namespace types {
        // Character and string types
        using cstr = const char *;
        using strview = std::string_view;
        using uchar = unsigned char;

        // Not implemented
        // using str = neko::string;

        // Fixed-width integer types
        using uint64 = std::uint64_t;
        using uint32 = std::uint32_t;
        using uint16 = std::uint16_t;
        using uint8 = std::uint8_t;

        using int64 = std::int64_t;
        using int32 = std::int32_t;
        using int16 = std::int16_t;
        using int8 = std::int8_t;

        /**
         * @brief Synchronization mode for event processing
         * @details
         * - Sync: Synchronous mode, blocks until the event is processed
         * - Async: Asynchronous mode, does not block, event is processed in the background
         */
        enum class SyncMode {
            Sync = 0,
            Async = 1
        };

        enum class State {
            Completed,     // Operation finished successfully, no further action needed
            ActionNeeded,  // Action required from user or system
            RetryRequired, // Temporary failure, should retry later
        };

        enum class Priority : uint8 {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        };

        inline neko::cstr toString(Priority priority) {
            switch (priority) {
                case Priority::Low:
                    return "Low";
                case Priority::Normal:
                    return "Normal";
                case Priority::High:
                    return "High";
                case Priority::Critical:
                    return "Critical";
                default:
                    return "Unknown";
            }
        }
    } // namespace types

} // namespace neko
