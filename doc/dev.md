# dev

## Style Guide

1. Clang Formatting

```json
"C_Cpp.clang_format_fallbackStyle": "{ 
    BasedOnStyle: LLVM, 
    UseTab: Never, 
    IndentWidth: 4, 
    TabWidth: 4, 
    BreakBeforeBraces: Attach, 
    AllowShortIfStatementsOnASingleLine: false, 
    IndentCaseLabels: true, 
    ColumnLimit: 0, 
    AccessModifierOffset: -4, 
    NamespaceIndentation: All, 
    FixNamespaceComments: true }"
```

2. **CamelCase** should be used for naming. Specifically:

```cpp
std::string myName;
void func();
```

3. Use **4-space** indentation per level. Source files should use **UTF-8 encoding** with **LF** line endings (not strictly enforced).

4. Organize `#include` statements into categories, such as:

```cpp
// Custom Headers
#include "exec.h"
#include "network.h"

// Standard Library
#include <string>
#include <iostream>

// Qt (or other third-party libraries)

// Qt large category
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
// Qt small category: Button-related
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>

// Other third-party libraries
#include <curl/curl.h>
```

5. Use `#pragma once` at the top of header files. Avoid introducing `using namespace` in the global scope within header files. Scoping should be applied most of the time.

6. For simple single-line `if` statements, it's recommended to omit extra braces for brevity:

```cpp
if (true)
    return true;
else
    return false;
```

### Module List

1. **Auto-init Function**:
    - Automatically configures all initial information.
    - Specifically, it will:
        - Correct the working directory
        - Load configuration
        - Initialize the info class
            - Read the user folder
            - Obtain a temporary folder
        - Set the log
        - Set thread numbers
        - Set thread names
        - Print configuration
        - Initialize networkBase
            - Load default proxy configuration
            - Set default user-agent
            - Configure default host
            - Set TLS settings
    
    Function prototype:

```cpp
inline void neko::autoInit(int argc, char *argv[])
```

2. **exec.h**:
    - Custom features and encapsulations are defined in this project:
    - Move operator overloading into the project's `neko` namespace.
    - Manage singleton objects:
        - **ThreadPool** 

            ```cpp
            neko::ThreadPool &exec::getThreadObj();
            ```

            Usage example:

            ```cpp
            auto it = exec::getThreadObj().enqueue(func, args);
            auto res = it.get();
            ```

        - **CSimpleIniA** (for config files)

            ```cpp
            CSimpleIniA &exec::getConfigObj();
            ```

            Usage example:

            ```cpp
                auto debug = exec::getConfigObj().GetValue("Section","key","default");
            ```

    - utility func
        - look the [exec.h](https://github.com/moehoshio/exec.h)

3. **network.h**: Inherits from `networkBase`.
    - **networkBase**:
        - Initialization function
        - Defines classes/structures:
            - **Config**: Configuration information
                - User-Agent (ua)
                - Proxy
                - Host
                - Protocol
                    - Includes a default configuration object:
                        - `static Dconfig`
            - **Api**: APIs, detailed in [server.md](server.md)
                - Members are `constexpr static const char *`
                - Includes a default configuration object:
                    - `constexpr static api`
        - **Opt Enum**: Enumeration for download options
            - `none`
            - `onlyRequest`
            - `downloadFile`
            - `postText`
            - `postFile`
            - `getSize`
            - `getContentType`
            - `getContent`
            - `getHeadContent`
        - **optMap Object** and Functions:

        ```cpp
        static std::unordered_map<Opt, std::string> optMap;
        ```

        ```cpp
        inline static auto optStr(Opt opt);
        ```

        ```cpp
        inline static auto strOpt(const std::string_view str);
        ```

        - **buildUrl**: Constructs a complete URL from a fixed or pre-concatenated path

        ```cpp
        template <typename T = std::string>
        constexpr static T buildUrl(const T &path, const T &host = Dconfig.host, const T &protocol = Dconfig.protocol);
        ```

        - **buildUrlP**: Constructs a complete URL with an indefinite number of path segments; protocol and host need to be manually specified.

        ```cpp
        template <typename T = std::string, typename... Paths>
        constexpr static T buildUrlP(const T &protocol, const T &host, Paths &&...paths);
        ```

        - **getSysProxy**: Retrieves system proxy information

        ```cpp
        template <typename T = std::string>
        static T getSysProxy();
        ```

        - Default callback functions:

        ```cpp
        static std::function<void()> defaultCallback;
        ```

        - Functions to handle retry logic and non-blocking execution are provided, such as:

            - **autoRetry**: Encapsulation function
                - Scope of use: same as `Do` function
                - If the maximum retry count is reached without obtaining the expected HTTP code, returns `false`.
                - Note: If resume is enabled, HTTP code 416 is also considered as completed.

                ```cpp
                inline bool autoRetry(Opt opt, autoRetryArgs &ra);
                ```

                - Overloaded version supports temporary arguments `(const autoRetryArgs &ra)`.

            - **autoRetryGet**: Encapsulation function
                - Scope of use: same as `get` function
                - If the maximum retry count is reached without obtaining the expected HTTP code, returns an empty object.
                - Note: If resume is enabled, HTTP code 416 is also considered as completed.

                ```cpp
                inline T autoRetryGet(Opt opt, autoRetryArgs &ra);
                ```

                - Overloaded version supports temporary arguments `(const autoRetryArgs &ra)`.

            - **nonBlockingDo**: Encapsulation function
                - Scope of use: same as `Do` function

                ```cpp
                inline auto nonBlockingDo(Opt opt, Args &args) -> std::future<void>;
                ```

            - **nonBlockingGet**: Encapsulation function
                - Scope of use: same as `get` function

                ```cpp
                inline auto nonBlockingGet(Opt opt, Args &args) -> std::future<T>;
                ```

            - **nonBlockingGetPtr**: Encapsulation function
                - Scope of use: same as `get` function

                ```cpp
                inline auto nonBlockingGetPtr(Opt opt, Args &args) -> std::future<T *>;
                ```

            - **Multi**: Encapsulation function
                - Scope of use: same as `Do` function

                ```cpp
                inline bool Multi(Opt opt, MultiArgs &ma);
                ```

                - Overloaded version supports temporary arguments `(const MultiArgs &ma)`.

4. **mainwindow.h**:
    - Inherits from `QMainWindow` for the main window.
    - Key interfaces:
        - **onUpdateDownloadPage** function
            - Displays the update page and information in the window.

            ```cpp
            void onUpdateDownloadPage(updateMsg m);
            ```

        - **showHint** function
            - Displays a hint window.
            - If `buttonType` is 1, a single confirmation button is displayed; otherwise, both confirmation and cancel buttons are shown.

            ```cpp
            void showHint(const hintMsg &m);
            ```

            - When the user clicks, `m.callback` is invoked with a boolean representing the user's choice (always `true` for a single button).
