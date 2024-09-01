#pragma once
#include "loguru.hpp"
// neko log
namespace nlog {
    struct autoLog {
        const char *file;
        unsigned int line;
        std::string name;
        std::string startMsg;
        std::string endMsg;
        autoLog(const char *file, unsigned int line, std::string name, std::string startMsg = "Enter ,up", std::string endMsg = "End , down") : file(file), line(line), name(name), startMsg(startMsg), endMsg(endMsg) {
            Info(file, line, "%s : %s", this->name.c_str(), this->startMsg.c_str());
        }
        ~autoLog() {
            Info(file, line, "%s : %s", this->name.c_str(), this->endMsg.c_str());
        }
    };

    // Use formatting strings, for example: ("main.cpp",1,"%s : %s , v%d","func" ,"hello", 1,1)
    // Out the :  ... "func : hello , v1.1"
    constexpr inline void Info(const char *file, unsigned int line, const char *format, const auto &...args) {
        loguru::log(loguru::Verbosity_INFO, file, line, format, args...);
    }
    constexpr inline void Err(const char *file, unsigned int line, const char *format, const auto &...args) {
        loguru::log(loguru::Verbosity_ERROR, file, line, format, args...);
    }
    constexpr inline void Warn(const char *file, unsigned int line, const char *format, const auto &...args) {
        loguru::log(loguru::Verbosity_WARNING, file, line, format, args...);
    }

} // namespace nlog
