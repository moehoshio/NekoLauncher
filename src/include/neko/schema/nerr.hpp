/**
 * @file nerr.hpp
 * @brief Error handling classes for the NekoLauncher
 */
#pragma once

#include "neko/schema/nekodefine.hpp"
#include "neko/schema/types.hpp"

#include <exception>

#if defined(nerrImpLoggerModeDefine) && __has_include("neko/log/nlog.hpp")
#include "neko/log/nlog.hpp"
#endif

namespace nerr = neko::err;

namespace neko::err {

    /**
     * @brief Base error class extending std::exception
     * 
     * Provides basic error handling functionality for all derived error types
     */
    struct Error : public std::exception {

        /**
         * @brief Global default value for whether to enable error logging
         */
        inline static bool enableLogger = false;

        std::string msg;
        neko::cstr fileName;
        neko::uint32 line;
        neko::cstr funcName;

        /**
         * @brief Constructor with detailed error information
         * 
         * @param Msg Error message
         * @param fileName Source file name
         * @param line Line number
         * @param funcName Function name
         * @param logger Whether to log this error
         */
        Error(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = enableLogger) noexcept : msg(Msg), fileName(fileName), line(line), funcName(funcName) {
#if defined(nerrImpLoggerModeDefine)
            if (logger)
                nlog::Err(fileName, line, "%s : %s", funcName, msg.c_str());
#endif
        };
        
        /**
         * @brief Simplified constructor with only an error message
         * 
         * @param Msg Error message
         * @param logger Whether to log this error
         */
        Error(std::string Msg, bool logger = enableLogger) noexcept : msg(Msg) {
#if defined(nerrImpLoggerModeDefine)
            if (logger)
                nlog::Err(fileName, line, "%s : %s", funcName, msg.c_str());
#endif
        };

        /**
         * @brief Get the error message
         * 
         * @return C-style string containing the error message
         */
        inline neko::cstr what() const noexcept { return msg.c_str(); };
    };

    /**
     * @brief Error indicating that an object already exists
     */
    struct TheSame : public Error {
        TheSame(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        TheSame(std::string Msg = "The same thing already exists!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that an operation timed out
     */
    struct TimeOut : public Error {
        TimeOut(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        TimeOut(std::string Msg = "Operation timed out!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that an invalid argument was provided
     */
    struct InvalidArgument : public Error {
        InvalidArgument(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        InvalidArgument(std::string Msg = "Invalid argument!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that an object is in an invalid state
     */
    struct InvalidState : public Error {
        InvalidState(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        InvalidState(std::string Msg = "Invalid state!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that an operation is invalid in the current context
     */
    struct InvalidOperation : public Error {
        InvalidOperation(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        InvalidOperation(std::string Msg = "Invalid operation!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that a type is invalid
     */
    struct InvalidType : public Error {
        InvalidType(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        InvalidType(std::string Msg = "Invalid type!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that a value is invalid
     */
    struct InvalidValue : public Error {
        InvalidValue(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        InvalidValue(std::string Msg = "Invalid value!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a file reading problem
     */
    struct FileRead : public Error {
        FileRead(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        FileRead(std::string Msg = "File read error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a file opening problem
     */
    struct FileOpen : public Error {
        FileOpen(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        FileOpen(std::string Msg = "File open error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a file writing problem
     */
    struct FileWrite : public Error {
        FileWrite(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        FileWrite(std::string Msg = "File write error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that a file was not found
     */
    struct FileNotFound : public Error {
        FileNotFound(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        FileNotFound(std::string Msg = "File not found!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a network connection problem
     */
    struct NetworkConnection : public Error {
        NetworkConnection(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        NetworkConnection(std::string Msg = "Network connection error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a network timeout
     */
    struct NetworkTimeout : public Error {
        NetworkTimeout(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        NetworkTimeout(std::string Msg = "Network timeout!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a network protocol problem
     */
    struct NetworkProtocol : public Error {
        NetworkProtocol(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        NetworkProtocol(std::string Msg = "Network protocol error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a database connection problem
     */
    struct DatabaseConnection : public Error {
        DatabaseConnection(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        DatabaseConnection(std::string Msg = "Database connection error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a database query problem
     */
    struct DatabaseQuery : public Error {
        DatabaseQuery(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        DatabaseQuery(std::string Msg = "Database query error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a database write problem
     */
    struct DatabaseWrite : public Error {
        DatabaseWrite(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        DatabaseWrite(std::string Msg = "Database write error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating an authentication failure
     */
    struct AuthFailure : public Error {
        AuthFailure(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        AuthFailure(std::string Msg = "Authentication failure!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating that an operation was denied due to insufficient permissions
     */
    struct PermissionDenied : public Error {
        PermissionDenied(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        PermissionDenied(std::string Msg = "Permission denied!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a hardware failure
     */
    struct HardwareFailure : public Error {
        HardwareFailure(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        HardwareFailure(std::string Msg = "Hardware failure!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

    /**
     * @brief Error indicating a problem with an external library
     */
    struct ExternalLibrary : public Error {
        ExternalLibrary(std::string Msg, neko::cstr fileName, neko::uint32 line, neko::cstr funcName, bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName, logger) {};
        ExternalLibrary(std::string Msg = "External library error!", bool logger = enableLogger) : Error(Msg, logger) {};
    };

} // namespace neko::err