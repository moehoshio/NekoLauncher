/**
 * @file wmsg.hpp
 * @brief Header file defining structures for window messages in the Neko UI framework.
 * This file contains structures for input messages, loading messages, and hint messages.
 * These structures are used to manage user interactions and display messages in the UI.
 */

#pragma once

// Structure definition for window messages

#include <functional>
#include <string>
#include <vector>

namespace neko::ui {

    /**
     * @brief Structure representing an input message dialog.
     */
    struct InputMsg {
        /**
         * @brief The title of the input dialog.
         */
        std::string title;

        /**
         * @brief The main message content.
         */
        std::string msg;

        /**
         * @brief Path to the background image (poster).
         */
        std::string poster;

        /**
         * @brief Each string represents a line, returned by getLines method, maintaining the same order as input.
         */
        std::vector<std::string> lines;

        /**
         * @brief Callback function for the cancel or confirm button click.
         * @param bool True if confirmed, false if cancelled.
         */
        std::function<void(bool)> callback;

        // InputMsg(const std::string & title ,const std::string & msg ,const std::string & poster,const std::vector<std::string> & lines,std::function<void(bool)> callBack) : title(title) , msg(msg),poster(poster),lines(lines),callback(callBack){};
    };

    /**
     * @brief Structure representing a loading message window.
     */
    struct loadMsg {
        /**
         * @brief Enum for the type of loading message to display.
         */
        enum Type {
            OnlyRaw,  /**< Only show loading icon and process text */
            Text,     /**< Show text widget, process, and loading icon */
            Progress, /**< Show progress bar, process, and loading icon */
            All       /**< Show all elements */
        };

        /**
         * @brief Show type.
         */
        Type type;

        /**
         * @brief Process text, default is "loading...".
         */
        std::string process = "loading...";

        /**
         * @brief Title of the text widget.
         */
        std::string h1;

        /**
         * @brief Time or other info to display in the second line of the text widget.
         */
        std::string h2;

        /**
         * @brief Additional message to display in the text widget.
         */
        std::string msg;

        /**
         * @brief Path to the background image (poster).
         */
        std::string poster;

        /**
         * @brief Speed of the loading icon (gif) animation in milliseconds.
         */
        int speed = 100;

        /**
         * @brief Current progress value.
         */
        int progressVal;

        /**
         * @brief Maximum progress value.
         */
        int progressMax;

        // loadMsg(Type type ,const std::string &process ,const std::string &h1,const std::string &h2,const std::string &msg,const std::string &poster,int speed,int progressVal,int progressMax) :  type(type) , process(process), h1(h1),h2(h2),msg(msg),poster(poster),speed(speed),progressVal(progressVal),progressMax(progressMax){};
    };

    /**
     * @brief Structure representing a hint or alert message dialog.
     */
    struct hintMsg {
        /**
         * @brief The title of the hint dialog.
         */
        std::string title;

        /**
         * @brief The main message content.
         */
        std::string msg;

        /**
         * @brief Path to the background image (poster).
         */
        std::string poster;

        /**
         * @brief Number of buttons to display.
         * 1: only OK button
         * 2: OK and Cancel button
         * other value: default show two buttons
         */
        int buttonNum = 1;

        /**
         * @brief Callback function after clicking the button.
         * If using a radio button, the returned boolean value can be ignored.
         * @param bool True if confirmed, false if cancelled.
         * default is an empty function that does nothing.
         */
        std::function<void(bool)> callback = [](bool) {};

        // hintMsg(const std::string & title,const std::string &msg,const std::string &poster,int buttonType,std::function<void(bool)> callback) : title(title),msg(msg),poster(poster),buttonType(buttonType),callback(callback){};
    };

} // namespace neko::ui
