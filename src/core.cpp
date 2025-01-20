#include "core.hpp"
#include <filesystem>

#if _WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace neko {

#if _WIN32
    void launchNewProcess(const std::string &command) {
        nlog::autoLog log{FI, LI, FN};
        nlog::Info(FI, LI, "%s : command : %s", FN, command.c_str());
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;

        if (!CreateProcessA(NULL, const_cast<char *>(command.c_str()), NULL, NULL, false, NULL, NULL, info::workPath().c_str(), &si, &pi)) {
            nlog::Err(FI, LI, "%s : Faild to Create process! cmd : %s , in dir : %s ", FN, command.c_str(), info::workPath().c_str());
        } else {
            nlog::Info(FI, LI, "%s : Create process okay , cmd : %s , in dir : %s", FN, command.c_str(), info::workPath().c_str());
        }
    }
#else
    void launchNewProcess(const std::string &command) {
        nlog::autoLog log{FI, LI, FN};
        nlog::Info(FI, LI, "%s : command : %s", FN, command.c_str());
        pid_t pid = fork();
        if (pid == 0) {
            execl("/bin/sh", "sh", "-c", command.c_str(), (char *)0);
            _exit(EXIT_FAILURE); // if execl return ,an the err
        } else if (pid < 0) {
            // fork err
        } else {
            _exit(EXIT_FAILURE);
        }
    }
#endif

#if _WIN32
    void launcherProcess(const std::string &command, launcherOpt opt, std::function<void(bool)> winFunc) {
        nlog::autoLog log{FI, LI, FN};
        nlog::Info(FI, LI, "%s : command : %s", FN, command.c_str());
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        switch (opt) {
            case launcherOpt::keep:
                if (!CreateProcessA(NULL, const_cast<char *>(command.c_str()), NULL, NULL, true, NULL, NULL, info::workPath().c_str(), &si, &pi)) {
                    nlog::Err(FI, LI, "%s : Faild to Create process! cmd : %s , in dir : %s ", FN, command.c_str(), info::workPath().c_str());
                } else {
                    nlog::Info(FI, LI, "%s : Create process okay , cmd : %s , in dir : %s", FN, command.c_str(), info::workPath().c_str());
                }
                break;
            case launcherOpt::endProcess:
                launchNewProcess(command);
                QApplication::quit();
                break;
            case launcherOpt::hideProcessAndOverReShow:
                winFunc(false);
                if (!CreateProcessA(NULL, const_cast<char *>(command.c_str()), NULL, NULL, true, NULL, NULL, info::workPath().c_str(), &si, &pi)) {
                    nlog::Err(FI, LI, "%s : Faild to Create process! cmd : %s , in dir : %s ", FN, command.c_str(), info::workPath().c_str());
                } else {
                    nlog::Info(FI, LI, "%s : Create process okay , cmd : %s , in dir : %s", FN, command.c_str(), info::workPath().c_str());
                }
                winFunc(true);
                break;
            default:
                break;
        }
    }
#else
void launcherProcess(const std::string &command, launcherOpt opt, std::function<void(bool)> winFunc) {
        nlog::autoLog log{FI, LI, FN};
        nlog::Info(FI, LI, "%s : command : %s", FN, command.c_str());
        switch (opt) {
            case launcherOpt::keep:
                std::system(command.c_str());
                break;
            case launcherOpt::endProcess:
                launchNewProcess(command);
                QApplication::quit();
                break;
            case launcherOpt::hideProcessAndOverReShow:
                winFunc(false);
                std::system(command.c_str());
                winFunc(true);
                break;
            default:
                break;
        }
    }
#endif

} // namespace neko
