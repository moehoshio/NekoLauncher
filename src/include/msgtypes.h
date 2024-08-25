#pragma once
#include <string>

namespace ui {
    struct updateMsg {
        std::string h1;
        std::string h2;
        std::string msg;
        std::string poster;
        int max;
    };

    struct hintMsg
    {
        std::string title,
        msg,
        poster;
        int buttonType;
        std::function<void(bool)> callback;
    };
    
} // namespace ui
