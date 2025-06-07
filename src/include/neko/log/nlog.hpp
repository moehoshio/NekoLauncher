/**
 * @file nlog.hpp
 * @brief Logging for the NekoLc project, providing convenient logging macros and RAII logging.
 */

#pragma once

#include "neko/schema/types.hpp"

#include "library/loguru.hpp"

#include <string>
#include <source_location>

// neko log
namespace nlog = neko::log;

namespace neko::log {

    /**
     * @brief Log an info message with formatting.
     * @param file Source file name.
     * @param line Source line number.
     * @param format Format string.
     * @param args Arguments for formatting.
     */
    constexpr inline void Info(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
        loguru::log(loguru::Verbosity_INFO, file, line, format, args...);
    }

    /**
     * @brief Log an error message with formatting.
     * @param file Source file name.
     * @param line Source line number.
     * @param format Format string.
     * @param args Arguments for formatting.
     */
    constexpr inline void Err(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
        loguru::log(loguru::Verbosity_ERROR, file, line, format, args...);
    }

    /**
     * @brief Log a warning message with formatting.
     * @param file Source file name.
     * @param line Source line number.
     * @param format Format string.
     * @param args Arguments for formatting.
     */
    constexpr inline void Warn(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
        loguru::log(loguru::Verbosity_WARNING, file, line, format, args...);
    }

    /**
     * @brief Log a debug message with formatting.
     * @param file Source file name.
     * @param line Source line number.
     * @param format Format string.
     * @param args Arguments for formatting.
     */
    constexpr inline void Debug(neko::cstr file, neko::uint32 line, neko::cstr format, const auto &...args) {
        loguru::log(loguru::Verbosity_9, file, line, format, args...);
    }

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
        autoLog(neko::cstr File = std::source_location::current().file_name(), neko::uint32 Line = std::source_location::current().line(), std::string Name = std::source_location::current().function_name(), std::string StartMsg = "Enter", std::string EndMsg = "End")
            : file(File), line(Line), name(Name), startMsg(StartMsg), endMsg(EndMsg) {
            Info(file, line, "%s : %s", this->name.c_str(), this->startMsg.c_str());
        }

        /**
         * @brief Destructor, logs exit message.
         */
        ~autoLog() {
            Info(file, line, "%s : %s", this->name.c_str(), this->endMsg.c_str());
        }
    };
} // namespace neko::log
