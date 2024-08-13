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
        neko::autoInit(argc, argv);
        ui::MainWindow w(exec::getConfigObj(), nullptr);
        w.show();

        neko::core c;
        c.autoUpdate(&w);

        return app.exec();
    } catch (const std::exception &) {
    } catch (...) {
    }
}