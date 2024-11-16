#pragma once
#include "cconfig.h"
#include "io.h"
#include "network.h"
#include <filesystem>
namespace neko {

    inline void setLog(int argc, char *argv[], Config cfg) {
        bool
            dev = cfg.dev.enable,
            debug = cfg.dev.debug;
        if (!dev)
            return;

        // loguru::g_stderr_verbosity = loguru::Verbosity_OFF; // Avoid output to console

        if (!debug) {
            std::string file_name = exec::sum<std::string>("logs/", exec::getTimeString(), ".log");
            try {
                oneIof file(file_name, file_name, std::ios::out);
            } catch (...) {
            }
            // If there is a callback, the user can be notified
            (void)loguru::add_file(file_name.c_str(), loguru::Append, loguru::Verbosity_WARNING);

            return;
        }

        loguru::init(argc, argv);
        const char *file_name[]{"logs/debug.log", "logs/new-debug.log"};

        for (auto path : file_name) {
            if (!std::filesystem::exists(std::filesystem::path(path))) {
                try {
                    oneIof file(path, path, std::ios::out);
                } catch (...) {
                }
            }
            loguru::FileMode mode = (path == std::string_view("logs/new-debug.log")) ? loguru::Truncate : loguru::Append;
            // If there is a callback, the user can be notified
            (void)loguru::add_file(path, mode, loguru::Verbosity_6);
        }
    }

    inline void setLogThreadName() {
        size_t nums = exec::getThreadObj().get_thread_nums();
        nlog::autoLog log{FI, LI, FN, "threadNums : " + std::to_string(nums)};

        for (size_t i = 0; i < nums; ++i) {
            exec::getThreadObj().enqueue(
                [i_str = std::to_string(i + 1)]() {
                    loguru::set_thread_name((std::string("thread ") + i_str).c_str());
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    nlog::Info(FI, LI, "%s : Hello thread %s", FN, i_str.c_str());
                });
        }
    }

    inline void setThreadNums(Config cfg) {
        if (cfg.net.thread > 0)
            exec::getThreadObj().set_pool_size(static_cast<size_t>(cfg.net.thread));
        nlog::Info(FI, LI, "%s : End. expect thread nums : %d ", FN, cfg.net.thread);
    }

    inline void configInfoPrint(Config config) {
        nlog::Info(FI, LI, "%s : config main : lang : %s , bgType : %s , bg : %s , windowSize : %s , launcherMode : %d ,  useSysWinodwFrame: %s , barKeepRight : %s ", FN, config.main.lang, config.main.bgType, config.main.bg, config.main.windowSize, config.main.launcherMode, exec::boolTo<const char *>(config.main.useSysWindowFrame), exec::boolTo<const char *>(config.main.barKeepRight));
        nlog::Info(FI, LI, "%s : config net : thread : %d , proxy : %s", FN, config.net.thread, config.net.proxy);
        nlog::Info(FI, LI, "%s : config style : blurHint : %d , blurValue : %d , fontPointSize : %d , fontFamilies : %s ", FN, config.style.blurHint, config.style.blurValue, config.style.fontPointSize, config.style.fontFamilies);
        nlog::Info(FI, LI, "%s : config dev : enable : %s , debug : %s , server : %s , tls : %s ", FN, exec::boolTo<const char *>(config.dev.enable), exec::boolTo<const char *>(config.dev.debug), config.dev.server, exec::boolTo<const char *>(config.dev.tls));
        nlog::Info(FI, LI, "%s : config manage : account : %s , name : %s , uuid : %s , authlibPrefetched : %s ", FN, config.manage.account, config.manage.displayName, config.manage.uuid, config.manage.authlibPrefetched);
        nlog::Info(FI, LI, "%s : config more : temp : %s , resVersion : %s", FN, config.more.temp, config.more.resVersion);
    }

    inline void currentPathCorrection() {

#if defined(__APPLE__) // if using the macOS package need ?
        const char *mPath[]{".app/Contents/MacOS", ".app\\Contents\\MacOS"};
        for (auto it : mPath) {
            if (std::filesystem::current_path().string().find(it) != std::string::npos) {
                std::filesystem::current_path(std::filesystem::current_path().parent_path().parent_path().parent_path());
                break;
            }
        }
#endif
    }

    inline auto autoInit(int argc, char *argv[]) {

        currentPathCorrection();

        if (exec::getConfigObj().LoadFile("config.ini") < 0)
            oneIof o("loadBad.txt"); // If there is a callback, the user can be notified

        Config cfg(exec::getConfigObj());

        setLog(argc, argv, cfg);
        setThreadNums(cfg);
        setLogThreadName();
        configInfoPrint(cfg);
        info::language(cfg.main.lang);
        return networkBase::init();
    };
} // namespace neko
