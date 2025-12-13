# Developer documentation
 
[Project layout](#project-layout)  
[Style guide](#style-guide)  
[Runtime bundle](#runtime-bundle)  
[App module](#app-module)  
[Core module](#core-module)  
[Minecraft module](#minecraft-module)  
[UI module](#ui-module)  
[Bus](#bus)  
[FAQ / Troubleshooting](#faq--troubleshooting)  
[Links](#links)  
[Tools](#tools)

## Configuration

- Copy `config.ini.example` to `config.ini` and adjust background, window size, update endpoints, and paths.
- Language packs live in `lang/`; assets in `resource/img/`.

### Testing

If the build generates tests, you can run them via CTest:

```powershell
ctest --test-dir build --build-config Release
```

## Runtime bundle

What a complete runtime directory should contain:

- `config.ini` (optional; auto-generated on first run if missing).
- `langs/` with language JSON files. Copy from the repository `langs/` into your working directory so translations load.
- `themes/` containing theme JSON (sample themes included).
- `img/loading.gif` for the loading page. You can copy the provided asset from `resource/img/` (free for commercial use) or replace with your own.
- If you build with dynamic linking, copy the required runtime libraries (`.dll` on Windows) alongside the executable.

## Project layout

- `src/` — launcher code (UI pages, windows, dialogs, logic)
- `include/` — public headers
- `lang/` — localization JSON files
- `resource/` — images/icons used by the UI
- `tools/` — helper scripts/tools
- `doc/` — documentation

## Style Guide

1. Use **CamelCase** naming convention: For **type definitions**, use an uppercase initial letter, variables and functions use a lowercase initial letter. Specifically:

```cpp
void func();
std::string myName;
struct Name {} name;
namespace neko {};
```

1. Use **4-space** indentation per level. Source files should use **UTF-8 encoding**.

1. In this project, use double quotes `""` to include headers from within the project, and angle brackets `<>` for external headers. This helps quickly distinguish between internal and external headers.

1. Organize `#include` statements into categories, such as:

```cpp
// Neko Modules
#include <neko/network/network.hpp>

// NekoLc project
#include "neko/function/lang.hpp"
#include "neko/function/info.hpp"


// STL
#include <string>
#include <iostream>

// Qt Modules

#include <QtWidgets/QLabel>
// Qt small category: Button-related
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>

// Other
#include <curl/curl.h>
```

1. Use `#pragma once` at the top of header files. Avoid introducing `using namespace` in the global scope within header files.

1. Always include curly braces `{}` for `for` or `while` loops.

## App module

Lifecycle, configuration, localization, and app metadata.

- Key headers: `app.hpp`, `appinit.hpp`, `clientConfig.hpp`, `configManager.hpp`, `lang.hpp`, `appinfo.hpp`, `api.hpp`, `nekoLc.hpp`.
- Boot and config access:

```cpp
neko::app::init::initialize();                 // logging, language, config, network bootstrap
auto cfg = neko::bus::config::getClientConfig();
neko::bus::config::updateClientConfig([](neko::ClientConfig &c) {
	c.main.lang = "en";
});
neko::bus::config::save(neko::app::getConfigFileName());

const auto title = neko::lang::tr(neko::lang::keys::maintenance::category,
								  neko::lang::keys::maintenance::title);
```

- Config is INI-based (SimpleIni). Localizations are UTF-8 JSON; all access is thread-safe through the config bus.

## Core module

Launcher services for update/maintenance, process launch, remote config, feedback/auth, and poster downloads.

- Key headers: `update.hpp`, `maintenance.hpp`, `launcher.hpp`, `launcherProcess.hpp`, `remoteConfig.hpp`, `auth.hpp`, `feedback.hpp`, `downloadPoster.hpp`.
- Typical update flow:

```cpp
// One-shot auto update (publishes UI progress events, may exit on maintenance)
neko::core::update::autoUpdate();

// Manual steps
if (auto raw = neko::core::update::checkUpdate()) {
	auto resp = neko::core::update::parseUpdate(*raw);
	neko::core::update::update(resp);
}
```

- Update and maintenance emit bus events consumed by the UI loading page and notice dialogs.

## Minecraft module

Minecraft-specific auth, download sources, installation, and launch wiring.

- Key headers: `installMinecraft.hpp`, `downloadSource.hpp`, `launcherMinecraft.hpp`, `authMinecraft.hpp`, `minecraftSubscribe.hpp`.
- Install/download (blocking; keep off the UI thread):

```cpp
// Download all assets for a version into the target directory
neko::minecraft::setupMinecraftDownloads(neko::minecraft::DownloadSource::Official,
										 "1.20.1",
										 versionJson,
										 "./.minecraft");

// Or do a full install with defaults
neko::minecraft::installMinecraft("./.minecraft", "1.20.1");
```

- Build and launch:

```cpp
neko::minecraft::LauncherMinecraftConfig launchCfg{
	.minecraftFolder = "./.minecraft",
	.targetVersion = "1.20.1",
	.javaPath = "java",
	.playerName = "Steve",
	.uuid = "...",
	.accessToken = "...",
};

auto cmd = neko::minecraft::getLauncherMinecraftCommand(launchCfg);
// Or execute and hook callbacks
neko::minecraft::launcherMinecraft(clientConfig,
	[](){ /* onStart */ },
	[](int code){ /* onExit */ });
```

- Authlib injector fields live in `LauncherMinecraftConfig::authlib`. Exceptions surface for network/file/parse errors; progress should be surfaced via bus events.

## UI module

Goal: emit UI actions from any thread/process code while keeping UI work on the Qt main thread. Wire it once, then publish events.

### Setup

- During UI startup, point the dispatcher at the main window and subscribe the UI to the bus:

```cpp
ui::UiEventDispatcher::setNekoWindow(&window);
ui::subscribeToUiEvent();
```

- If you hold a direct `NekoWindow*`, you can also call the instance methods below instead of publishing events.

### Notice dialog (`ShowNotice`)

- Message type: `neko::ui::NoticeMsg` (`title`, `message`, optional `posterPath`, `buttonText`, `callback`, `autoClose`, `defaultButtonIndex`).
- Fire via bus:

```cpp
neko::ui::NoticeMsg notice{
	.title = "Update finished",
	.message = "All files are up to date.",
	.buttonText = {"OK", "Open folder"},
	.callback = [](neko::uint32 index) {
		if (index == 1) { /* open folder */ }
	}
};
bus::event::publish<neko::event::ShowNoticeEvent>(notice);
```

- Default behavior: if `buttonText` is empty, a single OK button is shown. `autoClose` (ms) will trigger the callback with `defaultButtonIndex`.

### Input dialog (`ShowInput` / `HideInput` / getLines)

- Message type: `neko::ui::InputMsg` (`title`, `message`, optional `posterPath`, `lineText` placeholders, `callback<bool confirmed>`).
- Show and collect input:

```cpp
neko::ui::InputMsg login{
	.title = "Login",
	.message = "Enter account and password",
	.lineText = {"Account", "Password"},
	.callback = [](bool confirmed) {
		auto *win = neko::ui::UiEventDispatcher::getNekoWindow();
		if (!confirmed || !win) { return; }

		const auto lines = win->getLines(); // mirrors input order
		win->hideInput();                    // close the dialog manually
		// use lines[0] (account), lines[1] (password)
	}
};
bus::event::publish<neko::event::ShowInputEvent>(login);
```

- Notes: placeholders auto-fill if `lineText` is empty. The dialog does not auto-hide; call `HideInputEvent` or `NekoWindow::hideInput()` after you process the data.

### Loading page (`ShowLoading` / progress updates)

- Message type: `neko::ui::LoadingMsg` with `type` (`OnlyRaw`, `Text`, `Progress`, `All`), `process` text, optional `h1`/`h2`/`message`, `posterPath`, `loadingIconPath`, `speed`, `progressVal`, `progressMax`.
- Show and drive progress:

```cpp
bus::event::publish<neko::event::ShowLoadingEvent>(neko::ui::LoadingMsg{
	.type = neko::ui::LoadingMsg::Type::All,
	.h1 = "Preparing",
	.h2 = "Step 1/3",
	.message = "Fetching metadata...",
	.progressVal = 0,
	.progressMax = 100,
});

bus::event::publish<neko::event::LoadingStatusChangedEvent>({.statusMessage = "Downloading..."});
bus::event::publish<neko::event::LoadingValueChangedEvent>({.progressValue = 40});
```

- Direct calls if you have the window: `window.showLoading(msg)`, `window.setLoadingValueD(value)`, `window.setLoadingStatusD(text)`.
- The loading GIF defaults to `img/loading.gif`; override with `loadingIconPath` and adjust animation speed via `speed` (percent).

### Page switching and reset helpers

- Switch pages through the bus: `bus::event::publish<neko::event::CurrentPageChangeEvent>({.page = ui::Page::home});`
- To return to home after launch start: publish `LaunchStartedEvent` (UI subscribes automatically).
- Notice dialog cleanup helpers exist (`resetNoticeStateD`, `resetNoticeButtonsD`) but are wired internally by `NekoWindow`.

## Bus

NekoBus is essentially a singleton-wrapped bus that lets you access and manipulate the bus object via static methods without directly calling the singleton. This reduces global state and makes the code easier to test and maintain.

## FAQ / Troubleshooting

- CMake cannot find Qt/Boost

	- Set `NEKO_LC_LIBRARY_PATH` to the CMake package roots (semicolon-separated on Windows). Example:

	```powershell
	$env:NEKO_LC_LIBRARY_PATH = "C:/Qt/6.6.3/mingw_64/lib/cmake;C:/local/boost_1_83_0/lib/cmake/Boost"
	cmake -S . -B build -DNEKO_LC_LIBRARY_PATH="$env:NEKO_LC_LIBRARY_PATH"
	cmake --build build --config Release
	```

	- Ensure the path points to the directories containing `Qt6Config.cmake` or `BoostConfig.cmake` / `boost-config.cmake`. If using vcpkg, use its installed package cmake folders.

- Qt GIF loading looks broken in loading page

	- Verify `img/loading.gif` exists in the runtime working directory; override with `loadingIconPath` in `LoadingMsg` if you deploy a different asset.

- Input dialog does not close after callback

	- The dialog stays open by design; call `HideInputEvent` or `NekoWindow::hideInput()` after you consume `getLines()`.

## Tools

`Update`: Update tool, which should be distributed with the client. It is used to perform Core updates. It replaces the NekoCore files and then restarts NekoLc.

```shell
cmake --build build --target NekoLcUpdateTool --config Release
```

## Links

- API: <https://github.com/moehoshio/NekoLcApi/wiki>
- Server: <https://github.com/moehoshio/NekoLcServer>
