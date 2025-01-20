#pragma once
#include <exception>
// neko error
namespace nerr {

#define nerrImpLoggerMode true

#if nerrImpLoggerMode
#include "log.hpp"
#endif

    using uint = unsigned int;

    struct error : public std::exception {

        inline static bool enableLogger = false;

        const char *msg;
        const char *fileName;
        uint line;
        const char *funcName;

        error(const char *Msg, const char *fileName, uint line, const char *funcName, bool logger = enableLogger) noexcept : msg(Msg), fileName(fileName), line(line), funcName(funcName) {
            #if nerrImpLoggerMode
                if (logger)
                    nlog::Err(fileName, line, "%s : %s", funcName, msg);
            #endif
        };
        error(const char *Msg, bool logger = enableLogger) noexcept : msg(Msg) {
            #if nerrImpLoggerMode
                if (logger)
                    nlog::Err(fileName, line, "%s : %s", funcName, msg);
            #endif
        };
        
        inline const char *what() const noexcept { return msg; };
    };

    struct TheSame : public error {
        TheSame(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        TheSame(const char *Msg = "The same thing already exists!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct TimeOut : public error {
        TimeOut(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        TimeOut(const char *Msg = "Operation timed out!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct FileRead : public error {
        FileRead(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        FileRead(const char *Msg = "File read error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct FileOpen : public error {
        FileOpen(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        FileOpen(const char *Msg = "File open error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct FileWrite : public error {
        FileWrite(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        FileWrite(const char *Msg = "File write error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct FileNotFound : public error {
        FileNotFound(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        FileNotFound(const char *Msg = "File not found!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct NetworkConnection : public error {
        NetworkConnection(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        NetworkConnection(const char *Msg = "Network connection error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct NetworkTimeout : public error {
        NetworkTimeout(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        NetworkTimeout(const char *Msg = "Network timeout!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct NetworkProtocol : public error {
        NetworkProtocol(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        NetworkProtocol(const char *Msg = "Network protocol error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct DatabaseConnection : public error {
        DatabaseConnection(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        DatabaseConnection(const char *Msg = "Database connection error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct DatabaseQuery : public error {
        DatabaseQuery(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        DatabaseQuery(const char *Msg = "Database query error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct DatabaseWrite : public error {
        DatabaseWrite(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        DatabaseWrite(const char *Msg = "Database write error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct AuthFailure : public error {
        AuthFailure(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        AuthFailure(const char *Msg = "Authentication failure!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct PermissionDenied : public error {
        PermissionDenied(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        PermissionDenied(const char *Msg = "Permission denied!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct HardwareFailure : public error {
        HardwareFailure(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        HardwareFailure(const char *Msg = "Hardware failure!",bool logger = enableLogger) : error(Msg,logger) {};
    };

    struct ExternalLibrary : public error {
        ExternalLibrary(const char *Msg, const char *fileName, uint line, const char *funcName , bool logger = error::enableLogger) noexcept : error(Msg, fileName, line, funcName,logger) {};
        ExternalLibrary(const char *Msg = "External library error!",bool logger = enableLogger) : error(Msg,logger) {};
    };

} // namespace nerr