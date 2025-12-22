/**
 * @file bgm.cpp
 * @brief Background music (BGM) playback module implementation
 * @author moehoshio
 */

#include "neko/core/bgm.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include <neko/log/nlog.hpp>
#include <neko/function/utilities.hpp>
#include <neko/schema/exception.hpp>

#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtWidgets/QApplication>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>

namespace neko::core {

    struct BgmManager::Impl {
        BgmConfig config;
        BgmState state = BgmState::Stopped;

        std::unique_ptr<QMediaPlayer> player;
        std::unique_ptr<QAudioOutput> audioOutput;
        std::unique_ptr<QTimer> fadeTimer;

        std::vector<std::pair<std::regex, BgmTrigger>> compiledTriggers;
        std::function<void(BgmState)> stateCallback;

        std::string currentTrack;
        float targetVolume = 0.7f;
        float fadeStartVolume = 0.0f;
        float fadeEndVolume = 0.0f;
        int fadeElapsedMs = 0;
        int fadeDurationMs = 0;
        bool fadingIn = false;
        bool fadingOut = false;
        bool pendingStop = false;

        mutable std::mutex mutex;

        void setState(BgmState newState) {
            state = newState;
            if (stateCallback) {
                stateCallback(state);
            }
            bus::event::publish(event::BgmStateChangedEvent{.state = static_cast<int>(newState), .track = currentTrack});
        }

        void compileTriggers() {
            compiledTriggers.clear();
            for (const auto &trigger : config.triggers) {
                try {
                    std::regex re(trigger.pattern, std::regex::icase | std::regex::optimize);
                    compiledTriggers.emplace_back(std::move(re), trigger);
                } catch (const std::regex_error &e) {
                    log::warn("Failed to compile BGM trigger regex '{}': {}", {}, trigger.pattern, e.what());
                }
            }
            // Sort by priority (descending)
            std::sort(compiledTriggers.begin(), compiledTriggers.end(),
                      [](const auto &a, const auto &b) { return a.second.priority > b.second.priority; });
        }

        std::string resolveMusicPath(const std::string &path) const {
            if (path.empty()) {
                return path;
            }
            if (std::filesystem::path(path).is_absolute()) {
                return path;
            }
            if (!config.basePath.empty()) {
                return (std::filesystem::path(config.basePath) / path).string();
            }
            return path;
        }

        void startFade(float startVol, float endVol, int durationMs, bool isFadeIn) {
            fadeStartVolume = startVol;
            fadeEndVolume = endVol;
            fadeDurationMs = durationMs;
            fadeElapsedMs = 0;
            fadingIn = isFadeIn;
            fadingOut = !isFadeIn;

            if (fadeTimer) {
                fadeTimer->start(16); // ~60fps update rate
            }
        }

        void updateFade() {
            if (!fadingIn && !fadingOut) {
                return;
            }

            fadeElapsedMs += 16;
            float progress = std::min(1.0f, static_cast<float>(fadeElapsedMs) / static_cast<float>(fadeDurationMs));

            // Ease in-out quad
            float easedProgress;
            if (progress < 0.5f) {
                easedProgress = 2.0f * progress * progress;
            } else {
                easedProgress = 1.0f - std::pow(-2.0f * progress + 2.0f, 2.0f) / 2.0f;
            }

            float currentVolume = fadeStartVolume + (fadeEndVolume - fadeStartVolume) * easedProgress;
            audioOutput->setVolume(currentVolume);

            if (progress >= 1.0f) {
                fadeTimer->stop();
                if (fadingOut && pendingStop) {
                    player->stop();
                    pendingStop = false;
                    setState(BgmState::Stopped);
                }
                fadingIn = false;
                fadingOut = false;
            }
        }
    };

