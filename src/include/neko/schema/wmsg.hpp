#pragma once

// Structure definition for window messages

#include <string>
#include <vector>
#include <functional>

namespace ui {

    struct InputMsg {
        std::string title,
        msg,
        poster; // background image path
        
        std::vector<std::string> lines; // Each string represents a line, returned by getLines method, maintaining the same order as input

        std::function<void(bool)> callback; // Callback function for the cancel or confirm button click

        // InputMsg(const std::string & title ,const std::string & msg ,const std::string & poster,const std::vector<std::string> & lines,std::function<void(bool)> callBack) : title(title) , msg(msg),poster(poster),lines(lines),callback(callBack){};
    };

    struct loadMsg {
        enum Type {
            OnlyRaw,//only show loading ico and process text
            Text,//show text widget and process、loading ico
            Progress,//show progress bar and process、loading ico
            All
        };
        Type type; // show type
        std::string process = "loading...";
        std::string h1; // title
        std::string h2; // time or other info
        std::string msg; // message
        std::string poster; // background image path
        int speed = 100;
        int progressVal;
        int progressMax;
        // loadMsg(Type type ,const std::string &process ,const std::string &h1,const std::string &h2,const std::string &msg,const std::string &poster,int speed,int progressVal,int progressMax) :  type(type) , process(process), h1(h1),h2(h2),msg(msg),poster(poster),speed(speed),progressVal(progressVal),progressMax(progressMax){};
    };

    struct hintMsg
    {
        std::string title,
        msg,
        poster; // background image path

        // 1 : only OKay button
        // 2 : OKay and Cancel button
        // other value : defalut show two button
        int buttonNum;
        // Callback function after clicking the button.
        // If using a radio button, the returned boolean value can be ignored.
        std::function<void(bool)> callback = [](bool){};
        // hintMsg(const std::string & title,const std::string &msg,const std::string &poster,int buttonType,std::function<void(bool)> callback) : title(title),msg(msg),poster(poster),buttonType(buttonType),callback(callback){};
    };
    
} // namespace ui
