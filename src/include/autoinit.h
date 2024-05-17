#pragma once
#include "io.h"
#include "network.h"
#include <filesystem>
namespace neko {
    void setLog(int argc, char *argv[]);

    void setLogThreadName();

    void currentPathCorrection();

    inline void autoInit(int argc, char *argv[]) {

        if (exec::getConfigObj().LoadFile("config.ini") < 0)
            ; // If there is a callback, the user can be notified

        currentPathCorrection();
        info::init();
        setLog(argc, argv);
        setLogThreadName();
        networkBase::init().get();
    };
} // namespace neko
