#include "autoinit.h"
namespace neko {

    void setLog(int argc, char *argv[]) {
        bool
            dev = exec::getConfigObj().GetBoolValue("dev", "enable", false),
            debug = exec::getConfigObj().GetBoolValue("dev", "debug", false);
        if (!dev)
            return;

        loguru::g_stderr_verbosity = loguru::Verbosity_OFF; // Avoid output to console

        if (!debug) {
            std::string file_name = exec::sum(std::string("logs/"), exec::getTimeString(), ".log");
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
            loguru::FileMode mode = (path == std::string("logs/new-debug.log")) ? loguru::Truncate : loguru::Append;
            // If there is a callback, the user can be notified
            (void)loguru::add_file(path, mode, loguru::Verbosity_6);
        }
    }

    void setLogThreadName() {

        size_t nums = exec::getThreadObj().get_thread_nums();
        nlog::Info(FI, LI, "%s : Enter ,threadNums : %zu", FN, nums);
        for (size_t i = 1; i <= nums; ++i) {
            exec::getThreadObj().enqueue(
                [i_str = std::to_string(i)]() {
                    loguru::set_thread_name(
                        (std::string("thread ") + i_str).c_str());
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    LOG_F(INFO, "%s : Hello", FN);
                });
        }
        nlog::Info(FI, LI, "%s : End ", FN);
    }

    void setThreadNums() {
        long threadNums = exec::getConfigObj().GetLongValue("dev", "thread", 0);
        if (threadNums > 0)
            exec::getThreadObj().set_pool_size(static_cast<size_t>(threadNums));
        nlog::Info(FI,LI,"%s : End. expect thread nums : %d ",FN,threadNums);
    }
    void configInfoPrint(Config config){
        nlog::Info(FI,LI,"%s : config main : bg : %s , windowSize : %s , useSysWinodwFrame: %s , barKeepRight : %s ",FN,config.main.bg,config.main.windowSize,exec::boolTo<const char *>(config.main.useSysWindowFrame),exec::boolTo<const char *>(config.main.barKeepRight));
        nlog::Info(FI,LI,"%s : config net : thread : %d , proxy : %s",FN,config.net.thread,config.net.proxy);
        nlog::Info(FI,LI,"%s : config dev : enable : %s , debug : %s , server : %s , tls : %s ",FN,exec::boolTo<const char *>(config.dev.enable),exec::boolTo<const char *>(config.dev.debug),config.dev.server,exec::boolTo<const char *>(config.dev.tls));
        nlog::Info(FI,LI,"%s : config dev : temp : %s ",FN,config.more.temp);

    }
    void currentPathCorrection() {

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
} // namespace neko
