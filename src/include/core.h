#pragma once

#include <nlohmann/json.hpp>

#include "network.h"

namespace neko {
    class core {
    public:
        enum class State {
            over,
            unDone,
            tryAgainLater
        };
    inline State autoUpdate(){
        
    }
    };
} // namespace neko
