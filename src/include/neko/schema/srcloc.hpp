#pragma once

#include "neko/schema/types.hpp"

#include <source_location>

namespace neko {

    inline namespace types {
        using SrcLoc = std::source_location;
    }

    /**
     * @brief Source location information
     */
    struct SrcLocInfo {
        neko::uint32 line = 0;
        neko::cstr file = nullptr;
        neko::cstr funcName = nullptr;

        constexpr SrcLocInfo(
            const SrcLoc &loc = SrcLoc::current()) noexcept
            : line(loc.line()), file(loc.file_name()), funcName(loc.function_name()) {}
        constexpr SrcLocInfo(neko::uint32 line, neko::cstr file, neko::cstr funcName) noexcept
            : line(line), file(file), funcName(funcName) {}

        constexpr neko::uint32 getLine() const noexcept { return line; }
        constexpr neko::cstr getFile() const noexcept { return file; }
        constexpr neko::cstr getFuncName() const noexcept { return funcName; }
        constexpr bool hasInfo() const noexcept {
            return (line != 0 && file != nullptr) || funcName != nullptr;
        }
    };

} // namespace neko