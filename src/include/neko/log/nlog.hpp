#pragma once

#include "library/loguru.hpp"
#include <string>

// neko log
namespace nlog {
    

    // Use formatting strings, for example: ("main.cpp",1,"%s : %s , v%d","func" ,"hello", 1,1)
    // Out the :  main.cpp line 1 : "func : hello , v1.1"
    constexpr inline void Info(const char *file, unsigned int line, const char *format, const auto &...args) {
        loguru::log(loguru::Verbosity_INFO, file, line, format, args...);
    }
    constexpr inline void Err(const char *file, unsigned int line, const char *format, const auto &...args) {
        loguru::log(loguru::Verbosity_ERROR, file, line, format, args...);
    }
    constexpr inline void Warn(const char *file, unsigned int line, const char *format, const auto &...args) {
        loguru::log(loguru::Verbosity_WARNING, file, line, format, args...);
    }

    struct autoLog {
        const char *file;
        unsigned int line;
        std::string name;
        std::string startMsg;
        std::string endMsg;
        autoLog(const char *file, unsigned int line, std::string name, std::string startMsg = "Enter", std::string endMsg = "End") : file(file), line(line), name(name), startMsg(startMsg), endMsg(endMsg) {
            Info(file, line, "%s : %s", this->name.c_str(), this->startMsg.c_str());
        }
        ~autoLog() {
            Info(file, line, "%s : %s", this->name.c_str(), this->endMsg.c_str());
        }
    };
    
} // namespace nlog
