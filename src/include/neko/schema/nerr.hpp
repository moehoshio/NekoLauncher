#pragma once

#include "neko/schema/nekodefine.hpp"

#include <exception>

// neko error
namespace nerr {

#if defined(nerrImpLoggerModeDefine) && __has_include("neko/log/nlog.hpp")
#include "neko/log/nlog.hpp"
#else
#define nerrImpLoggerModeDefine false
#endif

    struct Error : public std::exception {

        inline static bool enableLogger = false;

        std::string msg;
        const char *fileName;
        uint line;
        const char *funcName;

        Error(std::string Msg, const char *fileName, uint line, const char *funcName, bool logger = enableLogger) noexcept : msg(Msg), fileName(fileName), line(line), funcName(funcName) {
            #if defined(nerrImpLoggerModeDefine) && nerrImpLoggerModeDefine != false
                if (logger)
                    nlog::Err(fileName, line, "%s : %s", funcName, msg.c_str());
            #endif
        };
        Error(std::string Msg, bool logger = enableLogger) noexcept : msg(Msg) {
            #if defined(nerrImpLoggerModeDefine) && nerrImpLoggerModeDefine != false
                if (logger)
                    nlog::Err(fileName, line, "%s : %s", funcName, msg.c_str());
            #endif
        };
        
        inline const char *what() const noexcept { return msg.c_str(); };
    };

    struct TheSame : public Error {
        TheSame(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        TheSame(std::string Msg = "The same thing already exists!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct TimeOut : public Error {
        TimeOut(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        TimeOut(std::string Msg = "Operation timed out!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct InvalidArgument : public Error {
        InvalidArgument(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        InvalidArgument(std::string Msg = "Invalid argument!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct InvalidState : public Error {
        InvalidState(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        InvalidState(std::string Msg = "Invalid state!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct InvalidOperation : public Error {
        InvalidOperation(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        InvalidOperation(std::string Msg = "Invalid operation!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct InvalidType : public Error {
        InvalidType(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        InvalidType(std::string Msg = "Invalid type!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct InvalidValue : public Error {
        InvalidValue(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        InvalidValue(std::string Msg = "Invalid value!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileRead : public Error {
        FileRead(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileRead(std::string Msg = "File read error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileOpen : public Error {
        FileOpen(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileOpen(std::string Msg = "File open error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileWrite : public Error {
        FileWrite(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileWrite(std::string Msg = "File write error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileNotFound : public Error {
        FileNotFound(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileNotFound(std::string Msg = "File not found!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct NetworkConnection : public Error {
        NetworkConnection(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        NetworkConnection(std::string Msg = "Network connection error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct NetworkTimeout : public Error {
        NetworkTimeout(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        NetworkTimeout(std::string Msg = "Network timeout!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct NetworkProtocol : public Error {
        NetworkProtocol(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        NetworkProtocol(std::string Msg = "Network protocol error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct DatabaseConnection : public Error {
        DatabaseConnection(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        DatabaseConnection(std::string Msg = "Database connection error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct DatabaseQuery : public Error {
        DatabaseQuery(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        DatabaseQuery(std::string Msg = "Database query error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct DatabaseWrite : public Error {
        DatabaseWrite(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        DatabaseWrite(std::string Msg = "Database write error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct AuthFailure : public Error {
        AuthFailure(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        AuthFailure(std::string Msg = "Authentication failure!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct PermissionDenied : public Error {
        PermissionDenied(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        PermissionDenied(std::string Msg = "Permission denied!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct HardwareFailure : public Error {
        HardwareFailure(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        HardwareFailure(std::string Msg = "Hardware failure!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct ExternalLibrary : public Error {
        ExternalLibrary(std::string Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        ExternalLibrary(std::string Msg = "External library error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

} // namespace nerr