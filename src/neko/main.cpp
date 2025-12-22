
#include <neko/log/nlog.hpp>

#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/appinit.hpp"
#include "neko/app/lang.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/core/bgm.hpp"
#include "neko/core/crashReporter.hpp"
#include "neko/core/install.hpp"
#include "neko/core/logFileWatcher.hpp"
#include "neko/core/news.hpp"
#include "neko/core/remoteConfig.hpp"
#include "neko/core/update.hpp"

#include "neko/ui/themeIO.hpp"
#include "neko/ui/uiEventDispatcher.hpp"
#include "neko/ui/windows/logViewerWindow.hpp"
#include "neko/ui/windows/nekoWindow.hpp"

#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>

#include <chrono>
#include <iostream>
#include <thread>

using namespace neko;

namespace {
    // Store the last network error reason for re-displaying dialog after settings
    std::string lastNetworkErrorReason;
    
    // Flag to track if we're in network error recovery mode
    bool networkErrorRecoveryMode = false;

    /**
     * @brief Show network error dialog with options to retry or go to settings.
     * @param reason The error message to display.
     */
    void showNetworkErrorDialog(const std::string &reason) {
        // Store reason for later use when returning from settings
        lastNetworkErrorReason = reason;
        networkErrorRecoveryMode = true;
        
        neko::ui::NoticeMsg notice;
        notice.title = lang::tr(lang::keys::error::category, lang::keys::error::networkInitFailed, "Network Initialization Failed");
        notice.message = reason + "\n\n" + 
            lang::tr(lang::keys::error::category, lang::keys::error::networkInitFailedMessage, 
                     "Please check your network connection or proxy settings.");
        notice.buttonText = {
            lang::tr(lang::keys::network::category, lang::keys::network::goToSettings, "Go to Settings"),
            lang::tr(lang::keys::button::category, lang::keys::button::retry, "Retry")
        };
        notice.callback = [](neko::uint32 buttonIndex) {
            if (buttonIndex == 0) {
                // Go to settings page - publish event so we know to return to network error flow
                bus::event::publish(event::NetworkSettingsRequestedEvent{});
                bus::event::publish(event::CurrentPageChangeEvent{.page = ui::Page::setting});
            } else {
                // Retry network initialization
                networkErrorRecoveryMode = false;
                bus::event::publish(event::NetworkRetryRequestEvent{});
            }
        };
        bus::event::publish(event::ShowNoticeEvent(notice));
    }

    /**
     * @brief Execute post-network initialization tasks (auto-install, update, news).
     */
    void executePostNetworkTasks() {
        bus::thread::submit([]() {
            try {
                const bool installed = core::install::autoInstall();
                if (!installed) {
                    core::update::autoUpdate();
                }

                // Preload launcher news once network and updates are ready.
                try {
                    const auto launcherConfig = core::getRemoteLauncherConfig();
                    const auto newsResponse = core::fetchNews(launcherConfig, 8);
                    if (newsResponse.has_value() && !newsResponse->items.empty()) {
                        bus::event::publish(event::NewsLoadedEvent{
                            .items = newsResponse->items,
                            .hasMore = newsResponse->hasMore});
                        
                        // Check if news should be shown based on dismiss settings
                        const auto clientCfg = bus::config::getClientConfig();
                        bool showNews = true;
                        
                        // Check time-based dismiss
                        if (clientCfg.other.newsDismissUntil > 0) {
                            auto now = std::chrono::system_clock::now();
                            auto nowSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                                now.time_since_epoch()).count();
                            if (nowSeconds < clientCfg.other.newsDismissUntil) {
                                showNews = false;
                            }
                        }
                        
                        // Check version-based dismiss
                        if (!clientCfg.other.newsDismissVersion.empty()) {
                            if (clientCfg.other.newsDismissVersion == std::string(app::getVersion())) {
                                showNews = false;
                            }
                        }
                        
                        if (showNews) {
                            // Switch to news page to display news
                            bus::event::publish(event::CurrentPageChangeEvent{
                                .page = ui::Page::news});
                        } else {
                            // News dismissed, go directly to home
                            bus::event::publish(event::CurrentPageChangeEvent{
                                .page = ui::Page::home});
                        }
                    } else {
                        bus::event::publish(event::NewsLoadedEvent{});
                        // No news, go directly to home
                        bus::event::publish(event::CurrentPageChangeEvent{
                            .page = ui::Page::home});
                    }
                } catch (const ex::Exception &e) {
                    log::warn("News preload failed: {}", {}, e.what());
                    bus::event::publish(event::NewsLoadFailedEvent{.reason = e.what()});
                    // On failure, go to home
                    bus::event::publish(event::CurrentPageChangeEvent{
                        .page = ui::Page::home});
                } catch (const std::exception &e) {
                    log::warn("News preload failed (std): {}", {}, e.what());
                    bus::event::publish(event::NewsLoadFailedEvent{.reason = e.what()});
                    // On failure, go to home
                    bus::event::publish(event::CurrentPageChangeEvent{
                        .page = ui::Page::home});
                }
            } catch (const ex::Exception &e) {
                std::string reason = std::string("Auto-update/install failed: ") + e.what();
                log::error(reason);
                bus::event::publish(event::UpdateFailedEvent{.reason = reason});
                bus::event::publish(event::CurrentPageChangeEvent{
                    .page = ui::Page::home});
            }
        });
    }

    /**
     * @brief Handle network initialization completion.
     * Checks if hosts are available and shows error dialog if not.
     * @param networkReady The future from network initialization.
     */
    void handleNetworkInitCompletion(std::future<void> &networkReady) {
        try {
            networkReady.get();
            
            // Check if we have available hosts
            auto status = app::init::checkNetworkStatus();
            if (!status.success) {
                log::warn("Network initialization completed but no hosts available");
                bus::event::publish(event::NetworkInitFailedEvent{
                    .reason = status.errorMessage,
                    .allowRetry = true
                });
                return;
            }
            
            // Network is ready, proceed with post-network tasks
            executePostNetworkTasks();
            
        } catch (const ex::Exception &e) {
            log::error("Network initialization failed: {}", {}, e.what());
            bus::event::publish(event::NetworkInitFailedEvent{
                .reason = e.what(),
                .allowRetry = true
            });
        } catch (const std::exception &e) {
            log::error("Network initialization failed (std): {}", {}, e.what());
            bus::event::publish(event::NetworkInitFailedEvent{
                .reason = e.what(),
                .allowRetry = true
            });
        }
    }

    /**
     * @brief Subscribe to network-related events.
     */
    void subscribeToNetworkEvents() {
        // Handle network initialization failure
        bus::event::subscribe<event::NetworkInitFailedEvent>([](const event::NetworkInitFailedEvent &e) {
            log::info("NetworkInitFailedEvent received: {}", {}, e.reason);
            showNetworkErrorDialog(e.reason);
        });

        // Handle network retry request
        bus::event::subscribe<event::NetworkRetryRequestEvent>([](const event::NetworkRetryRequestEvent &) {
            log::info("NetworkRetryRequestEvent received, retrying network initialization...");
            
            // Clear recovery mode
            networkErrorRecoveryMode = false;
            
            // Show loading status
            bus::event::publish(event::ShowLoadingEvent(neko::ui::LoadingMsg{
                .type = neko::ui::LoadingMsg::Type::OnlyRaw,
                .process = lang::tr(lang::keys::network::category, lang::keys::network::retrying, "Retrying connection...")
            }));
            bus::event::publish(event::CurrentPageChangeEvent{.page = ui::Page::loading});
            
            // Retry network initialization in background thread
            bus::thread::submit([]() {
                auto retryFuture = app::init::retryNetworkInit();
                handleNetworkInitCompletion(retryFuture);
            });
        });

        // Handle when user closes settings page after network error
        bus::event::subscribe<event::NetworkSettingsClosedEvent>([](const event::NetworkSettingsClosedEvent &) {
            log::info("NetworkSettingsClosedEvent received, networkErrorRecoveryMode={}", {}, networkErrorRecoveryMode ? "true" : "false");
            if (networkErrorRecoveryMode && !lastNetworkErrorReason.empty()) {
                // Switch back to loading page and show error dialog again
                bus::event::publish(event::CurrentPageChangeEvent{.page = ui::Page::loading});
                showNetworkErrorDialog(lastNetworkErrorReason);
            } else {
                // Normal close - go to home page
                bus::event::publish(event::CurrentPageChangeEvent{.page = ui::Page::home});
            }
        });
    }
} // anonymous namespace

