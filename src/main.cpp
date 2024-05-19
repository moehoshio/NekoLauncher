#include "autoinit.h"
#include "mainwindow.h"

#include <iostream>


#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>


#include <QtGui/QGuiApplication>


#include <curl/curl.h>


int main(int argc, char *argv[]){
    try
    {
        QApplication app(argc, argv);
        neko::autoInit(argc, argv);
        ui::MainWindow w(exec::getConfigObj());
        w.show();
        return app.exec();
    }
    catch(const std::exception&) {
        
    } catch(...){

    }
    
}