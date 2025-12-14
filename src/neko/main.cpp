
#include <neko/log/nlog.hpp>

#include "neko/app/app.hpp"
#include "neko/app/appinit.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/core/install.hpp"
#include "neko/core/update.hpp"

#include "neko/ui/themeIO.hpp"
#include "neko/ui/uiEventDispatcher.hpp"
#include "neko/ui/windows/nekoWindow.hpp"
#include "neko/ui/windows/logViewerWindow.hpp"

#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>

#include <iostream>
#include <thread>

using namespace neko;

int main(int argc, char *argv[]) {
    try {

        // Initialize Application
        QApplication qtApp(argc, argv);
        app::init::initialize();
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
        
        bus::thread::submit([](){
            const bool installed = core::install::autoInstall();
            if (!installed) {
                core::update::autoUpdate();
            }

            bus::event::publish(event::CurrentPageChangeEvent{
                .page = ui::Page::home
            });
        });

        // Start Qt event loop
        qtApp.exec();

        // Stop event loop and worker threads before waiting on the loop future to prevent deadlock
        app::shutdown();
        runingInfo.eventLoopFuture.get();
        ui::UiEventDispatcher::clearNekoWindow();

        {
            auto cfg = bus::config::getClientConfig();
            if (cfg.dev.enable && cfg.dev.showLogViewer) {
                ui::window::LogViewerWindow logWindow(QString::fromStdString(system::workPath()) + "/logs/new-debug.log");
                logWindow.show();
                return qtApp.exec();
            }
        }
    } catch (const ex::Exception &e) {
        log::error("Unhandled Exception: " + std::string(e.what()));
    } catch (const std::exception &e) {
        log::error("Unexpected error: " + std::string(e.what()));
    }
}