int main(int argc, char *argv[]) {
    try {

        // Initialize Application
        QApplication qtApp(argc, argv);
        auto networkReady = app::init::initialize();
        auto runingInfo = app::run();

        log::info("main: app::run complete");

        auto cfg = bus::config::getClientConfig();
        log::info("main: config loaded");
        auto themeOpt = ui::themeio::loadThemeByName(cfg.style.theme, lc::ThemeFolderName.data());
        log::info("main: theme loaded {}", {}, cfg.style.theme);
        ui::setCurrentTheme(themeOpt.value_or(ui::lightTheme));
        log::info("main: creating NekoWindow");
        ui::window::NekoWindow window(cfg);
        log::info("main: NekoWindow constructed");
        ui::UiEventDispatcher::setNekoWindow(&window);
        window.show();
        log::info("main: window shown");

        // Initialize BGM system from JSON config
        {
            std::string bgmConfigPath = system::workPath() + "/bgm.json";
            try {
                core::BgmConfig bgmConfig = core::loadBgmConfigFromJson(bgmConfigPath);
                // Override enabled and volume from client config
                bgmConfig.enabled = cfg.other.bgmEnabled;
                bgmConfig.masterVolume = cfg.other.bgmVolume;
                core::getBgmManager().initialize(bgmConfig);
                core::subscribeBgmToProcessEvents();
                // Subscribe LogFileWatcher to start watching Minecraft logs when game launches
                core::subscribeLogWatcherToProcessEvents();
                log::info("main: BGM system initialized from {}", {}, bgmConfigPath);
            } catch (const std::exception &e) {
                log::warn("main: Failed to load BGM config, BGM disabled: {}", {}, e.what());
            }
        }

        // Subscribe to network events before handling network initialization
        subscribeToNetworkEvents();

        // Handle network initialization in background thread
        bus::thread::submit([networkReady = std::move(networkReady)]() mutable {
            handleNetworkInitCompletion(networkReady);
        });

        // Start Qt event loop
        qtApp.exec();

        app::shutdown();
        runingInfo.eventLoopFuture.get();
        ui::UiEventDispatcher::clearNekoWindow();

        {
            auto cfg = bus::config::getClientConfig();
            if (cfg.dev.enable && cfg.dev.showLogViewer) {
                ui::window::LogViewerWindow logWindow(QString::fromStdString(system::workPath()) + "/logs/new-debug.log");
                logWindow.show();
                auto code = qtApp.exec();
                core::crash::markCleanShutdown();
                return code;
            }
        }

        core::crash::markCleanShutdown();
        return 0;
    } catch (const ex::Exception &e) {
        log::error("Unhandled Exception: " + std::string(e.what()));
    } catch (const std::exception &e) {
        log::error("Unexpected error: " + std::string(e.what()));
    }

    return 1;
}