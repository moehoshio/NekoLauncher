#pragma once

#include <string>
#include <functional>

namespace neko::core {

    // This function blocks the thread until the process ends.
    void launcherProcess(const std::string &command, std::function<void()> onStart, std::function<void(int)> onExit);
    // This function launches a new process and detaches it.
    void launcherNewProcess(const std::string &command);

} // namespace neko::core
