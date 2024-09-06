#pragma once
#include <string>
#include <functional>
namespace ui {
    struct loadMsg {
        enum Type {
            OnlyRaw,//only loading ico and process text
            Text,//show onlyraw and test widget
            Progress,//show onlyraw and progress
            All
        };
        Type type;
        std::string process = "loading...";
        std::string h1;
        std::string h2;
        std::string msg;
        std::string poster;
        int speed = 100;
        int progressVal;
        int progressMax;
         
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
