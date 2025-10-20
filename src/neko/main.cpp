#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/appinit.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/bus/threadBus.hpp"

#include "neko/core/update.hpp"

#include "neko/ui/windows/logViewerWindow.hpp"
#include "neko/ui/windows/nekoWindow.hpp"

#include <QtGui/QGuiApplication>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>

int main(int argc, char *argv[]) {

    using namespace neko;

    try {

        // Initialization
        QApplication app(argc, argv);
        app::init::initialize();
        auto runInfo = app::run();
        

        bus::thread::submit([=, &w] {
            core::update::autoUpdate()
        });

        neko::ui::NekoWindow w;
        w.show();

        // The main thread enters the event loop
        app.exec();

        // If execution reaches this point, it means the program has exited.
        neko::ClientConfig config = bus::config::getClientConfig();
        if (config.dev.enable && config.dev.debug) {
            ui::LogViewer logViewer(QString::fromStdString(system::workPath() + "/logs/new-debug.log"));
            logViewer.setWindowTitle("NekoLauncher Developer Debug Log");
            logViewer.show();
            return app.exec();
        }

        return 0;

    } catch (const ex::Exception &e) {
        log::error({}, std::string("main : unexpected not catch neko exception , msg : ") + e.what());
    } catch (const std::exception &e) {
        log::error({}, std::string("main : unexpected not catch std exception , what : ") + e.what());
    } catch (...) {
        log::error("main : unexpected not catch unknown exception");
    }
}