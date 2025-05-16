#pragma once

#include <string>
#include <functional>

namespace neko {

    // This function blocks the thread until the process ends.
    void launcherProcess(const std::string &command, std::function<void()> on_start, std::function<void(int)> on_exit);

} // namespace neko
