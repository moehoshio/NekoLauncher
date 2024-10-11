#pragma once
#include <string>
#include <functional>
namespace ui {

    struct InputMsg {
        std::string title,
        msg,
        poster;
        std::vector<std::string> lines;
        std::function<void(bool)> callback;
        // InputMsg(const std::string & title ,const std::string & msg ,const std::string & poster,const std::vector<std::string> & lines,std::function<void(bool)> callBack) : title(title) , msg(msg),poster(poster),lines(lines),callback(callBack){};
    };

    struct loadMsg {
        enum Type {
            OnlyRaw,//only show loading ico and process text
            Text,//show onlyraw and text widget
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
        // loadMsg(Type type ,const std::string &process ,const std::string &h1,const std::string &h2,const std::string &msg,const std::string &poster,int speed,int progressVal,int progressMax) :  type(type) , process(process), h1(h1),h2(h2),msg(msg),poster(poster),speed(speed),progressVal(progressVal),progressMax(progressMax){};
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
        std::function<void(bool)> callback = nullptr;
        // hintMsg(const std::string & title,const std::string &msg,const std::string &poster,int buttonType,std::function<void(bool)> callback) : title(title),msg(msg),poster(poster),buttonType(buttonType),callback(callback){};
    };
    
} // namespace ui
