#include "autoinit.hpp"
#include "core.hpp"
#include "mainwindow.hpp"
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
            for (size_t i = 0; i < 5; ++i) {
                try {
                    neko::checkAndAutoInstall(config);
                    break;
                } catch (const nerr::error &e) {
                    if (i == 4) {
                        hintFunc({neko::info::translations(neko::info::lang.title.error), e.msg, "", 1, [](bool) {
                                      QApplication::quit();
                                  }});
                    } else {
                        hintFunc({neko::info::translations(neko::info::lang.title.error), e.msg, "", 2, [](bool check) {
                                      if (!check) {
                                          QApplication::quit();
                                      }
                                  }});
                    }
                }
            }
            it.get();
            if (neko::autoUpdate(hintFunc, loadFunc, setLoadInfo) == neko::State::over) {
                emit w.showPageD(ui::MainWindow::pageState::index);
            }
        });

        return app.exec();
    } catch (const nerr::error &e) {
        nlog::Err(FI, LI, "main : unexpected not catch nerr exception , msg : %s", e.what());
    } catch (const std::exception &e) {
        nlog::Err(FI, LI, "main : unexpected not catch std exception , what : %s", e.what());
    } catch (...) {
        nlog::Err(FI, LI, "main : unexpected not catch unknown exception");
    }
    QApplication::quit();
}