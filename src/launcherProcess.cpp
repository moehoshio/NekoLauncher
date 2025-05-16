#include "neko/core/launcherProcess.hpp"
#include "neko/schema/nekodefine.hpp"
#include "neko/log/nlog.hpp"

#include <boost/process.hpp>

#include <string>
#include <string_view>
#include <iostream>
#include <functional>

namespace neko {

    // This function blocks the thread until the process ends.
    void launcherProcess(const std::string &command, std::function<void()> on_start, std::function<void(int)> on_exit) {
        try {

#ifdef _WIN32
            boost::process::child proc(boost::process::search_path("cmd"), "/c", command);
#else
            boost::process::child proc("/bin/sh", "-c", command);
#endif

            if (!proc.valid()) {
                throw std::runtime_error("Failed to launch process");
            }

            if (on_start) {
                on_start();
            }

            proc.wait();
            int code = proc.exit_code();

            if (on_exit) {
                on_exit(code);
            }

        } catch (const std::exception &e) {
            nlog::Err(FI, LI, "%s : Launcher error: %s", FN, e.what());
            on_exit(-1);
        }
    }

} // namespace neko
