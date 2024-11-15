# dev
[module](#module-list)  
[style guide](#style-guide)  
[module for example](#module-example)  
[server](server.md)  

## Style Guide

1. **CamelCase** should be used for naming. Specifically:

```cpp
std::string myName;
void func();
```

2. Use **4-space** indentation per level. Source files should use **UTF-8 encoding** with **LF** line endings (not strictly enforced).

3. Organize `#include` statements into categories, such as:

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

4. Use `#pragma once` at the top of header files. Avoid introducing `using namespace` in the global scope within header files. Scoping should be applied most of the time.

5. For simple single-line `if` statements, it's recommended to omit extra braces for brevity:

```cpp
if (true)
    return true;
else
    return false;
```

6. Always include curly braces `{}` for `for` or `while` loops.

### Module List

1. **Auto-init Function**:
    - Automatically configures all initial information.
    - Specifically, it will:
        - Correct the working directory
        - Load configuration
        - Set the log
        - Set thread numbers
        - Set thread names
        - Print configuration
        - Set language
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
                const char * debug = exec::getConfigObj().GetValue("Section","key","default");
            ```

    - utility func
        - look the [exec.h](https://github.com/moehoshio/exec.h)

3. **network.h**: Inherits from `networkBase`.
    - **networkBase**:
        - Initialization function
        - Defines classes/structures:
            - **Config**: Network Configuration information
                - User-Agent (ua)
                - Proxy
                - Host
                - Protocol
                    - Includes a default configuration object:
                        - `static Dconfig`
            - **Api**: APIs, detailed in [server.md](server.md)
                - Members are `constexpr static const char *`
                - Minecraft Authlib Apis for [authlib-injector](https://github.com/yushijinhun/authlib-injector/wiki/)
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
        static std::unordered_map<Opt, std::string_view> optMap;
        ```

        ```cpp
        inline static std::string optStr(Opt opt);
        ```

        ```cpp
        inline static Opt strOpt(const std::string_view str);
        ```

        - **buildUrl**: Constructs a complete URL from a fixed or pre-concatenated path

        ```cpp
        static std::string buildUrl(const std::string &path, const std::string &host = Dconfig.host, const std::string &protocol = Dconfig.protocol);
        ```

        - **buildUrlP**: Constructs a complete URL with an indefinite number of path segments; protocol and host need to be manually specified.

        ```cpp
        template <typename T = std::string, typename... Paths>
        constexpr static T buildUrlP(const T &protocol, const T &host, Paths &&...paths);
        ```

        - **getSysProxy**: Retrieves system proxy information (only unix)

        ```cpp
        template <typename T = std::string>
        static T getSysProxy();
        ```

        - **errCodeReason** : Provides a general reason for the error code This. is typically used to inform the user.

        ```cpp
        template<typename T = std::string>
        constexpr static T errCodeReason(int code);
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
                - `header`: cstr Custom request headers, multiple key-value pairs can be separated by `\n`.
                - `data`: cstr data to be sent when using POST
                - `id` : cstr
                - `writeCallback`: `size_t(char*, size_t, size_t, void*)`
                - `headerCallback`: `size_t(char*, size_t, size_t, void*)`
                - `config`: Network Config
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

5. **core.h**:
   - All objects are within the project namespace `neko`.
   - **Defined Classes/Structures**:
     - **State Enum**:
       - `over`: Indicates the task is completed, based on specific semantics.
       - `undone`: Indicates the task is not completed, possibly due to a fatal error such as JSON parsing failure.
       - `tryAgainLater`: Indicates the task is not completed but can be retried, such as a failed network request.
     - **updateInfo Struct**:
       - `title`: std::string
       - `msg`: std::string
       - `poster`: std::string
       - `time`: std::string
       - `resVersion`: std::string
       - `mandatory`: bool
       - **urlInfo**: Information for a single URL
         - `url`: std::string
         - `name`: std::string
         - `hash`: std::string
         - `hashAlgorithm`: std::string
         - `multis`: bool
         - `temp`: bool
         - `randName`: bool
         - `absoluteUrl`: bool
         - **empty func**
       - `urls`: urlInfo
       - **empty func**
     - **launcherOpt Enum**:
       - `keep`: Indicates maintaining the current window state
       - `endProcess`: Indicates ending the launcher after starting
       - `hideProcessAndOverReShow`: Indicates hiding after starting and re-displaying after the process ends.
   - **Functions**
     - Contains the main logic and functions of the program:
       - **downloadPoster**: Downloads the poster and returns the file name. If it fails, returns an empty container and notifies the user of the failure.
         - `hintFunc`: Function for notifying the user
         - `url`: Poster URL

         ```cpp
         template <typename T = std::string>
         inline T downloadPoster(std::function<void(const ui::hintMsg &)> hintFunc, const std::string &url);
         ```

       - **checkMaintenance**: Checks maintenance information and returns `over` if no maintenance is needed, `undone` if maintenance is ongoing.
         - `hint`: Function for notifying the user
         - `loadFunc`: Function for displaying the loading page; the function updates the status to loading (shows the loading page)
         - `setLoadInfoFunc`: Function for setting the loading status

         ```cpp
         inline State checkMaintenance(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc);
         ```

       - **checkUpdate**: Checks for update information; returns `over` if no update is available, `undone` if there is an update.
         - `res`: The returned update information.

         ```cpp
         inline State checkUpdate(std::string &res);
         ```

       - **parseUpdate**: Parses update information into `updateInfo`; returns an empty object if an error occurs.
         - `res`: Content to be parsed.

         ```cpp
         inline updateInfo parseUpdate(const std::string &res);
         ```

       - **autoUpdate**: Automatically updates; if an update is available, it will automatically parse and download. Returns `over` if the update is completed (i.e., if there is no update or the update is completed).
         - `hintFunc`: Function for notifying the user; passed to `checkMaintenance` and used to notify the user upon download failure and update completion.
         - `loadFunc`: Function for displaying the loading page; passed to `checkMaintenance` and used to show download progress.
         - `setLoadInfoFunc`: Function for setting loading progress; passed to `checkMaintenance` and used to display download progress during the download.

         ```cpp
         inline State autoUpdate(std::function<void(const ui::hintMsg &)> hintFunc, std::function<void(const ui::loadMsg &)> loadFunc, std::function<void(unsigned int val, const char *msg)> setLoadInfoFunc);
         ```

       - **launchNewProcess**: Launches a new process that can continue running after the main program ends. Typically used when updating the core.
         - `command`: The command to be executed

         ```cpp
         void launchNewProcess(const std::string &command);
         ```

       - **launcherProcess**: Launches a process
         - `command`: The command to be executed
         - `opt`: The method for handling the program after launching the process
         - `winFunc`: If using `launcherOpt::hideProcessAndOverReShow`, this function is required. `false` means hide the window, `true` means show the window.

         ```cpp
         inline void launcherProcess(const std::string &command, launcherOpt opt, std::function<void(bool)> winFunc = NULL);
         ```

       - **launcherLuaPreCheck**: A sample for launching Lua, checking for Lua environment variables and whether the target script is exists before launching.

         ```cpp
         inline bool launcherLuaPreCheck();
         ```

       - **launcher**: A sample for launching Lua that you can fully customize. Called when the user clicks.
         - `opt`: The method for handling the program after launching the process

         ```cpp
         inline void launcher(launcherOpt opt);
         ```

## Module Example

1. network: You can refer to [test/network/main.cpp](../test/network/main.cpp) It includes usage examples for most scenarios.
