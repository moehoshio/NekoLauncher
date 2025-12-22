# Neko Launcher

Note: We are refactoring the project and parts may be temporarily unavailable. Thanks for your patience.

[正體中文](readme_zh_hant.md) | [简体中文](readme_zh_hans.md) | [English](readme.md)  

Neko Launcher (NekoLc) is a modern, cross-platform, multi-language launcher with built-in auto-update. It can launch arbitrary targets; the current template ships with Minecraft: Java Edition support. It can update both your content and Neko Core, install assets automatically, and present a customizable UI.

Preview:  
![img](resource/img/readme1.png)  
![img2](resource/img/readme2.png)  

## Features

- Auto-update pipeline for both the launcher and your distributed content
- Cross-platform Qt UI with theme, font, and language customization
- Template for launching Minecraft: Java Edition; extensible to other targets
- Built-in dialogs for notices, inputs, and progress/loading states

## Requirements

- CMake 3.20+ and a C++20 compiler (MSVC/Clang/GCC)
- Qt 6 (Widgets + Gui + Core); Qt 5.15+ may work but is untested
- Git (for submodules) and internet access for dependency fetch during configure

## Quick Start (desktop)

```powershell
# Clone
git clone https://github.com/moehoshio/NekoLauncher.git
cd NekoLauncher

# Configure
cmake -B build -DNEKO_LC_LIBRARY_PATH="<Qt_path>;<Package_path>" -DNEKO_LC_STATIC_LINK=ON -S .

# Build (Debug|Release)
cmake --build build --config Release

# Run
./build/Release/NekoLc.exe
```

For other toolchains, replace the generator/commands accordingly. Set `NEKO_LC_LIBRARY_PATH` if Qt is not auto-detected.

## Configuration

- `config.ini.example` shows available options; copy it to `config.ini` and adjust paths, background, and update endpoints.
- Language files live in `lang/`; images live in `resource/img/`.

## Contributing

- Templates: additional launch targets beyond Minecraft
- QoL: drag-and-drop layout customization, optional music playback, more tests

Feel free to open issues or PRs. See the developer notes in `doc/dev.md`.

## Documentation

- Developer guide: [doc/dev.md](doc/dev.md)
- Server side: [NekoLcServer](https://github.com/moehoshio/NekoLcServer)
