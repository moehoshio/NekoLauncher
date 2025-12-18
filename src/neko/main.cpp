
#include <neko/log/nlog.hpp>

#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/appinit.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/core/crashReporter.hpp"
#include "neko/core/install.hpp"
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

        bus::thread::submit([&networkReady]() {
            try {
                networkReady.get();
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