#include "autoinit.h"


#include <iostream>

#include <curl/curl.h>
#include <QApplication>

int main(int argc, char *argv[]){
    try
    {
        QApplication app(argc, argv);
        neko::autoInit(argc, argv);
    }
    catch(const std::exception& e) {
        
    } catch(...){

    }
    
}