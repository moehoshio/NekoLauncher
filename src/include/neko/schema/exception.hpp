/**
 * @file exception.hpp
 * @brief Exception classes for the NekoLauncher
 */
#pragma once

#include "neko/schema/types.hpp"

#include <exception>
#include <source_location>
#include <sstream>
#include <string>
#if defined(USE_BOOST_STACKTRACE) && __has_include("boost/stacktrace.hpp")
#include <boost/stacktrace.hpp>
#else
#undef USE_BOOST_STACKTRACE
#endif

namespace neko::ex {

    /**
     * @brief Stores extended exception information such as line, file, and function name.
     */
    struct ExceptionExtensionInfo {
        neko::uint32 line = 0;
        neko::cstr file = nullptr;
        neko::cstr funcName = nullptr;

        /**
         * @brief Constructs ErrorExtensionInfo with line, file, and function name.
         */
        constexpr ExceptionExtensionInfo(
            const std::source_location &loc = std::source_location::current()) noexcept
            : line(loc.line()), file(loc.file_name()), funcName(loc.function_name()) {}

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
    class Exception : public std::exception, public std::nested_exception {
    private:
        std::string msg;
        ExceptionExtensionInfo extInfo;
#if defined(USE_BOOST_STACKTRACE)
        boost::stacktrace::stacktrace trace;
#else
        struct stacktrace {
            bool empty() const { return true; }
        };
        stacktrace trace;
#endif

    public:
#if defined(USE_BOOST_STACKTRACE)
        /**
         * @brief Construct an Exception with a message and extension info.
         * @param Msg Error message.
         * @param ExtInfo Extended error information.
         */
        explicit Exception(const std::string &Msg, const ExceptionExtensionInfo &ExtInfo) noexcept
            : msg(Msg), extInfo(ExtInfo), trace(boost::stacktrace::stacktrace()) {}
        /**
         * @brief Construct an Exception with a message.
         * @param Msg Error message.
         */
        explicit Exception(const std::string &Msg) noexcept
            : msg(Msg), trace(boost::stacktrace::stacktrace()) {}
        /**
         * @brief Construct an Exception with a C-string message.
         * @param Msg Error message.
         */
        explicit Exception(neko::cstr Msg) noexcept
            : msg(Msg ? Msg : ""), trace(boost::stacktrace::stacktrace()) {}
#else
        explicit Exception(const std::string &Msg, const ExceptionExtensionInfo &ExtInfo) noexcept
            : msg(Msg), extInfo(ExtInfo) {}
        explicit Exception(const std::string &Msg) noexcept
            : msg(Msg) {}
        explicit Exception(neko::cstr Msg) noexcept
            : msg(Msg ? Msg : "") {}
#endif

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
        const ExceptionExtensionInfo &getExtensionInfo() const noexcept {
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

#if defined(USE_BOOST_STACKTRACE)
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
#endif
    };

    /**
     * @brief Exception for already existing objects.
     */
    struct AlreadyExists : public Exception {
        explicit AlreadyExists(const std::string &Msg = "Object already exists!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for invalid arguments.
     */
    struct InvalidArgument : public Exception {
        explicit InvalidArgument(const std::string &Msg = "Invalid argument!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for system errors.
     */
    class SystemError : public Exception {
    public:
        explicit SystemError(const std::string &Msg = "System error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for file-related errors.
     */
    class FileError : public SystemError {
    public:
        explicit FileError(const std::string &Msg = "File error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for network-related errors.
     */
    class NetworkError : public SystemError {
    public:
        explicit NetworkError(const std::string &Msg = "Network error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for database-related errors.
     */
    class DatabaseError : public SystemError {
    public:
        explicit DatabaseError(const std::string &Msg = "Database error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for external library errors.
     */
    class ExternalLibraryError : public SystemError {
    public:
        explicit ExternalLibraryError(const std::string &Msg = "External library error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : SystemError(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for out-of-range errors.
     */
    class OutOfRange : public Exception {
    public:
        explicit OutOfRange(const std::string &Msg = "Out of range!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for unimplemented features.
     */
    class NotImplemented : public Exception {
    public:
        explicit NotImplemented(const std::string &Msg = "Not implemented!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for configuration errors.
     */
    class Config : public Exception {
    public:
        explicit Config(const std::string &Msg = "Configuration error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for parsing errors.
     */
    class Parse : public Exception {
    public:
        explicit Parse(const std::string &Msg = "Parse error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for concurrency errors.
     */
    class Concurrency : public Exception {
    public:
        explicit Concurrency(const std::string &Msg = "Concurrency error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for assertion failures.
     */
    class Assertion : public Exception {
    public:
        explicit Assertion(const std::string &Msg = "Assertion failed!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for invalid operations.
     */
    class InvalidOperation : public Exception {
    public:
        explicit InvalidOperation(const std::string &Msg = "Invalid operation!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for permission denied errors.
     */
    class PermissionDenied : public Exception {
    public:
        explicit PermissionDenied(const std::string &Msg = "Permission denied!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for timeout errors.
     */
    class Timeout : public Exception {
    public:
        explicit Timeout(const std::string &Msg = "Timeout!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for logic errors.
     */
    class Logic : public Exception {
    public:
        explicit Logic(const std::string &Msg = "Logic error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

    /**
     * @brief Exception for runtime errors.
     */
    class Runtime : public Exception {
    public:
        explicit Runtime(const std::string &Msg = "Runtime error!", const ExceptionExtensionInfo &ExtInfo = {}) noexcept
            : Exception(Msg, ExtInfo) {}
    };

} // namespace neko::ex