#pragma once

#include <functional>
#include <string>

namespace neko::core {

    constexpr int windowsCommandLengthLimit = 8192 - 1; // Windows command line length limit

    struct ProcessInfo {
        /// @brief Command to execute
        std::string command;
        /// @brief Working directory for the process
        /// @note If empty, the current working directory will be used
        std::string workingDir = "";
        /// @brief Callback invoked when the process starts
        std::function<void()> onStart = nullptr;
        /// @brief Callback invoked when the process exits
        std::function<void(int)> onExit = nullptr;
        /**
         @brief Callback function to handle process output stream
         @note This callback function will be called whenever the process produces a line of output
         */
        std::function<void(const std::string &)> pipeStreamCb = nullptr;
    };

    /**
     * @brief Launches a process and waits for it to finish.
     * @param processInfo The information about the process to launch.
     * @throws ex::Runtime if the process fails to start. e.g the command is invalid , process is not found, etc.
     */
    void launcherProcess(const ProcessInfo &processInfo);

    /**
     * @brief Launches a new process and detaches it. (Even if the parent process ends, it will continue running)
     * @param command The command to execute.
     * @param workingDir The working directory for the process.
     * @throws ex::Runtime if the process fails to start. e.g the command is invalid , process is not found, etc.
     */
    void launcherNewProcess(const std::string &command, const std::string &workingDir = "");

} // namespace neko::core
