/**
 * @file nerr.hpp
 * @brief Error handling classes for the NekoLauncher
 */
#pragma once

#include "neko/schema/nekodefine.hpp"
#include "neko/schema/types.hpp"

#include <exception>
#include <nested_exception>
#include <string>

#include <boost/stacktrace.hpp>


#if defined(__has_builtin)
#if __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE) && __has_builtin(__builtin_FUNCTION)
#define NEKO_HAS_BUILTIN_LOCATION 1
#endif
#endif

#if !defined(NEKO_HAS_BUILTIN_LOCATION)
#if defined(_MSC_VER) && _MSC_VER >= 1929 // MSVC 2019 v16.10+
#define NEKO_HAS_BUILTIN_LOCATION 1
#endif
#endif

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
#include <source_location>
#define NEKO_HAS_STD_SOURCE_LOCATION 1
#endif

static_assert(NEKO_HAS_BUILTIN_LOCATION || NEKO_HAS_STD_SOURCE_LOCATION, "Error handling requires either __builtin_* or std::source_location(c++20) support.");

namespace nerr = neko::err;

namespace neko::err {

    /**
     * @brief Stores extended error information such as line, file, and function name.
     */
    struct ErrorExtensionInfo {
        neko::uint32 line = 0;
        neko::cstr file = nullptr;
        neko::cstr funcName = nullptr;

#if defined(NEKO_HAS_BUILTIN_LOCATION)
        /**
         * @brief Constructs ErrorExtensionInfo with line, file, and function name.
         * @note Uses __builtin_LINE(), __builtin_FILE(), and __builtin_FUNCTION() as default values.
         * @note Note: These are not part of the standard library, but are available in newer compilers.
         * @note Supported compilers: GCC, Clang, MSVC (VS2019 v16.10+)
         */
        constexpr ErrorExtensionInfo(neko::uint32 Line = __builtin_LINE(), neko::cstr File = __builtin_FILE(), neko::cstr FuncName = __builtin_FUNCTION()) noexcept
            : line(Line), file(File), funcName(FuncName) {}
#else
        constexpr ErrorExtensionInfo(
            const std::source_location &loc = std::source_location::current()) noexcept
            : line(loc.line()), file(loc.file_name()), funcName(loc.function_name()) {}
#endif
        constexpr neko::uint32 getline() const noexcept { return line; }
        constexpr neko::cstr getFile() const noexcept { return file; }
        constexpr neko::cstr getFuncName() const noexcept { return funcName; }
        constexpr bool hasInfo() const noexcept {
            return (line != 0 && file != nullptr) || funcName != nullptr;
        }
    };

    /**
     * @brief Base error class extending std::exception and std::nested_exception.
     *
     * Provides basic error handling functionality for all derived error types.
     * Stores error message, extension info, and stack trace.
     */
    class Error : public std::exception, public std::nested_exception {
    private:
        std::string msg;
        ErrorExtensionInfo extInfo;
        boost::stacktrace::stacktrace trace;

    public:
        /**
         * @brief Construct an Error with a message and extension info.
         * @param Msg Error message.
         * @param ExtInfo Extended error information.
         */
        explicit Error(const std::string &Msg, const ErrorExtensionInfo &ExtInfo) noexcept
            : msg(Msg), extInfo(ExtInfo),
              trace(boost::stacktrace::stacktrace()) {};

        /**
         * @brief Construct an Error with a message.
         * @param Msg Error message.
         */
        explicit Error(const std::string &Msg) noexcept
            : msg(Msg),
              trace(boost::stacktrace::stacktrace()) {};

        /**
         * @brief Construct an Error with a C-string message.
         * @param Msg Error message.
         */
        explicit Error(neko::cstr Msg) noexcept
            : msg(Msg ? Msg : ""),
              trace(boost::stacktrace::stacktrace()) {};

        /**
         * @brief Get the error message.
         * @return Error message as a C-string.
         */
        neko::cstr what() const noexcept override {
            return msg.c_str();
        }

        /**
         * @brief Check if extra error info is available.
         * @return True if extension info is present.
         */
        bool hasExtraInfo() const noexcept {
            return extInfo.hasInfo();
        }

        /**
         * @brief Check if a stack trace is available.
         * @return True if stack trace is not empty.
         */
        bool hasStackTrace() const noexcept {
            return !trace.empty();
        }

        /**
         * @brief Get the extended error information.
         * @return Reference to ErrorExtensionInfo.
         */
        const ErrorExtensionInfo &getExtensionInfo() const noexcept {
            return extInfo;
        }

