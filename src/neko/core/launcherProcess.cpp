#include "neko/core/launcherProcess.hpp"
#include "neko/log/nlog.hpp"
#include "neko/schema/nekodefine.hpp"

#include <boost/process.hpp>

#ifdef _WIN32
#include <boost/process/v1/windows.hpp>
#endif

#include <functional>
#include <iostream>
#include <string>
#include <string_view>

namespace neko::core {

    // This function blocks the thread until the process ends.
    void launcherProcess(const std::string &command, std::function<void()> onStart, std::function<void(int)> onExit, const std::string &workingDir) {

        try {

#ifdef _WIN32
            bool usePowershell = command.length() > windowsCommandLengthLimit;
            std::string baseCommand = usePowershell ? "powershell" : "cmd";
            std::string commandPrefix = usePowershell ? "-Command" : "/c";
            boost::process::child proc(
                boost::process::search_path(baseCommand),
                commandPrefix,
                command,
                boost::process::start_dir = workingDir,
                boost::process::windows::hide);
#else
            boost::process::child proc("/bin/sh", "-c", command, boost::process::start_dir = workingDir);
#endif

            if (onStart) {
                onStart();
            }

            proc.wait();
            int code = proc.exit_code();

            if (onExit) {
                onExit(code);
            }

        } catch (const std::exception &e) {
            log::Err(log::SrcLoc::current() ,"Launcher error: %s", e.what());
            if (onExit) {
                onExit(-1);
            }
        }
    }

    void launcherNewProcess(const std::string &command, const std::string &workingDir) {
        try {
#ifdef _WIN32
            bool usePowershell = command.length() > windowsCommandLengthLimit;
            std::string baseCommand = usePowershell ? "powershell" : "cmd";
            std::string commandPrefix = usePowershell ? "-Command" : "/c";
            boost::process::child proc(
                boost::process::search_path(baseCommand),
                commandPrefix,
                command,
                boost::process::start_dir = workingDir
                    boost::process::windows::hide);
#else
            boost::process::child proc("/bin/sh", "-c", command, boost::process::start_dir = workingDir);
#endif
            proc.detach();
        } catch (const std::exception &e) {
            log::Err(log::SrcLoc::current() ,"Launcher error: %s", e.what());
        }
    }

} // namespace neko::core
