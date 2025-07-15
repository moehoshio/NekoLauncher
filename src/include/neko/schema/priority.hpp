#pragma once
#include "neko/schema/types.hpp"

namespace neko {

    inline namespace types {

        enum class Priority : neko::uint8 {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        };

        inline neko::cstr toString(Priority priority) {
            switch (priority) {
                case Priority::Low:      return "Low";
                case Priority::Normal:   return "Normal";
                case Priority::High:     return "High";
                case Priority::Critical: return "Critical";
                default:                 return "Unknown";
            }
        }

    } // namespace schema

} // namespace neko