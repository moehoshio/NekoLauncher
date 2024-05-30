#pragma once
#include "io.h"
#include "network.h"
#include <filesystem>
namespace neko {
    void setLog(int argc, char *argv[]);
    void setThreadNums();

    void setLogThreadName();

    void currentPathCorrection();

    inline void autoInit(int argc, char *argv[]) {

        currentPathCorrection();

        if (exec::getConfigObj().LoadFile("config.ini") < 0)
            ; // If there is a callback, the user can be notified

        info::init();
        setLog(argc, argv);
        setThreadNums();
        setLogThreadName();
        networkBase::init().get();
    };
} // namespace neko
