#pragma once
#include "nekodefine.hpp"

#include <exception>

// neko error
namespace nerr {

#if defined(nerrImpLoggerModeDefine)
#include "nlog.hpp"
#endif

    struct Error : public std::exception {

        inline static bool enableLogger = false;

        const char *msg;
        const char *fileName;
        uint line;
        const char *funcName;

        Error(const char *Msg, const char *fileName, uint line, const char *funcName, bool logger = enableLogger) noexcept : msg(Msg), fileName(fileName), line(line), funcName(funcName) {
            #if defined(nerrImpLoggerModeDefine)
                if (logger)
                    nlog::Err(fileName, line, "%s : %s", funcName, msg);
            #endif
        };
        Error(const char *Msg, bool logger = enableLogger) noexcept : msg(Msg) {
            #if defined(nerrImpLoggerModeDefine)
                if (logger)
                    nlog::Err(fileName, line, "%s : %s", funcName, msg);
            #endif
        };
        
        inline const char *what() const noexcept { return msg; };
    };

    struct TheSame : public Error {
        TheSame(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        TheSame(const char *Msg = "The same thing already exists!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct TimeOut : public Error {
        TimeOut(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        TimeOut(const char *Msg = "Operation timed out!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileRead : public Error {
        FileRead(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileRead(const char *Msg = "File read error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileOpen : public Error {
        FileOpen(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileOpen(const char *Msg = "File open error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileWrite : public Error {
        FileWrite(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileWrite(const char *Msg = "File write error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct FileNotFound : public Error {
        FileNotFound(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        FileNotFound(const char *Msg = "File not found!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct NetworkConnection : public Error {
        NetworkConnection(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        NetworkConnection(const char *Msg = "Network connection error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct NetworkTimeout : public Error {
        NetworkTimeout(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        NetworkTimeout(const char *Msg = "Network timeout!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct NetworkProtocol : public Error {
        NetworkProtocol(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        NetworkProtocol(const char *Msg = "Network protocol error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct DatabaseConnection : public Error {
        DatabaseConnection(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        DatabaseConnection(const char *Msg = "Database connection error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct DatabaseQuery : public Error {
        DatabaseQuery(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        DatabaseQuery(const char *Msg = "Database query error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct DatabaseWrite : public Error {
        DatabaseWrite(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        DatabaseWrite(const char *Msg = "Database write error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct AuthFailure : public Error {
        AuthFailure(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        AuthFailure(const char *Msg = "Authentication failure!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct PermissionDenied : public Error {
        PermissionDenied(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        PermissionDenied(const char *Msg = "Permission denied!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct HardwareFailure : public Error {
        HardwareFailure(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        HardwareFailure(const char *Msg = "Hardware failure!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

    struct ExternalLibrary : public Error {
        ExternalLibrary(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = Error::enableLogger) noexcept : Error(Msg, fileName, line, funcName,logger) {};
        ExternalLibrary(const char *Msg = "External library error!",bool logger = enableLogger) : Error(Msg,logger) {};
    };

} // namespace nerr