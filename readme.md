# Neko Launcher

Note: We are currently refactoring the project and it is temporarily unavailable. This will take some time, so please stay tuned!

[正體中文](readme_zh_hant.md) | [简体中文](readme_zh_hans.md) | [English](readme.md)  
Neko Launcher (NekoLc) is a modern, cross-platform, multi-language supported auto-updating launcher solution.  
It can launch any target you want, and the current template can successfully launch Minecraft for Java.  
It includes automatic content updates and self-updates (both your content and Neko Core itself), as well as automatic installation of your content.  
If you're still searching for an auto-update solution, or if you're troubled by automating updates (users don't know how to operate, lack of automated update management solutions...), then try it.  
The project is still in development, and any constructive ideas are welcome.  
Preview :  
![img](resource/img/readme1.png)  
Gif:  
![img2](resource/img/readme2.gif)  

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

## Documentation

For more detailed information, please refer to:
[dev.md](doc/dev.md)

After completing these steps, you'll need to deploy your server components:
[Server](https://github.com/moehoshio/NekoLcServer)  
