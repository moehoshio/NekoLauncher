#include "neko/system/platform.hpp"

#include <sys/utsname.h>

namespace neko::system {
    std::string get_system_version() {
        struct utsname uts;
        if (uname(&uts) == 0) {
            return uts.release;
        }
        return "";
    }
} // namespace neko::system
