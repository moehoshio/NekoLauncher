#pragma once
#include "loguru.hpp"
//neko log
namespace nlog
{
    // Use formatting strings, for example: ("main.cpp",1,"%s : %s , v%d","func" ,"hello", 1,1)
    // Out the :  ... "func : hello , v1.1"
    constexpr inline void Info(const char * file , unsigned int line ,const char * format,const auto&... args){
        loguru::log(loguru::Verbosity_INFO,file,line,format,args...);
    }
    constexpr inline void Err(const char * file , unsigned int line ,const char * format,const auto&... args){
        loguru::log(loguru::Verbosity_ERROR,file,line,format,args...);
    }
    constexpr inline void Warn(const char * file , unsigned int line ,const char * format,const auto&... args){
        loguru::log(loguru::Verbosity_WARNING,file,line,format,args...);
    }

} // namespace neko log
