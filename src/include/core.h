#pragma once

#include "nlohmann/json.hpp"

#include "network.h"

namespace neko {
    class core {
    public:
        enum class State {
            over,
            undone,
            tryAgainLater
        };
    inline State autoUpdate(){
        network net;
        auto 
    }
    };
} // namespace neko
