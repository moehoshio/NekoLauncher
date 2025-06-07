# Neko Launcher

[正體中文](readme_zh_hant.md) | [简体中文](readme_zh_hans.md) | [English](readme.md)  
Neko Launcher (NekoLc) is a modern, cross-platform, multi-language supported auto-updating launcher solution.  
It can launch any target you want, and the current template can successfully launch Minecraft for Java.  
It includes automatic content updates and self-updates (both your content and Neko Core itself), as well as automatic installation of your content.  
If you're still searching for an auto-update solution, or if you're troubled by automating updates (users don't know how to operate, lack of automated update management solutions...), then try it.  
The project is still in development, and any constructive ideas are welcome.  
Preview :  
![img](resource/img/example1.png)  
gif:  
![img2](resource/img/example2.gif)  

## Become Contributors

Currently, the following aspects are still incomplete:

- **UI**: Transition animations, theme settings/style customization, and better art design.

- **Others**: more template examples, and Considering drag-and-drop mechanism for visual customization of the interface, maybe even adding music playback functionality?

Of course, I would be very grateful for any help you can provide or any ideas to make the project more robust. If you are interested, feel free to submit issues.

## Supported Platforms

I use Qt6 to build the GUI, However, it shouldn't use any special APIs.  
You should be able to use Qt5 to build:  
Ubuntu 18.04  
macOS 10.13  
Windows 7  

Our main program also uses cross-platform methods, making it supported on most platforms.  
In other words, you can also forgo the GUI; its core should still run properly.

## Build

prerequisites:  
std >= c++20  
Qt5.6.0 ? (6.6 & 6.8 is the tested version)  
cmake 3.20 or above (3.29 is the tested version)  
libcurl 8.1 is the tested version  
openssl 3.4 is the tested version  
boost.process 1.86.0 is the testd version  
boost.stacktrace 1.86.0 is the testd version  

Already included in the project dependencies:  

simpleIni 4.20  
nlohmann/json 3.11.3  
loguru 2.1  
threadpool for 2021-3507796

After satisfying the prerequisites, continue:

```shell
git clone https://github.com/moehoshio/NekoLauncher.git && cd NekoLauncher

# Unix
chmod +x ./build.sh && ./build.sh

# Windows
build.bat

# Or build manually

cp CmakeListsCopy.txt CmakeLists.txt

# fill path for your
cmake . -B./build -DLIBRARY_DIRS="Package path (e.g /to/path/vcpkg/installed/x64-windows)" -DQt6_DIR="Qt path (e.g /to/path/Qt/6.6.1/msvc_2019_64)" -DCMAKE_BUILD_TYPE="Debug or Release(Default)"

cmake --build ./build --config Release

cmake --install ./build --config Release --prefix ~/NekoLc
```

In addition, the working directory needs to contain the following files:  
Folders required: lang/, img/  
Files required: config.ini, cacert.pem, img/loading.gif, img/ico.png, lang/en.json. (This does not include your dynamic link libraries)

### Contribution and Customization

If you want to use this project, you might need to make modifications to be able to use it. Because it is highly customized.  

If you simply want to keep the content updated automatically, it shouldn't be too difficult.

Using our provided template method: modify the `launcherMode` variable at the top of `src/include/neko/schema/nekodefine.hpp`.  

We currently provide a template for Minecraft Java; change the variable to `"minecraft"`.  

If you want complete custom logic: write your function anywhere and call it in the `launcher` function located in `src/include/neko/core/launcher.hpp`.  

You might need to edit the version number and server links in `src/include/neko/schema/nekodefine.hpp`.  

For example, if you want to launch `example.exe`:  

```cpp
inline void launcher(std::function<void(const ui::hintMsg &)> hintFunc,std::function<void()> onStart, std::function<void(int)> onExit) {
    // Write your content

    // shell: example.exe -args ...
    std::string command = std::string("example.exe ") + "-you might need some parameters " + "args...";
    launcherProcess(command,onStart,onExit);
}
```

For more detailed information, please refer to:
[dev.md](doc/dev.md)

After completing these steps, you'll need to deploy your server. A simple example can be found at:  
[server.md](doc/server.md).
