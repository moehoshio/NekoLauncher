#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#endif // _WIN32

#include <iostream>
#include "neko/app/appinit.hpp"

using namespace neko;

int main(){
    app::init::initialize();
    
    return 0;
}