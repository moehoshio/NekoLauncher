#pragma once

#include <cstdint>

namespace neko {

    inline namespace types {
        // Character and string types
        using cstr = const char *;
        using uchar = unsigned char;

        // Not implemented
        // using str = neko::string;

        // Fixed-width integer types
        using uint64 = std::uint64_t;
        using uint32 = std::uint32_t;
        using uint16 = std::uint16_t;
        using uint8  = std::uint8_t;

        using int64 = std::int64_t;
        using int32 = std::int32_t;
        using int16 = std::int16_t;
        using int8  = std::int8_t;
    } // namespace types

} // namespace neko
