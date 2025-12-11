#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#endif // _WIN32


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
#include <filesystem>
#include <fstream>
#include <optional>

namespace bp = boost::process::v1;

namespace neko::core {

    void launcherProcess(const ProcessInfo &processInfo) {

        try {

            bp::ipstream pipeStream;
            // Platform-specific helpers declared here so cleanup logic below can compile everywhere.
            std::optional<std::filesystem::path> tempScript;
            std::optional<std::ofstream> childLog;

#ifdef _WIN32
            // Use cmd for typical commands; if too long for the Windows limit, write to a temp .cmd file to avoid PowerShell parsing issues.
            std::string cmdToRun = processInfo.command;
            if (processInfo.command.length() >= windowsCommandLengthLimit) {
                auto tmpDir = std::filesystem::temp_directory_path();
                auto scriptPath = tmpDir / ("nekolauncher-" + std::to_string(::GetCurrentProcessId()) + "-" + std::to_string(::GetTickCount64()) + ".cmd");
                std::ofstream ofs(scriptPath, std::ios::out | std::ios::trunc);
                if (!ofs.is_open()) {
                    throw ex::Runtime("Failed to create temp launch script: " + scriptPath.string());
                }
                ofs << "@echo off\r\n" << processInfo.command << "\r\n";
                ofs.close();
                tempScript = scriptPath;
                cmdToRun = "\"" + scriptPath.string() + "\""; // protect spaces
            }

            // Mirror child output to a temp log file to capture full Java stack traces.
            std::filesystem::path childLogPath = std::filesystem::temp_directory_path() / "nekolauncher-child.log";
            childLog.emplace(childLogPath, std::ios::out | std::ios::trunc);
            if (!childLog->is_open()) {
                childLog.reset();
            } else {
                log::info("Child output will also be written to: {}", {} , childLogPath.string());
            }

            bp::child proc = processInfo.workingDir.empty()
                ? bp::child(
                    bp::search_path("cmd"),
                    "/c",
                    cmdToRun,
                    bp::windows::hide,
                    bp::std_out > pipeStream,
                    bp::std_err > pipeStream)
                : bp::child(
                    bp::search_path("cmd"),
                    "/c",
                    cmdToRun,
                    bp::start_dir = processInfo.workingDir,
                    bp::windows::hide,
                    bp::std_out > pipeStream,
                    bp::std_err > pipeStream);
#else
            bp::child proc = processInfo.workingDir.empty()
                ? bp::child(
                    "/bin/sh",
                    "-c",
                    processInfo.command,
                    bp::std_out > pipeStream,
                    bp::std_err > pipeStream)
                : bp::child(
                    "/bin/sh",
                    "-c",
                    processInfo.command,
                    bp::start_dir = processInfo.workingDir,
                    bp::std_out > pipeStream,
                    bp::std_err > pipeStream);
#endif

            if (processInfo.onStart) {
                processInfo.onStart();
            }
            // Always drain the pipe to avoid missing errors; log when no callback provided.
            std::string line;
            while (std::getline(pipeStream, line)) {
                if (childLog) {
                    (*childLog) << line << std::endl;
                }
                if (processInfo.pipeStreamCb) {
                    processInfo.pipeStreamCb(line);
                } else {
                    log::info("Launcher output: {}", {} , line);
                }
            }

            proc.wait();
            int code = proc.exit_code();
            // Cleanup temp script if created
            if (tempScript.has_value()) {
                std::error_code ec;
                std::filesystem::remove(*tempScript, ec);
                if (ec) {
                    log::warn("Failed to remove temp launch script: {} , ec: {}", {} , tempScript->string(), ec.value());
                }
            }
            if (childLog && childLog->is_open()) {
                childLog->flush();
            }
            log::info("Launcher exit code: {}", {} , code);

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
            const bool usePowershell = command.length() > windowsCommandLengthLimit;
            const std::string baseCommand = usePowershell ? "powershell" : "cmd";
            const std::string commandPrefix = usePowershell ? "-Command" : "/c";

            // Suppress extra windows and ensure the command is executed via shell
            const std::string payload = usePowershell ? ("& " + command) : command;

            bp::child proc = workingDir.empty()
                ? bp::child(
                    bp::search_path(baseCommand),
                    commandPrefix,
                    payload,
                    bp::windows::create_no_window)
                : bp::child(
                    bp::search_path(baseCommand),
                    commandPrefix,
                    payload,
                    bp::start_dir = workingDir,
                    bp::windows::create_no_window);
#else
            bp::child proc = workingDir.empty()
                ? bp::child("/bin/sh", "-c", command)
                : bp::child("/bin/sh", "-c", command, bp::start_dir = workingDir);
#endif
            proc.detach();
        } catch (const std::system_error &e) {
            log::error("Launcher error: {} , code: {}", {} , e.what(), e.code().value());
            throw ex::Runtime("Failed to launch process : " + std::string(e.what()));
        }
    }

} // namespace neko::core
