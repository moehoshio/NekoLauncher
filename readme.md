# Neko Launcher

Neko Launcher (NekoLc or NeLC) is a modern, cross-platform, multi-language supported auto-updating launcher solution.  
It can launch any target you want, and the current template can successfully launch Minecraft for Java.  
It includes automatic content updates and self-updates (both your content and Neko Core itself).  
If you're still searching for an auto-update solution, or if you're troubled by automating updates (users don't know how to operate, lack of automated update management solutions, issues hard to track...), then try it.  
This is also my first project learning C++ for nearly a year.  
The project is still in development, and any constructive ideas are welcome.  
Preview :  
![img](res/img/img1.png)

## Become Contributors

Currently, the following aspects are still incomplete:

- **UI**: Transition animations, theme settings/style customization, and better art design.

- **Others**: more template examples, and Considering drag-and-drop mechanism for visual customization of the interface, maybe even adding music playback functionality?

Of course, I would be very grateful for any help you can provide or any ideas to make the project more robust. If you are interested, feel free to submit issues.

## Supported Platforms

We use Qt6 to build the GUI. Its minimum requirements are:

- Windows 10 (H1809)
- macOS 11
- Linux (Ubuntu 20.04, OpenSuSE 15.4)

Our main program also uses cross-platform methods, making it supported on most platforms.  
In other words, if you change the GUI version or framework, it might support other platforms as well, more or less.
You can also forgo the GUI; its core should still run properly.

## Build

prerequisites:  
std >= c++20  
qt6  
cmake  
libcurl  
openssl  

Already included in the project dependencies:  

simpleIni  
nlohmann/json  
loguru  
threadpool  

After satisfying the prerequisites, continue:

```shell
git clone https://github.com/moehoshio/NekoLauncher.git && cd NekoLauncher
cp CmakeListsCopy.txt CmakeLists.txt

# fill path for your
cmake . -B./build -DCMAKE_PREFIX_PATH="package and qt path" -DLIBRARY_DIRS="package path this a opt" -DQt6="qt path" -DCMAKE_BUILD_TYPE="Debug or Release"

cmake --build ./build
```

If you want to use this project, you need to modify a few things:  
Using our provided template method: modify the launcherMode variable at the top of `src/include/core.h`.  
Or modify the provided template.  
If you want complete custom logic, you can write your own function and call it in the `launcher` function, which is also in core.h.  
Additionally, you need to edit the content in `src/data/`.  
More detailed information can be found in the [Contribution and Customization](#contribution-and-customization) section.

After completing these steps, you'll need to deploy your server. A simple example can be found at:  
[server.md](doc/server.md).

### Contribution and Customization

[dev.md](doc/dev.md)
