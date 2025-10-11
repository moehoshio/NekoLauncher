# Developer documentation

[Style guide](#style-guide)  
[Server](https://github.com/moehoshio/NekoLcServer)

## Style Guide

1. Use **CamelCase** naming convention: For **type definitions**, use an uppercase initial letter, variables and functions use a lowercase initial letter. Specifically:

```cpp
void func();
std::string myName;
struct Name {} name;
namespace neko {};
```

2. Use **4-space** indentation per level. Source files should use **UTF-8 encoding**.

3. In this project, use double quotes `""` to include headers from within the project, and angle brackets `<>` for external headers. This helps quickly distinguish between internal and external headers.

4. Organize `#include` statements into categories, such as:

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

5. Use `#pragma once` at the top of header files. Avoid introducing `using namespace` in the global scope within header files.

6. Always include curly braces `{}` for `for` or `while` loops.
