#include "autoinit.hpp"
#include "core.hpp"
#include "mainwindow.hpp"
#include "logviewer.hpp"
#include <iostream>

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>

#include <QtGui/QGuiApplication>

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);
        auto it = neko::autoInit(argc, argv);
        neko::ClientConfig config(exec::getConfigObj());
        ui::MainWindow w(config);
        w.showLoad({ui::loadMsg::OnlyRaw,
                    neko::info::translations(neko::info::lang.network.testtingNetwork)});
        w.show();

        auto hintFunc = [=, &w](const ui::hintMsg &m) { emit w.showHintD(m); };
        auto loadFunc = [=, &w](const ui::loadMsg &m) { emit w.showLoadD(m); };
        auto setLoadInfo = [=, &w](unsigned int val, const char *msg) {
            emit w.setLoadingValD(val);
            if (msg)
                emit w.setLoadingNowD(msg);
        };

        exec::getThreadObj().enqueue([=, &it, &w] {

            // check and auto install , if auto install faild , show hint and retry
            for (size_t i = 0; i < 5; ++i) {
                try {
                    neko::checkAndAutoInstall(config,hintFunc,loadFunc,setLoadInfo);
                    break;
                } catch (const nerr::Error &e) {

                    auto quitHint = [](bool) { QApplication::quit(); };
                    auto retryHint = [](bool check) { if (!check) QApplication::quit(); };
                    std::string msg = std::string(e.msg) + "\n" + neko::info::translations((i == 4)? neko::info::lang.error.clickToQuit : neko::info::lang.error.clickToRetry);
                    hintFunc({neko::info::translations(neko::info::lang.title.error), msg, "", (i == 4) ? 2 : 1, ((i == 4) ? quitHint : retryHint)});
                }
            }
            // wait for testing host
            it.get();
            if (neko::autoUpdate(hintFunc, loadFunc, setLoadInfo) == neko::State::over) {
                emit w.showPageD(ui::MainWindow::pageState::index);
            } else {
                QApplication::quit();
            }
        });

        app.exec();

        // If execution reaches this point, it means the program has exited.
        if (config.dev.enable && config.dev.debug)
        {
            LogViewer logViewer(QString::fromStdString(neko::info::workPath() + "/logs/new-debug.log"));
            logViewer.show();
            app.exec();
        }
    } catch (const nerr::Error &e) {
        nlog::Err(FI, LI, "main : unexpected not catch nerr exception , msg : %s", e.what());
    } catch (const std::exception &e) {
        nlog::Err(FI, LI, "main : unexpected not catch std exception , what : %s", e.what());
    } catch (...) {
        nlog::Err(FI, LI, "main : unexpected not catch unknown exception");
    }
}