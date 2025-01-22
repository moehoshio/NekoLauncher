#pragma once
#include <exception>
// neko error
namespace nerr {

    enum class errType {
        None,

        // one.h
        TheSame,
        // timeout
        TimeOut,
        // input
        InputInvalidFormat,
        InputMissingData,
        InputOverflow,
        InputOutOfRange,
        // file
        FileRead,
        FileOpen,
        FileWrite,
        FileNotFound,
        // network
        NetworkConnection,
        NetworkTimeout,
        NetworkProtocol,

        // memory
        MemoryAllocation,

        // database
        DatabaseConnection,
        DatabaseQuery,
        DatabaseWrite,

        // authentication
        AuthFailure,
        // permissions
        PermissionDenied,
        // hardware
        HardwareFailure,

        // external libraries
        Library,

        UnknownError = 255
    };
    // For a list of errCode and detailed messages, please refer to: dev.md
    using errCode = int;
    struct Error : public std::exception {
        errCode code;
        const char *msg;
        errType type;
        Error() noexcept = default;
        Error(errCode Code, const char *Msg, errType Type) noexcept : code(Code), msg(Msg), type(Type){};
        Error(errCode Code, const char *Msg) noexcept : code(Code), msg(Msg){};
        Error(const char *Msg) noexcept : msg(Msg){};
        ~Error() noexcept = default;
        inline const char *what() const noexcept{ return msg; };
    };

} // namespace nerr