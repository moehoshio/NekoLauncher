# Developer documentation
 
[Project layout](#project-layout)  
[Style guide](#style-guide)  
[Bus](#bus)  
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

## Bus

NekoBus is essentially a singleton-wrapped bus that lets you access and manipulate the bus object via static methods without directly calling the singleton. This reduces global state and makes the code easier to test and maintain.

## Tools

`Update`: Update tool, which should be distributed with the client. It is used to perform Core updates. It replaces the NekoCore files and then restarts NekoLc.

```shell
cmake --build build --target NekoLcUpdateTool --config Release
```

## Links

- API: <https://github.com/moehoshio/NekoLcApi/wiki>
- Server: <https://github.com/moehoshio/NekoLcServer>
