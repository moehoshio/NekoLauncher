# Neko Launcher

NekoL (Neko launcher or NekoLc) is a modern, customizable cross-platform launcher framework. It aims to lower the software usage barrier, making it user-friendly and straightforward to operate.  
It can launch any object, wrapping around anything, and includes self-updating and automatic software content updates.  
This is also my first project learning C++ for nearly a year. Through this project,learn more content and knowledge.

## Recruiting Contributors

If you are interested, feel free to submit issues.

## Supported Platforms

We use Qt6 to build the GUI. Its minimum requirements are:

- Windows 10 (H1809) x86_64
- macOS 11 x86_64, x86_64h, and arm64
- Linux (Ubuntu 20.04, OpenSuSE 15.4) x86_64
- iOS 14 armv8 (arm64)
- Android 8.0 (API level 26) armeabi-v7a, x86, arm64-v8, and x86_64

Our main program also uses cross-platform methods, making it supported on most platforms.  
In other words, if you change the GUI version or framework, it might support other platforms as well, more or less.

## Build

prerequisites:  
std >= c++20  
qt6  
cmake  
libcurl  

Already included in the project dependencies:  

simpleIni  
nlohmann/json  
loguru  
threadpool  
...

Clone code to the you host and :  

```shell
cp CmakeListsCopy.txt CmakeLists.txt
cmake ./build
cmake --build ./build
```

Custom include and library path in CMakeList.txt :  
INCLUDE_DIRS  
LIBRARY_DIRS  

```cmake
list(APPEND INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src/include
    /to/path/
)
```

CMAKE_PREFIX_PATH : qt path  
QtBin : qt bin path  

### Contribution and Customization

[dev.md](doc/dev.md)
