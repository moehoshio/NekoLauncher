/**
 * @file exception.hpp
 * @brief Exception classes for the NekoLauncher
 */
#pragma once

#include "neko/schema/types.hpp"
#include "neko/schema/srcloc.hpp"

#include <exception>
#include <source_location>
#include <sstream>
#include <string>

/**
 * @brief Exception classes for the NekoLauncher
 * @namespace neko::ex
 */
namespace neko::ex {

    /**
     * @brief Base error class extending std::exception and std::nested_exception.
     *
     * Provides basic error handling functionality for all derived error types.
     * Stores error message and extension info
     */
    class Exception : public std::exception, public std::nested_exception {
    private:
        std::string msg;
        neko::SrcLocInfo srcLoc;

    public:
        /**
         * @brief Construct an Exception with a message.
         * @param Msg Error message.
         * @param SrcLoc Source location information.
         */
        explicit Exception(const std::string &Msg, const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : msg(Msg), srcLoc(SrcLoc) {}
        /**
         * @brief Construct an Exception with a C-string message.
         * @param Msg Error message.
         * @param SrcLoc Source location information.
         */
        explicit Exception(neko::cstr Msg, const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : msg(Msg ? Msg : ""), srcLoc(SrcLoc) {}

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
            return srcLoc.hasInfo();
        }

        /**
         * @brief Get the extended error information.
         * @return Reference to ErrorExtensionInfo.
         */
        const neko::SrcLocInfo &getSourceLocation() const noexcept {
            return srcLoc;
        }

        /**
         * @brief Get the line number where the error occurred.
         * @return Line number.
         */
        neko::uint32 getLine() const noexcept {
            return srcLoc.getLine();
        }
        /**
         * @brief Get the file name where the error occurred.
         * @return File name as a C-string.
         */
        neko::cstr getFile() const noexcept {
            return srcLoc.getFile();
        }
        /**
         * @brief Get the function name where the error occurred.
         * @return Function name as a C-string.
         */
        neko::cstr getFuncName() const noexcept {
            return srcLoc.getFuncName();
        }
        /**
         * @brief Get the error message as a string.
         * @return Error message.
         */
        const std::string &getMessage() const noexcept {
            return msg;
        }
    };

    /**
     * @brief Exception for program termination or exit.
     */
    class ProgramExit : public Exception {
    public:
        explicit ProgramExit(const std::string &Msg = "Program exited!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for already existing objects.
     */
    class AlreadyExists : public Exception {
    public:
        explicit AlreadyExists(const std::string &Msg = "Object already exists!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for invalid arguments.
     */
    class InvalidArgument : public Exception {
    public:
        explicit InvalidArgument(const std::string &Msg = "Invalid argument!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for system errors.
     */
    class SystemError : public Exception {
    public:
        explicit SystemError(const std::string &Msg = "System error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for file-related errors.
     */
    class FileError : public SystemError {
    public:
        explicit FileError(const std::string &Msg = "File error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : SystemError(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for network-related errors.
     */
    class NetworkError : public SystemError {
    public:
        explicit NetworkError(const std::string &Msg = "Network error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : SystemError(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for database-related errors.
     */
    class DatabaseError : public SystemError {
    public:
        explicit DatabaseError(const std::string &Msg = "Database error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : SystemError(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for external library errors.
     */
    class ExternalLibraryError : public SystemError {
    public:
        explicit ExternalLibraryError(const std::string &Msg = "External library error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : SystemError(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for out-of-range errors.
     */
    class OutOfRange : public Exception {
    public:
        explicit OutOfRange(const std::string &Msg = "Out of range!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for unimplemented features.
     */
    class NotImplemented : public Exception {
    public:
        explicit NotImplemented(const std::string &Msg = "Not implemented!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for configuration errors.
     */
    class Config : public Exception {
    public:
        explicit Config(const std::string &Msg = "Configuration error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for parsing errors.
     */
    class Parse : public Exception {
    public:
        explicit Parse(const std::string &Msg = "Parse error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for concurrency errors.
     */
    class Concurrency : public Exception {
    public:
        explicit Concurrency(const std::string &Msg = "Concurrency error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for task rejection.
     */
    class TaskRejected : public Exception {
    public:
        explicit TaskRejected(const std::string &Msg = "Task rejected!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for assertion failures.
     */
    class Assertion : public Exception {
    public:
        explicit Assertion(const std::string &Msg = "Assertion failed!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for invalid operations.
     */
    class InvalidOperation : public Exception {
    public:
        explicit InvalidOperation(const std::string &Msg = "Invalid operation!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for permission denied errors.
     */
    class PermissionDenied : public Exception {
    public:
        explicit PermissionDenied(const std::string &Msg = "Permission denied!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for timeout errors.
     */
    class Timeout : public Exception {
    public:
        explicit Timeout(const std::string &Msg = "Timeout!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for logic errors.
     */
    class Logic : public Exception {
    public:
        explicit Logic(const std::string &Msg = "Logic error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

    /**
     * @brief Exception for runtime errors.
     */
    class Runtime : public Exception {
    public:
        explicit Runtime(const std::string &Msg = "Runtime error!", const neko::SrcLocInfo &SrcLoc = {}) noexcept
            : Exception(Msg, SrcLoc) {}
    };

} // namespace neko::ex