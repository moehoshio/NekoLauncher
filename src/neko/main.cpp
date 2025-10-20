#include <neko/app/app.hpp>
#include <neko/app/appinit.hpp>

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtGui/QGuiApplication>

int main(int argc, char *argv[]) {

    using namespace neko;

    QApplication app(argc, argv);
    QWidget w;
    w.resize(800, 600);
    w.setWindowTitle("NekoLauncher");
    w.show();
    app::init::initialize();
    auto runInfo = app::run();

    return app.exec();

    // try {

    //     // Initialization
        
    //     auto it = core::app::init::initialize();

    //     auto running = core::app::run();
    //     ClientConfig config = bus::config::getClientConfig();

    //     bus::thread::submit([=, &it, &w] {
    //         // check and auto install
    //         constexpr uint64 maxRetry = 5;
    //         for (uint64 i = 0; i < maxRetry; ++i) {
    //             try {
    //                 checkAndAutoInstall(config, hintFunc, loadFunc, setLoadInfo);
    //                 break;
    //             } catch (const ex::Exception &e) {

    //                 auto quitHint = [](bool) { core::app::quit(); };
    //                 auto retryHint = [](bool check) { if (!check) core::app::quit(); };

    //                 std::string msg = std::string(e.msg) + "\n" + info::tr((i == 4) ? info::lang.error.clickToQuit : info::lang.error.clickToRetry);
    //                 hintFunc({info::tr(info::lang.title.error), msg, "", (i == 4) ? 2 : 1, ((i == 4) ? quitHint : retryHint)});
    //             }
    //         }

    //         // wait for testing host
    //         it.get();

    //         // If the update is complete or there are no updates, show the homepage
    //         if (autoUpdate(hintFunc, loadFunc, setLoadInfo) == State::Completed) {
    //             emit w.showPageD(ui::NekoWindow::pageState::home);
    //         } else {
    //             QApplication::quit();
    //         }
    //     });

    //     // The main thread enters the event loop
    //     running.mainThreadRunLoopFunction();

    //     // If execution reaches this point, it means the program has exited.
    //     if (config.dev.enable && config.dev.debug) {
    //         ui::LogViewer logViewer(QString::fromStdString(info::workPath() + "/logs/new-debug.log"));
    //         logViewer.setWindowTitle("NekoLauncher Developer Debug Log");
    //         logViewer.show();
    //         running.mainThreadRunLoopFunction();
    //     }

    //     return 0;

    // } catch (const ex::Exception &e) {
    //     log::error({}, "main : unexpected not catch neko exception , msg : {}", e.what());
    // } catch (const std::exception &e) {
    //     log::error({}, "main : unexpected not catch std exception , what : {}", e.what());
    // } catch (...) {
    //     log::error({}, "main : unexpected not catch unknown exception");
    // }
}