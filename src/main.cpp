#include "autoinit.h"
#include "core.h"
#include "mainwindow.h"
#include <iostream>

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>

#include <QtGui/QGuiApplication>

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);
        auto it = neko::autoInit(argc, argv);
        neko::Config config(exec::getConfigObj());
        ui::MainWindow w(config);
        w.showLoad({ui::loadMsg::OnlyRaw,
                    "testing network connect..."});
        w.show();

        auto hintFunc = [=, &w](const ui::hintMsg &m) { emit w.showHintD(m); };
        auto loadFunc = [=, &w](const ui::loadMsg &m) { emit w.showLoadD(m); };
        auto setLoadInfo = [=, &w](unsigned int val, const char *msg) {
            emit w.setLoadingValD(val);
            if (msg)
                emit w.setLoadingNowD(msg);
        };

        exec::getThreadObj().enqueue([=, &it, &w] {
            it.get();
            if (neko::autoUpdate(hintFunc, loadFunc, setLoadInfo) == neko::State::over) {
                emit w.showPageD(ui::MainWindow::pageState::index);
            }
        });

        return app.exec();
    } catch (const std::exception &) {
    } catch (...) {
    }
}