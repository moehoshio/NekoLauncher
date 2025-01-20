# Neko Launcher

Neko Launcher (NekoLc or NeLC) is a modern, cross-platform, multi-language supported auto-updating launcher solution.  
It can launch any target you want, and the current template can successfully launch Minecraft for Java.  
It includes automatic content updates and self-updates (both your content and Neko Core itself), as well as automatic installation of your content.  
If you're still searching for an auto-update solution, or if you're troubled by automating updates (users don't know how to operate, lack of automated update management solutions...), then try it.  
The project is still in development, and any constructive ideas are welcome.  
Preview :  
![img](res/img/img1.png)

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
Theoretically Qt5.6.0 ? (i dont know) (6.6 is the tested version)  
Theoretically Cmake3.20 or above (3.29 is the tested version)  
libcurl 7.0.0 or above (8.1 is the tested version)  
openssl 3.0.0 or above (3.4 is the tested version)  

Already included in the project dependencies:  

simpleIni 4.20  
nlohmann/json 3.11.3  
loguru 2.1  
threadpool for 2021-3507796

After satisfying the prerequisites, continue:

```shell
git clone https://github.com/moehoshio/NekoLauncher.git && cd NekoLauncher
cp CmakeListsCopy.txt CmakeLists.txt

# fill path for your
cmake . -B./build -DCMAKE_PREFIX_PATH="qt path and package" -DLIBRARY_DIRS="package path(opt)" -DQt6="qt path(opt)" -DCMAKE_BUILD_TYPE="Debug or Release (opt)"

cmake --build ./build
```

If you want to use this project, you need to modify a few things:  
Using our provided template method: modify the launcherMode variable at the top of `src/include/nekodefine.hpp`.  
Or modify the provided template.  
If you want complete custom logic, you can write your own function and call it in the `launcher` function, which is in `src/indclude/core.hpp`.  
Additionally, you need to edit the content in `src/data/`.  
More detailed information can be found in the [Contribution and Customization](#contribution-and-customization) section.

After completing these steps, you'll need to deploy your server. A simple example can be found at:  
[server.md](doc/server.md).

### Contribution and Customization

[dev.md](doc/dev.md)
