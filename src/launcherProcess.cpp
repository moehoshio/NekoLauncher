#include "neko/log/nlog.hpp"
#include "neko/core/launcherProcess.hpp"
#include "neko/schema/nekodefine.hpp"

#include <boost/process.hpp>

#ifdef _WIN32
#include <boost/process/windows.hpp>
#endif

#include <iostream>
#include <functional>
#include <string>
#include <string_view>

namespace neko {

    // This function blocks the thread until the process ends.
    void launcherProcess(const std::string &command, std::function<void()> onStart, std::function<void(int)> onExit) {
        try {

#ifdef _WIN32
            bool usePowershell = command.length() > 8191; // Windows command line length limit
            std::string baseCommand = usePowershell ? "powershell" : "cmd";
            std::string commandPrefix = usePowershell ? "-Command" : "/c";
            boost::process::child proc(
                boost::process::search_path(baseCommand),
                commandPrefix,
                command,
                boost::process::windows::hide
            );
#else
            boost::process::child proc("/bin/sh", "-c", command);
#endif

            if (!proc.valid()) {
                throw std::runtime_error("Failed to launch process");
            }

            if (onStart) {
                onStart();
            }

            proc.wait();
            int code = proc.exit_code();

            if (onExit) {
                onExit(code);
            }

        } catch (const std::exception &e) {
            nlog::Err(FI, LI, "%s : Launcher error: %s", FN, e.what());
            onExit(-1);
        }
    }

    void launcherNewProcess(const std::string &command) {
        try {
#ifdef _WIN32
            bool usePowershell = command.length() > 8191; // Windows command line length limit
            std::string baseCommand = usePowershell ? "powershell" : "cmd";
            std::string commandPrefix = usePowershell ? "-Command" : "/c";
            boost::process::child proc(
                boost::process::search_path(baseCommand),
                commandPrefix,
                command,
                boost::process::windows::hide
            );
#else
            boost::process::child proc("/bin/sh", "-c", command);
#endif
            if (!proc.valid()) {
                throw std::runtime_error("Failed to launch process");
            }
            proc.detach();
        } catch (const std::exception &e) {
            nlog::Err(FI, LI, "%s : Launcher error: %s", FN, e.what());
        }
    }

} // namespace neko
