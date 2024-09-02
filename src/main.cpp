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
        w.show();

        auto hintFunc = [=, &w](const ui::hintMsg &m) { emit w.showHintD(m); };
        it.get();

        exec::getThreadObj().enqueue([=] {
            neko::autoUpdate(hintFunc);
        });

        return app.exec();
    } catch (const std::exception &) {
    } catch (...) {
    }
}