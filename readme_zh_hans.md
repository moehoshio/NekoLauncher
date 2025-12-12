# Neko Launcher

提示：项目正在重构，部分功能可能暂时不可用，感谢耐心等待。

[正體中文](readme_zh_hant.md) | [简体中文](readme_zh_hans.md) | [English](readme.md)  

Neko Launcher（NekoLc）是一个现代化、跨平台、多语言的启动器，内置自动更新机制。它能启动任意目标；当前模板默认支持 Minecraft Java 版。启动器可同时更新你的内容与 Neko Core，本身也能自动安装资源，并提供可自定义的 UI。

预览：  
![img](resource/img/readme1.png)  
![img2](resource/img/readme2.png)  

## 功能

- 启动器与发布内容的自动更新流程
- 跨平台 Qt 界面，支持主题、字体与语言自定义
- 内置 Minecraft Java 版启动模板，可扩展到其他目标
- 内置通知、输入、进度/加载显示等对话框

## 系统需求

- CMake 3.20+、C++20 编译器（MSVC/Clang/GCC）
- Qt 6（Widgets + Gui + Core）；Qt 5.15+ 可能可用但未充分测试
- Git（含子模块）与可访问网络以在配置时抓取依赖

## 快速开始（桌面）

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

若使用其他工具链，请替换合适的生成器/命令。若未自动找到 Qt，请设置 `NEKO_LC_LIBRARY_PATH`。

## 配置

- `config.ini.example` 展示可用项；复制为 `config.ini` 后调整路径、背景与更新端点。
- 语言文件位于 `lang/`；图片资源位于 `resource/img/`。

## 贡献

- UI：转场动画、更丰富的主题/样式预设、更佳视觉设计
- 模板：扩展到更多启动目标（不止 Minecraft）
- 体验：拖放式布局自定义、可选的音乐播放、更多测试

欢迎提交 issue 或 PR，开发笔记见 `doc/dev.md`。

## 文档

- 开发者指南： [doc/dev.md](doc/dev.md)
- 服务器项目： [NekoLcServer](https://github.com/moehoshio/NekoLcServer)