        /**
         * @brief Get the line number where the error occurred.
         * @return Line number.
         */
        neko::uint32 getLine() const noexcept {
            return extInfo.getline();
        }
        /**
         * @brief Get the file name where the error occurred.
         * @return File name as a C-string.
         */
        neko::cstr getFile() const noexcept {
            return extInfo.getFile();
        }
        /**
         * @brief Get the function name where the error occurred.
         * @return Function name as a C-string.
         */
        neko::cstr getFuncName() const noexcept {
            return extInfo.getFuncName();
        }
        /**
         * @brief Get the error message as a string.
         * @return Error message.
         */
        const std::string &getMessage() const noexcept {
            return msg;
        }
        /**
         * @brief Get a formatted stack trace as a string
         * @param format Optional format string to customize output
         * Supports placeholders:
         * - {index}: Frame index (0-based)
         * - {name}: Function name
         * - {source_file}: Source file name
         * - {source_line}: Source line number
         * @return Formatted stack trace string
         * @note If format is empty, returns the raw stack trace as a string.
         * If no stack trace is available, returns a default message.
         * @note The format string is not validated, so ensure it contains valid placeholders.
         */
        std::string getStackTraceStr(std::string_view format = "") const {
            if (trace.empty()) {
                return "No stack trace available.";
            }

            std::ostringstream oss;

            if (format.empty()) {
                oss << trace;
                return oss.str();
            }

            for (std::size_t i = 0; i < trace.size(); ++i) {
                const auto &frame = trace[i];
                std::string line(format);
                auto replace = [&](const std::string &key, const std::string &value) {
                    size_t pos = 0;
                    while ((pos = line.find(key, pos)) != std::string::npos) {
                        line.replace(pos, key.length(), value);
                        pos += value.length();
                    }
                };
                replace("{index}", std::to_string(i));
                replace("{name}", frame.name());
                replace("{source_file}", frame.source_file());
                replace("{source_line}", std::to_string(frame.source_line()));
                oss << line << '\n';
            }
            return oss.str();
        }

        /**
         * @brief Get the stack trace object.
         * @return Reference to boost::stacktrace::stacktrace.
         */
        const boost::stacktrace::stacktrace &getStackTrace() const noexcept { return trace; }
    };

    /**
     * @brief Exception for already existing objects.
     */
    struct AlreadyExists : public Error {
        explicit AlreadyExists(const std::string &Msg = "Object already exists!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for invalid arguments.
     */
    struct InvalidArgument : public Error {
        explicit InvalidArgument(const std::string &Msg = "Invalid argument!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for system errors.
     */
    class SystemError : public Error {
    public:
        explicit SystemError(const std::string &Msg = "System error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for file-related errors.
     */
    class FileError : public SystemError {
    public:
        explicit FileError(const std::string &Msg = "File error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for network-related errors.
     */
    class NetworkError : public SystemError {
    public:
        explicit NetworkError(const std::string &Msg = "Network error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for database-related errors.
     */
    class DatabaseError : public SystemError {
    public:
        explicit DatabaseError(const std::string &Msg = "Database error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for external library errors.
     */
    class ExternalLibraryError : public SystemError {
    public:
        explicit ExternalLibraryError(const std::string &Msg = "External library error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for out-of-memory errors.
     */
    class OutOfMemoryError : public Error {
    public:
        explicit OutOfMemoryError(const std::string &Msg = "Out of memory!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for unimplemented features.
     */
    class NotImplementedError : public Error {
    public:
        explicit NotImplementedError(const std::string &Msg = "Not implemented!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for configuration errors.
     */
    class ConfigError : public Error {
    public:
        explicit ConfigError(const std::string &Msg = "Configuration error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for parsing errors.
     */
    class ParseError : public Error {
    public:
        explicit ParseError(const std::string &Msg = "Parse error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for concurrency errors.
     */
    class ConcurrencyError : public Error {
    public:
        explicit ConcurrencyError(const std::string &Msg = "Concurrency error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for assertion failures.
     */
    class AssertionError : public Error {
    public:
        explicit AssertionError(const std::string &Msg = "Assertion failed!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for invalid operations.
     */
    class InvalidOperation : public Error {
    public:
        explicit InvalidOperation(const std::string &Msg = "Invalid operation!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for permission denied errors.
     */
    class PermissionDeniedError : public Error {
    public:
        explicit PermissionDeniedError(const std::string &Msg = "Permission denied!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for timeout errors.
     */
    class TimeoutError : public Error {
    public:
        explicit TimeoutError(const std::string &Msg = "Timeout!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for logic errors.
     */
    class LogicError : public Error {
    public:
        explicit LogicError(const std::string &Msg = "Logic error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for runtime errors.
     */
    class RuntimeError : public Error {
    public:
        explicit RuntimeError(const std::string &Msg = "Runtime error!", const ErrorExtensionInfo &ExtInfo = {}) noexcept
            : Error(Msg, ExtInfo) {}
    };

} // namespace neko::err