/**
 * @brief NekoLc Event Types
 * @file eventTypes.hpp
 * @author moehoshio
 */

#pragma once

#include <neko/schema/types.hpp>

#include "neko/app/api.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/ui/page.hpp"
#include "neko/ui/uiMsg.hpp"

#include <string>
#include <functional>
#include <vector>

namespace neko::event {

    /*****************/
    /** App Events **/
    /*****************/

    struct NekoStartEvent {};
    struct NekoQuitEvent {};

    struct ConfigLoadedEvent {
        std::string path;
        bool success = false;
    };

    struct ConfigSavedEvent {
        std::string path;
        bool success = false;
    };

    struct ConfigUpdatedEvent {
        neko::ClientConfig config;
    };

    /*****************/
    /*** UI Events ***/
    /*****************/

    struct CurrentPageChangeEvent {
        ui::Page page;
    };
    struct ShowNoticeEvent : public neko::ui::NoticeMsg {
        ShowNoticeEvent(const neko::ui::NoticeMsg &msg) : neko::ui::NoticeMsg(msg) {}
    };
    struct ShowLoadingEvent : public neko::ui::LoadingMsg {
        ShowLoadingEvent(const neko::ui::LoadingMsg &msg) : neko::ui::LoadingMsg(msg) {}
    };
    struct ShowInputEvent : public neko::ui::InputMsg {
        ShowInputEvent(const neko::ui::InputMsg &msg) : neko::ui::InputMsg(msg) {}
    };
    struct LoadingValueChangedEvent {
        neko::uint32 progressValue;
    };
    struct LoadingStatusChangedEvent {
        std::string statusMessage;
    };
    struct LoadingChangedEvent : public LoadingValueChangedEvent, public LoadingStatusChangedEvent {
        LoadingChangedEvent(const std::string &statusMessage, neko::uint32 progressValue)
            : LoadingValueChangedEvent{progressValue}, LoadingStatusChangedEvent{statusMessage} {}
    };
    // Request UI to refresh localized text (e.g., after config or resource updates).
    struct RefreshTextEvent {};
    struct HideInputEvent {};


    /*********************/
    /** Launching Events **/
    /*********************/

    struct LaunchRequestEvent {
        std::function<void(void)> onStart;
        std::function<void(int)> onExit;
    };
    struct LaunchStartedEvent {};
    struct LaunchFinishedEvent {
        int exitCode = 0;
    };

    struct ProcessStartedEvent {
        std::string command;
        std::string workingDir;
        bool detached = false;
    };
    struct ProcessExitedEvent {
        std::string command;
        int exitCode = 0;
        bool detached = false;
    };
    struct LaunchFailedEvent {
        std::string reason;
        int exitCode = -1;
    };

    struct RestartRequestEvent {
        std::string reason;
        std::string command;
    };


    /*****************/
    /** Core Events **/
    /*****************/

    struct MaintenanceEvent : public neko::ui::NoticeMsg {
        MaintenanceEvent() = default;
        MaintenanceEvent(const neko::ui::NoticeMsg &msg) : neko::ui::NoticeMsg(msg) {}
    };
    struct UpdateAvailableEvent : api::UpdateResponse {
        UpdateAvailableEvent() = default;
        explicit UpdateAvailableEvent(const api::UpdateResponse &resp) : api::UpdateResponse(resp) {}
    };
    struct UpdateCompleteEvent {};
    struct UpdateFailedEvent {
        std::string reason;
    };

    struct NewsLoadedEvent {
        std::vector<api::NewsItem> items;
        bool hasMore = false;
    };
    struct NewsLoadFailedEvent {
        std::string reason;
    };

    /*****************/
    /** BGM Events ***/
    /*****************/

    /**
     * @brief Event published when a line is read from process stdout.
     */
    struct ProcessOutputEvent {
        std::string line;
    };

    /**
     * @brief Event published when a new line is read from a log file.
     * Used by LogFileWatcher to notify BGM system of Minecraft log events.
     */
    struct LogFileLineEvent {
        std::string line;   ///< The log line content
        std::string source; ///< Path to the log file
    };

    struct BgmStateChangedEvent {
        int state; // Maps to neko::core::BgmState enum value
        std::string track;
    };

    struct BgmTriggerMatchedEvent {
        std::string triggerName;
        std::string pattern;
        std::string musicPath;
        std::string outputLine;
    };

    /*******************/
    /** Network Events **/
    /*******************/

    /**
     * @brief Event published when network initialization fails.
     * UI should display an error dialog with options to retry or adjust settings.
     */
    struct NetworkInitFailedEvent {
        std::string reason;
        bool allowRetry = true;
    };

    /**
     * @brief Event to request network re-initialization.
     * Published when user clicks "Retry" after a network failure.
     */
    struct NetworkRetryRequestEvent {};

    /**
     * @brief Event published when user navigates to settings due to network error.
     * Used to track that we need to show network error dialog when leaving settings.
     */
    struct NetworkSettingsRequestedEvent {};

    /**
     * @brief Event published when user leaves settings page after a network error.
     * Triggers re-display of network error dialog.
     */
    struct NetworkSettingsClosedEvent {};
    
} // namespace neko::event