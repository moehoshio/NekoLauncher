#pragma once
#include "io.h"
#include "cconfig.h"
#include "network.h"
#include <filesystem>
namespace neko {
    void setLog(int argc, char *argv[]);
    void setThreadNums();

    void setLogThreadName();

    void currentPathCorrection();

    void configInfoPrint(Config config);
    

    inline auto autoInit(int argc, char *argv[]) {

        currentPathCorrection();

        if (exec::getConfigObj().LoadFile("config.ini") < 0)
            oneIof o("loadBad.txt"); // If there is a callback, the user can be notified
        
        setLog(argc, argv);
        setThreadNums();
        setLogThreadName();
        configInfoPrint(exec::getConfigObj());
        info::language(exec::getConfigObj().GetValue("main","language","en"));
        return networkBase::init();
        
    };
} // namespace neko
