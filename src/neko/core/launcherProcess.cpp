#include "neko/core/launcherProcess.hpp"
#include "neko/schema/exception.hpp"
#include "neko/schema/nekodefine.hpp"

#include "neko/log/nlog.hpp"

#include <boost/process.hpp>

#ifdef _WIN32
#include <boost/process/v1/windows.hpp>
#endif

#include <functional>
#include <iostream>
#include <string>
#include <string_view>

namespace neko::core {

    void launcherProcess(const ProcessInfo &processInfo) {

        try {

            boost::process::ipstream pipeStream;

#ifdef _WIN32
            bool usePowershell = processInfo.command.length() >= windowsCommandLengthLimit;
            std::string baseCommand = usePowershell ? "powershell" : "cmd";
            std::string commandPrefix = usePowershell ? "-Command" : "/c";
            boost::process::child proc(
                boost::process::search_path(baseCommand),
                commandPrefix,
                processInfo.command,
                boost::process::start_dir = processInfo.workingDir,
                boost::process::windows::hide,
                boost::process::std_out > pipeStream);
#else
            boost::process::child proc(
                "/bin/sh",
                "-c",
                processInfo.command,
                boost::process::start_dir = processInfo.workingDir,
                boost::process::std_out > pipeStream);
#endif

            if (processInfo.onStart) {
                processInfo.onStart();
            }
            if (processInfo.pipeStreamCb) {
                std::string line;
                while (std::getline(pipeStream, line)) {
                    processInfo.pipeStreamCb(line);
                }
            }

            proc.wait();
            int code = proc.exit_code();

            if (processInfo.onExit) {
                processInfo.onExit(code);
            }

        } catch (const std::system_error &e) {
            log::error({}, "Launcher error: {} , code: {}", e.what(), e.code().value());
            throw ex::Runtime("Failed to launch process : " + std::string(e.what()));
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
                boost::process::start_dir = workingDir,
                boost::process::windows::hide);
#else
            boost::process::child proc(
                "/bin/sh",
                "-c",
                command,
                boost::process::start_dir = workingDir);
#endif
            proc.detach();
        } catch (const std::system_error &e) {
            log::error({}, "Launcher error: {} , code: {}", e.what(), e.code().value());
            throw ex::Runtime("Failed to launch process : " + std::string(e.what()));
        }
    }

} // namespace neko::core
