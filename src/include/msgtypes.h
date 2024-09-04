#pragma once
#include <string>
#include <functional>
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
        // button type 1 : use one button
        int buttonType;
        // Callback function after clicking the button.
        // If using a radio button, the returned boolean value can be ignored.
        std::function<void(bool)> callback;
    };
    
} // namespace ui
