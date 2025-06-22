#pragma once

#include <string>
#include <functional>

namespace neko::core {
    
    constexpr int windowsCommandLengthLimit = 8191; // Windows command line length limit

    // This function blocks the thread until the process ends.
    void launcherProcess(const std::string &command, std::function<void()> onStart, std::function<void(int)> onExit , const std::string &workingDir = "");
    // This function launches a new process and detaches it.
    void launcherNewProcess(const std::string &command,const std::string &workingDir = "");

} // namespace neko::core
