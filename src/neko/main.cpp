
#include <neko/log/nlog.hpp>

#include "neko/app/app.hpp"
#include "neko/app/appinit.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/core/update.hpp"

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

        auto cfg = bus::config::getClientConfig();
        if (cfg.style.theme == std::string("dark")) {
            ui::setCurrentTheme(ui::darkTheme);
        } else {
            ui::setCurrentTheme(ui::lightTheme);
        }
        ui::window::NekoWindow window(cfg);
        ui::UiEventDispatcher::setNekoWindow(&window);
        window.show();
        
        bus::thread::submit([](){
            core::update::autoUpdate();
            bus::event::publish(event::CurrentPageChangeEvent{
                .page = ui::Page::home
            });
        });

        // Start Qt event loop
        qtApp.exec();
        runingInfo.eventLoopFuture.get();
        app::shutdown();
        ui::UiEventDispatcher::clearNekoWindow();

        {
            auto cfg = bus::config::getClientConfig();
            if (cfg.dev.enable && cfg.dev.debug) {
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