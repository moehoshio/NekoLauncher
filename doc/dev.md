# dev
[module](#module-list)  
[style guide](#style-guide)  
[module for example](#module-example)  
[server](server.md)  

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
            - Set TLS settings
            - Configure default host and Returns a future object indicating whether the test of networkBase::Dconfig.host is complete
        - Returns the object returned by networkBase::init

    Function prototype:

```cpp
inline auto /* std::future<void>*/ neko::autoInit(int argc, char *argv[])
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

        - **Default Callback Functions**:

        ```cpp
        static size_t WriteCallbackString(char *ptr, size_t size, size_t nmemb, void *userdata);
        ```

        - When writing to a file is required, e.g., `Opt::downloadFile`

        ```cpp
        static size_t WriteCallbackFile(char *contents, size_t size, size_t nmemb, void *userp);
        ```

        - For cases where the return type is `QByteArray` (T = `QByteArray`)

        ```cpp
        static size_t WriteCallbackQBA(char *contents, size_t size, size_t nmemb, void *userp);
        ```

    - **network**:
        - Prototype: (T cannot be a pointer)

        ```cpp
        template <typename T = std::string>
        class network : public networkBase
        ```

        - **Defined Classes/Structures**:
            - **Args**: Parameters for the current request
                - `url`: cstr
                - `fileName`: cstr
                - `(ret)code`: int* returned HTTP code
                - `resBreakPoint`: bool
                - `range`: cstr range of the file to fetch
                - `userAgent`: cstr
                - `data`: cstr data to be sent when using POST
                - `id` : cstr
                - `writeCallback`: `size_t(char*, size_t, size_t, void*)`
                - `headerCallback`: `size_t(char*, size_t, size_t, void*)`
                - `config`: Config
            - **autoRetryArgs**: Parameters for using the `autoRetry` function
                - `args`: Args
                - `code`: vector list int expected HTTP code, retries if not met
                - `times`: size_t maximum number of retries
                - `sleep`: size_t sleep time between retries (milliseconds)
            - **MultiArgs**: Parameters for using the `Multi` function
                - `args`: Args
                - **Defined Approach Enum**: Download method
                    - `Auto`: Splits into 100 parts if less than 50MB, otherwise 5MB each part
                    - `Size`: Fixed size of 5MB
                    - `Quantity`: Fixed quantity of 100 files
                - `nums`: Number of download tasks, 0 equals thread count
                - `approach`
                - `code`: vector list int , Expected HTTP code, Usually http code 206
            - **Error Return Values**: (Args::code)  
                - `-1`: Indicates an error occurred during libcurl initialization.
                - `-3`: Indicates an unexpected standard exception occurred.
                - `-4`: Indicates an error during the execution of a network request, such as an SSL connection failure, DNS resolution failure, etc.
                - `-5`: Indicates the use of an incorrect method/option. Refer to the applicable options for the function later.
                - `-6`: Occurs in the getSize function, indicating an invalid Content-Length value.
                - `-7`: Occurs in the getSize function, indicating the Content-Length value is out of range.
                - `-8`: Occurs in the getCase function when using methods other than Opt::getSize or Opt::getContentType.
        - **Functions**:
            - **Do**: Base function
                - Applicable Options:
                    - `Opt::onlyRequest`: Only request
                    - `Opt::downloadFile`: Downloads to `args.fileName`; note the use of the file writing callback.
                    - `Opt::postText`: POSTs `args.data`
                    - `Opt::postFile`: Not implemented at the time of writing this document

                        ```cpp
                        inline static void Do(Opt opt, Args &args) noexcept;
                        ```

                        The T here is useless.

            - **get**: Base function
                - Applicable Options:
                    - `Opt::postText`: POSTs `args.data` with a return content
                    - `Opt::getContent`: Retrieves content (can be binary)
                    - `Opt::getHeadContent`: Retrieves only the header content

                        ```cpp
                        inline T get(Opt opt, Args &args) noexcept;
                        ```

                - Returns an empty object if an error occurs

            - **getCase**: Base function
                - Applicable Options:
                    - `Opt::getContentType`: Retrieves type from the header
                    - `Opt::getSize`: Retrieves size from the header

                        ```cpp
                        inline std::string getCase(Opt opt, Args &args);
                        ```

                - Returns an empty object if an error occurs
                - Note that all return values will be converted to lowercase.

            - **getSize**: Encapsulation function

                ```cpp
                inline size_t getSize(Args &args) noexcept;
                ```

                - Returns 0 if an error occurs

            - **getContentAndStorage**: Encapsulation function
                - Returns content and stores it to `args.fileName`
                - T must support `operator<<` for `std::ostream` (`std::ostream<<T`)

                    ```cpp
                    inline T getContentAndStorage(Args &args) noexcept;
                    ```

            - **getPtr**:
                - Scope of use: same as `get` function

                ```cpp
                inline T *getPtr(Opt opt, Args &args) noexcept;
                ```

                - Overloaded version supports temporary arguments `(const Args & args)`

            - **getShadPtr**:
                - Scope of use: same as `get` function

                ```cpp
                inline std::shared_ptr<T> getShadPtr(Opt opt, Args &args) noexcept;
                ```

                - Overloaded version supports temporary arguments `(const Args & args)`

            - **getUnqePtr**:
                - Scope of use: same as `get` function

                ```cpp
                inline std::unique_ptr<T> getUnqePtr(Opt opt, Args &args) noexcept;
                ```

                - Overloaded version supports temporary arguments `(const Args & args)`

        - Functions to handle retry logic and non-blocking execution are provided, such as:

        - **autoRetry**: Encapsulation function
            - Scope of use: same as `Do` function
            - If the maximum retry count is reached without obtaining the expected HTTP code, returns `false`.
            - Note: If resume is enabled, HTTP code 416 is also considered as completed.

                ```cpp
                inline static bool autoRetry(Opt opt, autoRetryArgs &ra);
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
                inline static auto nonBlockingDo(Opt opt, Args &args) -> std::future<void>;
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

## Module Example

1. network: You can refer to [test/network/main.cpp](../test/network/main.cpp) It includes usage examples for most scenarios.
