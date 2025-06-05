#pragma once

#include <cstdint>

namespace neko {

    inline namespace types {
        using cstr = const char *;

        using uint64 = std::uint64_t;
        using uint32 = std::uint32_t;
        using uint16 = std::uint16_t;

        using int64 = std::int64_t;
        using int32 = std::int32_t;
        using int16 = std::int16_t;
    } // namespace types

} // namespace neko
