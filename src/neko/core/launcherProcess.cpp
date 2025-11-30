#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
#define _WIN32_WINNT 0x0601

#include <neko/log/nlog.hpp>
#include <neko/schema/exception.hpp>

#include "neko/core/launcherProcess.hpp"

#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/search_path.hpp>
#include <boost/process/v1/start_dir.hpp>

#ifdef _WIN32
#include <boost/process/v1/windows.hpp>
#endif

#include <functional>
#include <iostream>
#include <string>
#include <string_view>

namespace bp = boost::process::v1;

namespace neko::core {

    void launcherProcess(const ProcessInfo &processInfo) {

        try {

            bp::ipstream pipeStream;

#ifdef _WIN32
            bool usePowershell = processInfo.command.length() >= windowsCommandLengthLimit;
            std::string baseCommand = usePowershell ? "powershell" : "cmd";
            std::string commandPrefix = usePowershell ? "-Command" : "/c";
            
            bp::child proc = processInfo.workingDir.empty()
                ? bp::child(
                    bp::search_path(baseCommand),
                    commandPrefix,
                    processInfo.command,
                    bp::windows::hide,
                    bp::std_out > pipeStream)
                : bp::child(
                    bp::search_path(baseCommand),
                    commandPrefix,
                    processInfo.command,
                    bp::start_dir = processInfo.workingDir,
                    bp::windows::hide,
                    bp::std_out > pipeStream);
#else
            bp::child proc = processInfo.workingDir.empty()
                ? bp::child(
                    "/bin/sh",
                    "-c",
                    processInfo.command,
                    bp::std_out > pipeStream)
                : bp::child(
                    "/bin/sh",
                    "-c",
                    processInfo.command,
                    bp::start_dir = processInfo.workingDir,
                    bp::std_out > pipeStream);
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
            log::error("Launcher error: {} , code: {}", {} , e.what(), e.code().value());
            throw ex::Runtime("Failed to launch process : " + std::string(e.what()));
        }
    }

    void launcherNewProcess(const std::string &command, const std::string &workingDir) {
        try {
#ifdef _WIN32
            bool usePowershell = command.length() > windowsCommandLengthLimit;
            std::string baseCommand = usePowershell ? "powershell" : "cmd";
            std::string commandPrefix = usePowershell ? "-Command" : "/c";
            
            bp::child proc = workingDir.empty()
                ? bp::child(
                    bp::search_path(baseCommand),
                    commandPrefix,
                    command,
                    bp::windows::hide)
                : bp::child(
                    bp::search_path(baseCommand),
                    commandPrefix,
                    command,
                    bp::start_dir = workingDir,
                    bp::windows::hide);
#else
            bp::child proc = workingDir.empty()
                ? bp::child(
                    "/bin/sh",
                    "-c",
                    command)
                : bp::child(
                    "/bin/sh",
                    "-c",
                    command,
                    bp::start_dir = workingDir);
#endif
            proc.detach();
        } catch (const std::system_error &e) {
            log::error("Launcher error: {} , code: {}", {} , e.what(), e.code().value());
            throw ex::Runtime("Failed to launch process : " + std::string(e.what()));
        }
    }

} // namespace neko::core
