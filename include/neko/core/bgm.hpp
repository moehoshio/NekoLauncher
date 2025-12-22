/**
 * @file bgm.hpp
 * @brief Background music (BGM) playback module triggered by process output
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

#include <neko/schema/types.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace neko::core {

    /**
     * @enum BgmState
     * @brief Represents the current state of the BGM player.
     */
    enum class BgmState {
        Stopped,   ///< No music is playing
        Playing,   ///< Music is currently playing
        Paused,    ///< Music is paused
        Loading,   ///< Loading a new track
        Error      ///< An error occurred during playback
    };

    /**
     * @struct BgmTrigger
     * @brief Defines a trigger pattern and its associated music file.
     */
    struct BgmTrigger {
        /**
         * @var pattern
         * @brief Regex pattern to match against process output.
         * @example "\\[Client\\]\\s*Loading\\s*world" to match world loading messages.
         */
        std::string pattern;

        /**
         * @var musicPath
         * @brief Path to the music file to play when the pattern matches.
         * @example "/path/to/bgm/world_loading.mp3"
         */
        std::string musicPath;

        /**
         * @var loop
         * @brief Whether to loop the music.
         * @default true
         */
        bool loop = true;

        /**
         * @var fadeInMs
         * @brief Fade-in duration in milliseconds.
         * @default 500
         */
        neko::uint32 fadeInMs = 500;

        /**
         * @var fadeOutMs
         * @brief Fade-out duration in milliseconds when switching tracks.
         * @default 500
         */
        neko::uint32 fadeOutMs = 500;

        /**
         * @var volume
         * @brief Volume level (0.0 to 1.0).
         * @default 0.7
         */
        float volume = 0.7f;

        /**
         * @var priority
         * @brief Priority level for this trigger (higher values take precedence).
         * @default 0
         */
        neko::int32 priority = 0;

        /**
         * @var name
         * @brief Optional name for this trigger (for logging/debugging).
         */
        std::string name;
    };

    /**
     * @struct BgmConfig
     * @brief Configuration for the BGM system.
     */
    struct BgmConfig {
        /**
         * @var enabled
         * @brief Whether the BGM system is enabled.
         */
        bool enabled = true;

        /**
         * @var basePath
         * @brief Base path for music files. Relative paths in triggers will be resolved from here.
         */
        std::string basePath;

        /**
         * @var masterVolume
         * @brief Master volume multiplier (0.0 to 1.0).
         */
        float masterVolume = 1.0f;

        /**
         * @var triggers
         * @brief List of BGM triggers to match against process output.
         */
        std::vector<BgmTrigger> triggers;

        /**
         * @var defaultFadeMs
         * @brief Default fade duration when not specified in trigger.
         */
        neko::uint32 defaultFadeMs = 500;
    };

    /**
     * @class BgmManager
     * @brief Manages background music playback triggered by process output.
     *
     * This class monitors process output (e.g., from Minecraft) and plays
     * appropriate background music based on configured trigger patterns.
     */
    class BgmManager {
    public:
        /**
         * @brief Constructs a BgmManager instance.
         */
        BgmManager();

        /**
         * @brief Destructor.
         */
        ~BgmManager();

        // Non-copyable
        BgmManager(const BgmManager &) = delete;
        BgmManager &operator=(const BgmManager &) = delete;

        // Movable
        BgmManager(BgmManager &&) noexcept;
        BgmManager &operator=(BgmManager &&) noexcept;

        /**
         * @brief Initializes the BGM system with the given configuration.
         * @param config The BGM configuration.
         * @return true if initialization succeeded, false otherwise.
         */
        bool initialize(const BgmConfig &config);

        /**
         * @brief Processes a line of output and checks for trigger matches.
         * @param outputLine The output line from the process.
         * @note This should be called for each line of process output.
         */
        void processOutput(const std::string &outputLine);

        /**
         * @brief Plays a specific music file.
         * @param musicPath Path to the music file.
         * @param loop Whether to loop the music.
         * @param fadeInMs Fade-in duration in milliseconds.
         * @return true if playback started successfully.
         */
        bool play(const std::string &musicPath, bool loop = true, neko::uint32 fadeInMs = 500);

        /**
         * @brief Stops the currently playing music.
         * @param fadeOutMs Fade-out duration in milliseconds.
         */
        void stop(neko::uint32 fadeOutMs = 500);

        /**
         * @brief Pauses the current music.
         */
        void pause();

        /**
         * @brief Resumes the paused music.
         */
        void resume();

        /**
         * @brief Sets the volume.
         * @param volume Volume level (0.0 to 1.0).
         */
        void setVolume(float volume);

        /**
         * @brief Gets the current volume.
         * @return Current volume level (0.0 to 1.0).
         */
        float getVolume() const;

        /**
         * @brief Gets the current playback state.
         * @return Current BGM state.
         */
        BgmState getState() const;

        /**
         * @brief Checks if any music is currently playing.
         * @return true if music is playing.
         */
        bool isPlaying() const;

        /**
         * @brief Gets the current configuration.
         * @return The current BGM configuration.
         */
        const BgmConfig &getConfig() const;

        /**
         * @brief Adds a trigger at runtime.
         * @param trigger The trigger to add.
         */
        void addTrigger(const BgmTrigger &trigger);

        /**
         * @brief Removes a trigger by name.
         * @param name The name of the trigger to remove.
         * @return true if the trigger was found and removed.
         */
        bool removeTrigger(const std::string &name);

        /**
         * @brief Clears all triggers.
         */
        void clearTriggers();

        /**
         * @brief Enables or disables the BGM system.
         * @param enabled Whether to enable or disable.
         */
        void setEnabled(bool enabled);

        /**
         * @brief Sets a callback for state changes.
         * @param callback The callback function.
         */
        void setStateCallback(std::function<void(BgmState)> callback);

    private:
        /**
         * @brief Internal play implementation.
         */
        void playInternal(const std::string &musicPath, bool loop, neko::uint32 fadeInMs, float volume);

        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };

    /**
     * @brief Gets the global BGM manager instance.
     * @return Reference to the global BgmManager.
     */
    BgmManager &getBgmManager();

    /**
     * @brief Loads BGM triggers from a JSON configuration file.
     * @param configPath Path to the JSON configuration file.
     * @return A vector of BgmTrigger configurations loaded from the file.
     * @throws ex::FileError if the file cannot be read.
     * @throws ex::Parse if the JSON is invalid.
     */
    std::vector<BgmTrigger> loadTriggersFromJson(const std::string &configPath);

    /**
     * @brief Loads BGM configuration from a JSON file.
     * @param configPath Path to the JSON configuration file.
     * @return BgmConfig loaded from the file.
     */
    BgmConfig loadBgmConfigFromJson(const std::string &configPath);

    /**
     * @brief Saves BGM configuration to a JSON file.
     * @param config The configuration to save.
     * @param configPath Path to the JSON configuration file.
     * @return true if saved successfully.
     */
    bool saveBgmConfigToJson(const BgmConfig &config, const std::string &configPath);

    /**
     * @brief Subscribes the BGM manager to process output events.
     * @note This sets up the connection between the event bus and the BGM manager.
     */
    void subscribeBgmToProcessEvents();

} // namespace neko::core