    BgmManager::BgmManager() : pImpl(std::make_unique<Impl>()) {
        pImpl->player = std::make_unique<QMediaPlayer>();
        pImpl->audioOutput = std::make_unique<QAudioOutput>();
        pImpl->player->setAudioOutput(pImpl->audioOutput.get());

        // Setup fade timer
        pImpl->fadeTimer = std::make_unique<QTimer>();
        QObject::connect(pImpl->fadeTimer.get(), &QTimer::timeout, [this]() {
            pImpl->updateFade();
        });

        // Connect to media player signals for state tracking
        QObject::connect(pImpl->player.get(), &QMediaPlayer::playbackStateChanged,
                         [this](QMediaPlayer::PlaybackState state) {
                             std::lock_guard<std::mutex> lock(pImpl->mutex);
                             switch (state) {
                             case QMediaPlayer::PlayingState:
                                 pImpl->setState(BgmState::Playing);
                                 break;
                             case QMediaPlayer::PausedState:
                                 pImpl->setState(BgmState::Paused);
                                 break;
                             case QMediaPlayer::StoppedState:
                                 if (!pImpl->pendingStop) {
                                     pImpl->setState(BgmState::Stopped);
                                 }
                                 break;
                             }
                         });

        QObject::connect(pImpl->player.get(), &QMediaPlayer::errorOccurred,
                         [this](QMediaPlayer::Error error, const QString &errorString) {
                             log::error("BGM playback error: {} - {}", {}, static_cast<int>(error), errorString.toStdString());
                             std::lock_guard<std::mutex> lock(pImpl->mutex);
                             pImpl->setState(BgmState::Error);
                         });

        QObject::connect(pImpl->player.get(), &QMediaPlayer::mediaStatusChanged,
                         [this](QMediaPlayer::MediaStatus status) {
                             if (status == QMediaPlayer::LoadedMedia) {
                                 log::debug("BGM media loaded: {}", {}, pImpl->currentTrack);
                             } else if (status == QMediaPlayer::InvalidMedia) {
                                 log::error("BGM invalid media: {}", {}, pImpl->currentTrack);
                                 std::lock_guard<std::mutex> lock(pImpl->mutex);
                                 pImpl->setState(BgmState::Error);
                             }
                         });
    }

    BgmManager::~BgmManager() {
        if (pImpl) {
            if (pImpl->fadeTimer) {
                pImpl->fadeTimer->stop();
            }
            if (pImpl->player) {
                pImpl->player->stop();
            }
        }
    }

    BgmManager::BgmManager(BgmManager &&) noexcept = default;
    BgmManager &BgmManager::operator=(BgmManager &&) noexcept = default;

    bool BgmManager::initialize(const BgmConfig &config) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->config = config;
        pImpl->compileTriggers();

        pImpl->audioOutput->setVolume(config.masterVolume);
        pImpl->targetVolume = config.masterVolume;

