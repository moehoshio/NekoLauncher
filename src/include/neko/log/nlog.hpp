/**
 * @file nlog.hpp
 * @brief Logging for the NekoLc project, providing convenient logging macros and RAII logging.
 */

#pragma once

#include "neko/schema/types.hpp"

#include "library/loguru.hpp"

#include <source_location>
#include <string>

namespace neko::log {

    using SrcLoc = std::source_location;

    /**
     * @brief Log a message with INFO level, manually specifying file, line, and format parameters.
     *
     * This version is useful for multi-layer wrappers in some modules, allowing the original call site
     * to be passed through for accurate logging.
     *
     * @param file   The source file name where the log is generated.
     * @param line   The line number in the source file.
     * @param format The format string for the log message.
     * @param args   Additional arguments for the format string.
     */
    inline namespace explicit_location {

        inline void Info(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
            loguru::log(loguru::Verbosity_INFO, file, line, format, args...);
        }
        inline void Err(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
            loguru::log(loguru::Verbosity_ERROR, file, line, format, args...);
        }
        inline void Warn(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
            loguru::log(loguru::Verbosity_WARNING, file, line, format, args...);
        }
        inline void Debug(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
            loguru::log(loguru::Verbosity_9, file, line, format, args...);
        }
    } // namespace explicit_location

    // Intermediate namespace: allows specifying format, but auto-fills file/line/function via std::source_location.
    inline namespace auto_location {

        inline void Info(SrcLoc loc, neko::cstr format, const auto &...args) {
            std::string formatWithFunc = std::string("%s : ") + format;
            loguru::log(loguru::Verbosity_INFO, loc.file_name(), loc.line(), formatWithFunc.c_str(), loc.function_name(), args...);
        }
        inline void Err(SrcLoc loc, neko::cstr format, const auto &...args) {
            std::string formatWithFunc = std::string("%s : ") + format;
            loguru::log(loguru::Verbosity_ERROR, loc.file_name(), loc.line(), formatWithFunc.c_str(), loc.function_name(), args...);
        }
        inline void Warn(SrcLoc loc, neko::cstr format, const auto &...args) {
            std::string formatWithFunc = std::string("%s : ") + format;
            loguru::log(loguru::Verbosity_WARNING, loc.file_name(), loc.line(), formatWithFunc.c_str(), loc.function_name(), args...);
        }
        inline void Debug(SrcLoc loc, neko::cstr format, const auto &...args) {
            std::string formatWithFunc = std::string("%s : ") + format;
            loguru::log(loguru::Verbosity_9, loc.file_name(), loc.line(), formatWithFunc.c_str(), loc.function_name(), args...);
        }
    } // namespace auto_location

    // only one message
    inline namespace simple {

        inline void Info(neko::cstr msg, SrcLoc loc = SrcLoc::current()) {
            loguru::log(loguru::Verbosity_INFO, loc.file_name(), loc.line(), "%s : %s", loc.function_name(), msg);
        }
        inline void Err(neko::cstr msg, SrcLoc loc = SrcLoc::current()) {
            loguru::log(loguru::Verbosity_ERROR, loc.file_name(), loc.line(), "%s : %s", loc.function_name(), msg);
        }
        inline void Warn(neko::cstr msg, SrcLoc loc = SrcLoc::current()) {
            loguru::log(loguru::Verbosity_WARNING, loc.file_name(), loc.line(), "%s : %s", loc.function_name(), msg);
        }
        inline void Debug(neko::cstr msg, SrcLoc loc = SrcLoc::current()) {
            loguru::log(loguru::Verbosity_9, loc.file_name(), loc.line(), "%s : %s", loc.function_name(), msg);
        }
    } // namespace simple

    /**
     * @brief RAII logger that logs entry and exit of a scope.
     */
    struct autoLog {
        /**
         * @brief Source file name.
         */
        neko::cstr file;
        /**
         * @brief Source line number.
         */
        neko::uint32 line;
        /**
         * @brief Name of the scope or function.
         */
        std::string name;
        /**
         * @brief Message to log on entry.
         */
        std::string startMsg;
        /**
         * @brief Message to log on exit.
         */
        std::string endMsg;

        /**
         * @brief Construct an autoLog object and log entry message.
         * @param file Source file name.
         * @param line Source line number.
         * @param name Name of the scope or function.
         * @param startMsg Entry message (default: "Enter").
         * @param endMsg Exit message (default: "End").
         */
        autoLog(std::string StartMsg = "Enter", std::string EndMsg = "End", SrcLoc loc = SrcLoc::current())
            : file(loc.file_name()), line(loc.line()), name(loc.function_name()), startMsg(StartMsg), endMsg(EndMsg) {
            Info(file, line, "%s : %s", name.c_str(), startMsg.c_str());
        }

        /**
         * @brief Destructor, logs exit message.
         */
        ~autoLog() {
            Info(file, line, "%s : %s", name.c_str(), endMsg.c_str());
        }
    };
} // namespace neko::log