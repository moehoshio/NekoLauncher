#include "autoinit.h"
#include "mainwindow.h"


#include <iostream>


#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>


#include <QtGui/QGuiApplication>


int main(int argc, char *argv[]){
    try
    {
        QApplication app(argc, argv);
        neko::autoInit(argc, argv);
        ui::MainWindow w(exec::getConfigObj(),nullptr);
        w.show();
        app.exec();
        nlog::Info(FI,LI,"%s : Exit",FN);
        return 0;
    }
    catch(const std::exception&) {
        
    } catch(...){

    }
    
}