        log::info("BGM system initialized with {} triggers, enabled: {}", {},
                  pImpl->config.triggers.size(), pImpl->config.enabled);
        return true;
    }

    void BgmManager::processOutput(const std::string &outputLine) {
        if (!pImpl->config.enabled) {
            return;
        }

        std::lock_guard<std::mutex> lock(pImpl->mutex);

        for (const auto &[regex, trigger] : pImpl->compiledTriggers) {
            if (std::regex_search(outputLine, regex)) {
                log::debug("BGM trigger matched: '{}' for pattern '{}'", {}, trigger.name, trigger.pattern);

                // Handle stop trigger (empty musicPath)
                if (trigger.musicPath.empty()) {
                    pImpl->mutex.unlock();
                    stop(trigger.fadeOutMs);
                    pImpl->mutex.lock();
                    return;
                }

                std::string musicPath = pImpl->resolveMusicPath(trigger.musicPath);

                // Check if it's already playing the same track
                if (pImpl->currentTrack == musicPath && pImpl->state == BgmState::Playing) {
                    log::debug("BGM already playing: {}", {}, musicPath);
                    return;
                }

                // Start playback with the trigger's settings
                float effectiveVolume = trigger.volume * pImpl->config.masterVolume;

                bus::event::publish(event::BgmTriggerMatchedEvent{
                    .triggerName = trigger.name,
                    .pattern = trigger.pattern,
                    .musicPath = musicPath,
                    .outputLine = outputLine});

                // Unlock before calling play to avoid deadlock
                pImpl->mutex.unlock();

                // Fade out current track if playing
                if (pImpl->state == BgmState::Playing) {
                    stop(trigger.fadeOutMs);
                    // Wait for fade out (schedule the new track)
                    bus::event::scheduleTask(trigger.fadeOutMs + 50, [this, musicPath, trigger, effectiveVolume]() {
                        playInternal(musicPath, trigger.loop, trigger.fadeInMs, effectiveVolume);
                    });
                } else {
                    playInternal(musicPath, trigger.loop, trigger.fadeInMs, effectiveVolume);
                }

                pImpl->mutex.lock();
                return; // Only process the first (highest priority) match
            }
        }
    }

    void BgmManager::playInternal(const std::string &musicPath, bool loop, neko::uint32 fadeInMs, float volume) {
        if (musicPath.empty()) {
            log::warn("BGM playInternal called with empty musicPath");
            return;
        }

        if (!std::filesystem::exists(musicPath)) {
            log::error("BGM file not found: {}", {}, musicPath);
            return;
        }

        // Ensure Qt operations run on the main thread
        if (QThread::currentThread() != QApplication::instance()->thread()) {
            QMetaObject::invokeMethod(QApplication::instance(), [this, musicPath, loop, fadeInMs, volume]() {
                playInternal(musicPath, loop, fadeInMs, volume);
            }, Qt::QueuedConnection);
            return;
        }

        pImpl->currentTrack = musicPath;
        pImpl->targetVolume = volume;
        pImpl->pendingStop = false;

        pImpl->player->setSource(QUrl::fromLocalFile(QString::fromStdString(musicPath)));

        if (loop) {
            pImpl->player->setLoops(QMediaPlayer::Infinite);
        } else {
            pImpl->player->setLoops(1);
        }

        // Start with zero volume for fade-in
        if (fadeInMs > 0) {
            pImpl->audioOutput->setVolume(0.0f);
        } else {
            pImpl->audioOutput->setVolume(volume);
        }

        pImpl->setState(BgmState::Loading);
        pImpl->player->play();

        // Fade in using timer
        if (fadeInMs > 0) {
            pImpl->startFade(0.0f, volume, static_cast<int>(fadeInMs), true);
        }

        log::info("BGM playing: {} (loop: {}, volume: {:.2f}, fadeIn: {}ms)", {},
                  musicPath, loop, volume, fadeInMs);
    }

    bool BgmManager::play(const std::string &musicPath, bool loop, neko::uint32 fadeInMs) {
        std::string resolvedPath;
        {
            std::lock_guard<std::mutex> lock(pImpl->mutex);
            resolvedPath = pImpl->resolveMusicPath(musicPath);
        }

        playInternal(resolvedPath, loop, fadeInMs, pImpl->targetVolume);
        return true;
    }

    void BgmManager::stop(neko::uint32 fadeOutMs) {
        // Ensure Qt operations run on the main thread
        if (QThread::currentThread() != QApplication::instance()->thread()) {
            QMetaObject::invokeMethod(QApplication::instance(), [this, fadeOutMs]() {
                stop(fadeOutMs);
            }, Qt::QueuedConnection);
            return;
        }

        std::lock_guard<std::mutex> lock(pImpl->mutex);

        if (pImpl->state == BgmState::Stopped) {
            return;
        }

        pImpl->pendingStop = true;

        if (fadeOutMs > 0) {
            pImpl->startFade(pImpl->audioOutput->volume(), 0.0f, static_cast<int>(fadeOutMs), false);
        } else {
            pImpl->player->stop();
            pImpl->pendingStop = false;
            pImpl->setState(BgmState::Stopped);
        }

        log::info("BGM stopping (fadeOut: {}ms)", {}, fadeOutMs);
    }

    void BgmManager::pause() {
        // Ensure Qt operations run on the main thread
        if (QThread::currentThread() != QApplication::instance()->thread()) {
            QMetaObject::invokeMethod(QApplication::instance(), [this]() {
                pause();
            }, Qt::QueuedConnection);
            return;
        }

        std::lock_guard<std::mutex> lock(pImpl->mutex);
        if (pImpl->state == BgmState::Playing) {
            pImpl->player->pause();
        }
    }

    void BgmManager::resume() {
        // Ensure Qt operations run on the main thread
        if (QThread::currentThread() != QApplication::instance()->thread()) {
            QMetaObject::invokeMethod(QApplication::instance(), [this]() {
                resume();
            }, Qt::QueuedConnection);
            return;
        }

        std::lock_guard<std::mutex> lock(pImpl->mutex);
        if (pImpl->state == BgmState::Paused) {
            pImpl->player->play();
        }
    }

    void BgmManager::setVolume(float volume) {
        // Ensure Qt operations run on the main thread
        if (QThread::currentThread() != QApplication::instance()->thread()) {
            QMetaObject::invokeMethod(QApplication::instance(), [this, volume]() {
                setVolume(volume);
            }, Qt::QueuedConnection);
            return;
        }

        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->targetVolume = std::clamp(volume, 0.0f, 1.0f);
        pImpl->audioOutput->setVolume(pImpl->targetVolume);
    }

    float BgmManager::getVolume() const {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        return pImpl->audioOutput->volume();
    }

    BgmState BgmManager::getState() const {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        return pImpl->state;
    }

    bool BgmManager::isPlaying() const {
        return getState() == BgmState::Playing;
    }

    const BgmConfig &BgmManager::getConfig() const {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        return pImpl->config;
    }

    void BgmManager::addTrigger(const BgmTrigger &trigger) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->config.triggers.push_back(trigger);
        pImpl->compileTriggers();
    }

    bool BgmManager::removeTrigger(const std::string &name) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        auto &triggers = pImpl->config.triggers;
        auto it = std::remove_if(triggers.begin(), triggers.end(),
                                 [&name](const BgmTrigger &t) { return t.name == name; });
        if (it != triggers.end()) {
            triggers.erase(it, triggers.end());
            pImpl->compileTriggers();
            return true;
        }
        return false;
    }

    void BgmManager::clearTriggers() {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->config.triggers.clear();
        pImpl->compiledTriggers.clear();
    }

    void BgmManager::setEnabled(bool enabled) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->config.enabled = enabled;
        if (!enabled && pImpl->state == BgmState::Playing) {
            pImpl->mutex.unlock();
            stop(pImpl->config.defaultFadeMs);
            pImpl->mutex.lock();
        }
    }

    void BgmManager::setStateCallback(std::function<void(BgmState)> callback) {
        std::lock_guard<std::mutex> lock(pImpl->mutex);
        pImpl->stateCallback = std::move(callback);
    }

    // Global instance
    BgmManager &getBgmManager() {
        static BgmManager instance;
        return instance;
    }

    // JSON serialization helpers
    namespace {
        void from_json(const nlohmann::json &j, BgmTrigger &t) {
            t.pattern = j.value("pattern", "");
            t.musicPath = j.value("musicPath", "");
            t.loop = j.value("loop", true);
            t.fadeInMs = j.value("fadeInMs", 500u);
            t.fadeOutMs = j.value("fadeOutMs", 500u);
            t.volume = j.value("volume", 0.7f);
            t.priority = j.value("priority", 0);
            t.name = j.value("name", "");
        }

        void to_json(nlohmann::json &j, const BgmTrigger &t) {
            j = nlohmann::json{
                {"pattern", t.pattern},
                {"musicPath", t.musicPath},
                {"loop", t.loop},
                {"fadeInMs", t.fadeInMs},
                {"fadeOutMs", t.fadeOutMs},
                {"volume", t.volume},
                {"priority", t.priority},
                {"name", t.name}};
        }

        void from_json(const nlohmann::json &j, BgmConfig &c) {
            c.enabled = j.value("enabled", true);
            c.basePath = j.value("basePath", "");
            c.masterVolume = j.value("masterVolume", 1.0f);
            c.defaultFadeMs = j.value("defaultFadeMs", 500u);

            if (j.contains("triggers") && j["triggers"].is_array()) {
                c.triggers.clear();
                for (const auto &triggerJson : j["triggers"]) {
                    BgmTrigger trigger;
                    from_json(triggerJson, trigger);
                    c.triggers.push_back(trigger);
                }
            }
        }

        void to_json(nlohmann::json &j, const BgmConfig &c) {
            j = nlohmann::json{
                {"enabled", c.enabled},
                {"basePath", c.basePath},
                {"masterVolume", c.masterVolume},
                {"defaultFadeMs", c.defaultFadeMs}};

            nlohmann::json triggersArray = nlohmann::json::array();
            for (const auto &trigger : c.triggers) {
                nlohmann::json triggerJson;
                to_json(triggerJson, trigger);
                triggersArray.push_back(triggerJson);
            }
            j["triggers"] = triggersArray;
        }
    } // namespace

    std::vector<BgmTrigger> loadTriggersFromJson(const std::string &configPath) {
        std::vector<BgmTrigger> triggers;

        if (!std::filesystem::exists(configPath)) {
            log::warn("BGM config file not found: {}", {}, configPath);
            return triggers;
        }

        std::ifstream file(configPath);
        if (!file.is_open()) {
            throw ex::FileError("Failed to open BGM config file: " + configPath);
        }

        try {
            nlohmann::json j;
            file >> j;

            if (j.contains("triggers") && j["triggers"].is_array()) {
                for (const auto &triggerJson : j["triggers"]) {
                    BgmTrigger trigger;
                    from_json(triggerJson, trigger);
                    triggers.push_back(trigger);
                }
            }

            log::info("Loaded {} BGM triggers from {}", {}, triggers.size(), configPath);
        } catch (const nlohmann::json::parse_error &e) {
            throw ex::Parse("Failed to parse BGM config JSON: " + std::string(e.what()));
        }

        return triggers;
    }

    BgmConfig loadBgmConfigFromJson(const std::string &configPath) {
        BgmConfig config;

        if (!std::filesystem::exists(configPath)) {
            log::warn("BGM config file not found, using defaults: {}", {}, configPath);
            return config;
        }

        std::ifstream file(configPath);
        if (!file.is_open()) {
            throw ex::FileError("Failed to open BGM config file: " + configPath);
        }

        try {
            nlohmann::json j;
            file >> j;
            from_json(j, config);
            log::info("Loaded BGM config from {} with {} triggers", {}, configPath, config.triggers.size());
        } catch (const nlohmann::json::parse_error &e) {
            throw ex::Parse("Failed to parse BGM config JSON: " + std::string(e.what()));
        }

        return config;
    }

    bool saveBgmConfigToJson(const BgmConfig &config, const std::string &configPath) {
        try {
            // Ensure directory exists
            std::filesystem::path path(configPath);
            if (path.has_parent_path()) {
                std::filesystem::create_directories(path.parent_path());
            }

            nlohmann::json j;
            to_json(j, config);

            std::ofstream file(configPath);
            if (!file.is_open()) {
                log::error("Failed to open BGM config file for writing: {}", {}, configPath);
                return false;
            }

            file << j.dump(4); // Pretty print with 4-space indent
            log::info("Saved BGM config to {}", {}, configPath);
            return true;
        } catch (const std::exception &e) {
            log::error("Failed to save BGM config: {}", {}, e.what());
            return false;
        }
    }

    void subscribeBgmToProcessEvents() {
        // Subscribe to process stdout events for BGM triggering
        bus::event::subscribe<event::ProcessOutputEvent>([](const event::ProcessOutputEvent &ev) {
            getBgmManager().processOutput(ev.line);
        });

        // Subscribe to log file events for BGM triggering (Minecraft log file)
        bus::event::subscribe<event::LogFileLineEvent>([](const event::LogFileLineEvent &ev) {
            getBgmManager().processOutput(ev.line);
        });

        // Stop BGM when process exits
        bus::event::subscribe<event::ProcessExitedEvent>([](const event::ProcessExitedEvent &) {
            getBgmManager().stop(1000);
        });

        log::info("BGM manager subscribed to process and log file events");
    }

} // namespace neko::core
