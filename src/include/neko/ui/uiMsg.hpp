/**
 * @file uiMsg.hpp
 * @brief Header file defining structures for ui messages in the Neko ui module.
 * This file contains structures for input messages, loading messages, and hint messages.
 * These structures are used to manage user interactions and display messages in the UI.
 */

#pragma once

// Structure definition for ui messages

#include "neko/schema/types.hpp"

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
        std::vector<std::string> lineText;

        /**
         * @brief Callback function for the cancel or confirm button click.
         * @param bool True if confirmed, false if cancelled.
         */
        std::function<void(bool)> callback;

        // InputMsg(const std::string & title ,const std::string & msg ,const std::string & poster,const std::vector<std::string> & lines,std::function<void(bool)> callBack) : title(title) , msg(msg),poster(poster),lines(lines),callback(callBack){};
    };

    /**
     * @brief Structure representing a loading message page.
     */
    struct LoadMsg {
        /**
         * @brief Enum for the type of loading message to display.
         */
        enum class Type {
            OnlyRaw,  /**< Only show loading icon and process text */
            Text,     /**< Show text widget, process, and loading icon */
            Progress, /**< Show progress bar, process, and loading icon */
            All       /**< Show all elements */
        };

        /**
         * @brief Show type.
         */
        Type type = Type::OnlyRaw;

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
         * @brief Path to the loading icon (gif).
         * This icon is displayed during the loading process.
         */
        std::string icon = "img/loading.gif"; /**< Path to the loading icon (gif) */

        /**
         * @brief Speed of the loading icon (gif) animation in milliseconds.
         */
        neko::uint32 speed = 100;

        /**
         * @brief Current progress value.
         */
        neko::uint32 progressVal = 0;

        /**
         * @brief Maximum progress value.
         */
        neko::uint32 progressMax = 0;

        // loadMsg(Type type ,const std::string &process ,const std::string &h1,const std::string &h2,const std::string &msg,const std::string &poster,int speed,int progressVal,int progressMax) :  type(type) , process(process), h1(h1),h2(h2),msg(msg),poster(poster),speed(speed),progressVal(progressVal),progressMax(progressMax){};
    };

    /**
     * @brief Structure representing a hint or alert message dialog.
     */
    struct HintMsg {
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
         * @brief Text labels for each button in the dialog.
         * Each element in the vector represents one button.
         */
        std::vector<std::string> buttonText;

        /**
         * @brief Callback function after clicking a button.
         * If using radio buttons, the returned boolean value can be ignored.
         * @param uint32 The callback parameter indicates which button the user clicked; the index corresponds
         * @param uint32 to the order of the buttonText vector.
         * @param uint32 If the dialog auto-closes or the window is closed (process ends), the default value 0 is returned.
         */
        std::function<void(neko::uint32)> callback = nullptr;

        /**
         * @brief Number of auto close millisecond
         * If set to 0, the dialog will not auto close.
         * If set to a positive value, the dialog will close automatically after that many millisecond.
         * @note Auto close will pass 0 to the callback function.
         */
        neko::uint32 autoClose = 0;

        neko::uint32 defaultButtonIndex = 0; /**< Default button index, used when autoClose is set */

        // hintMsg(const std::string & title,const std::string &msg,const std::string &poster,int buttonType,std::function<void(bool)> callback) : title(title),msg(msg),poster(poster),buttonType(buttonType),callback(callback){};
    };

    /**
     * @struct choiceMsg
     * @brief Represents a message dialog with multiple selectable choices.
     *
     * This structure is used to display a dialog with a title, message content, and a set of selectable choices.
     * Each element in the `choices` vector represents a choice column, and each `std::string` within a column
     * represents a specific selectable option in that column.
     */
    struct choiceMsg {
        /**
         * @brief The title of the choice dialog.
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
         * @brief A two-dimensional vector where each inner vector represents a choice column,
         * and each string in the inner vector is a specific option within that column.
         */
        std::vector<std::vector<std::string>> choices;

        /**
         * @brief Callback function for the cancel or confirm button click.
         * @param bool True if confirmed, false if cancelled.
         */
        std::function<void(bool)> callback = nullptr;
    };

} // namespace neko::ui
