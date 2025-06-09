# Developer documentation

[Style guide](#style-guide)  
[Definitions and Interfaces](#definitions-and-interfaces)  
[Examples](#examples)  
[Server](server.md)  
[Error](error.md)  

## Style Guide

1. Use **CamelCase** naming convention: For **type definitions**, use an uppercase initial letter (types that do not require corresponding variables are exceptions), variables and functions use a lowercase initial letter. Specifically:

```cpp
std::string myName;
void func();
struct Name {} name;

struct ovo {
    static ...// Only provides static methods, no need to instantiate the type
};
```

2. Use **4-space** indentation per level. Source files should use **UTF-8 encoding**.

3. Organize `#include` statements into categories, such as:

```cpp
// Custom Headers
#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

#include "neko/network/network.hpp"

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

4. Use `#pragma once` at the top of header files. Avoid introducing `using namespace` in the global scope within header files.

5. For simple single-line `if` statements, it is recommended to omit extra braces for brevity (of course, you can add them if you wish):

```cpp
if (true)
    return true;
else
    return false;
```

6. Always include curly braces `{}` for `for` or `while` loops.

## Definitions and Interfaces

Below are the types we define. A trailing `{}` means a type, `()` means a function, otherwise it's a variable:
- neko::info{}
    - tempDir()
        - Get or set the temporary directory path

    - workPath()
        - Get or set the current working directory path
    - getHome()
        - Get the user home path
    - getVersion()
        - Get NekoLc version
    - getVersionS()
        - std::string version
    - getResVersion()
        - Get resource version
    - getConfigFileName()
        - Get config file name
    - getConfigFileNameS()
        - std::string version
    - getOsName()
        - Get OS name
    - getOsNameS()
        - std::string version
    - getOsArch()
        - Get OS architecture
    - getOsArchS()
        - std::string version
    - LanguageKey{}
        - Stores translation keys for language files
        - General{}
        - Title{}
        - Loading{}
        - Network{}
        - Error{}
    - language()
        - Get or set preferred language
    - getLanguages()
        - Get available language list
    - loadTranslations()
        - Load and parse translation files
    - translations()
        - Get translations
- wmsg.hpp : window msg, defines structs for displaying window messages
    - ui::InputMsg{}
        - title
            - Title
        - msg
            - Message
        - poster
            - Background image path, empty string means not set
        - lines
            - Each object in the container represents a line of input required
        - callback
            - Callback function, receives a bool value, true means confirmed, false means user clicked cancel
    - ui::loadMsg{}
        - enum Type{}
            - OnlyRaw
                - Show loading icon (loading.gif) and a progress text
            - Text
                - Additionally show text components (with title h1, subtitle h2, and message msg)
            - Progress
                - Additionally show progress bar, progressVal is current value, progressMax is max value
            - All
                - Show all
        - type
            - Display type
        - process
            - Progress text
        - h1
            - Title, needs to display text component (type is Text or All)
        - h2
            - Subtitle, needs to display text component
        - msg
            - Message, needs to display text component
        - poster
            - Poster, needs background image file path, empty string means not set
        - speed
            - Speed of loading.gif icon playback, percentage
        - progressVal
            - Current value of progress bar, needs to display progress bar (type is Progress or All)
        - progressMax
            - Max value of progress bar

    - ui::hintMsg{}
        - title
            - Title
        - msg
            - Message
        - poster
            - Background image path, empty string means not set
        - buttonNum
            - Number of buttons
            - 1 : Show one ok button
            - 2 : Show ok and cancel buttons
        - callback
            - Callback function, receives a bool value, indicating whether user chose ok or cancel. Always true if only one button

- ui::PixmapWidget{}

    - setPixmap()

        - Set image
- ui::MainWindow{}

    - enum pageState
        - index : main page
        - setting : settings (menu) page
        - loading : loading page
    
    - resizeItem()
        - Resize items on the current page
    - setupSize()
        - Set basic size info, such as max/min size and spacing
    - setupStyle()
        - Set component style
    - setupTranslucentBackground()
        - Set component style
    - setupText()
        - Set text
    - setupFont()
        - Load fonts (requires config file)
    - setupBase()
        - Set basics, such as title, icon, and set values for corresponding options
    - autoSetText()
        - Automatically set h1, h2 font size based on input text size
    - setTextFont()
        - Set font size, but need to manually set text, h2, h1 font size
    - setupConnect()
        - Set up signal-slot connections
    - updatePage()
        - Switch page and auto-resize, parameters needed: page to switch to, previous page
    - showPage()
        - Encapsulated page switch, only needs one parameter

    Tips: Switching pages does not change the page state, it simply shows/hides. So functions like showHint, showLoad, etc. will set state/text when displaying the page.

    - showLoad()

        - Switch page and show loading state, and set display info
    - setLoadingVal()
        - Set current loading value
    - setLoadingNow()
        - Set current loading text
    - showHint()
        - Show hint component on current page
    - showInput()
        - Show input component on current page
    - getInput()
        - Get current input component values
    - hideInput()
        - Hide input component and disconnect previous callback signal
    - winShowHide()
        - Show or hide window, true to show, false to hide
    - signals : Signal versions of above functions, sending signals will invoke via main thread
        - showPageD()
        - showLoadD()
        - setLoadingValD()
        - setLoadingNowD()
        - showHintD()
        - showInputD()
        - hideInputD()
        - loginStatusChangeD()
            - Set status to logged in
        - winShowHideD()

- neko::networkBase{}

    - init()

        - Initialize network settings, including initializing libcurl, setting default UA, proxy, and testing available host list, etc.
    - Config{}
        - userAgent
        - proxy
        - host
        - protocol
    - Api{}
        - hostList
        - mainenance
        - checkUpdates
        - feedback
        - testing
        - Authlib{}
            - host
            - root
            - authenticate
            - refresh
            - validate
            - invalidate
            - signout
            - Injector{}
                - downloadHost
                - getVersionList
                - latest
                - baseRoot
    - enum Opt

        - none

            - Returned when method not found
        - onlyRequest
            - Only send request, no callback needed
        - downloadFile
            - Download file, no return value
            - When using this option, the specified callback is invalid
        - postText
            - Send post request, args.data is post content. Optional return value
        - postFile
            - Send post request, args.fileName is the file to upload. Optional return value.
        - getSize
            - Get target size, can only be used for getCase or getSize functions
        - getContentType
            - Get target type, can only be used for getCase function
        - getContent
            - Return content via get function
        - getHeadContent
            - Return header content via get function
    - optStr()
        - Convert option to string, returns "unknown" if not found
    - strOpt()
        - Convert string to Opt, returns Opt::none if not found
    - buildUrl()
        - Automatically build URL
    - buildUrlP()
        - Automatically build URL, need to manually fill all info, variadic parameters
    - getSysProxy()
        - Get system proxy link, returns empty if none
    - WriteCallbackString()
        - Default write to std::string callback function
    - WriteCallbackFile()
        - Default write to std::fstream callback function
- network{}

    - Args{}

        - url
        - fileName
            - Downloaded file name
        - code
            - HTTP code for this request
        - resBreakPoint
            - Whether to use breakpoint resume
        - range
            - Range request
        - userAgent
        - header
            - Header info for this request
        - data
            - Post content for this request
        - id
            - ID for this request, shown in log
        - writeCallback
            - Callback function for this request
        - headerCallback
            - Header callback function for this request
        - config
            - Config info for this request
    - autoRetryArgs{}
        - args
        - code
            - If any HTTP code in the container, considered success
        - times
            - Max retry times
        - sleep
            - Interval between retries
    - MultiArgs{}
        - args
        - enum Approach
            - Auto
                - If file >50MB, fixed to 100 parts, otherwise fixed to 5MB per part
            - Size
                - Fixed to 5MB per part
            - Quantity
                - Fixed to 100 parts
        - approach
            - File splitting method
        - nums
            - Number of threads, 0 means use system max-1
        - code
            - If any HTTP code in the container, considered success
    - Do()
        - Execute request without caring about return value
        - Available options: onlyRequest, downloadFile, postText, postFile
    - getCase()
        - Execute specific request
        - Available options: getSize, getContentType
    - getSize()
        - Get file size
    - getContentAndStorage()
        - Store to file and return content
        - T needs to support operator<< for std::ostream
    - get()
        - Request URL and return value
        - Available options: postText, postFile, getContent, getHeadContent.
    - getPtr()
    - getShadPtr()
    - getUnqePtr()
    - autoRetry()
        - Automatically retry request on error, until max times or success
    - autoRetryGet()
    - nonBlockingDo()
        - Execute Do in non-blocking thread
    - nonBlockingGet()
    - nonBlockingGetPtr()
    - Multi()
        - Download file via multi-threading
    
## Examples

1. Ask user for input and get input info

```cpp
#include "neko/ui/mainwindow.hpp"

#include "neko/function/exec.hpp"

#include "neko/schema/wmsg.hpp"

void example(){
    ui::MainWindow w(exec::getConfigObj());
    ui::InputMsg imsg;
    imsg.title = "Enter your information";
    imsg.msg = "";
    imsg.poster = "";
    imsg.lines = std::vector<std::string>{"Name","Age"};
    imsg.callback = [=](bool check){
        if (!check){
            std::cout<< "User cancelled input!\n";
            w.hideInput(); // Hide input component
            return;
        }
        std::vector<std::string> res = w.getInput();
        // The order of results matches the input order
        std::cout<< "User entered all info! Name: " <<res[0] << ", Age: " << res[1] << "\n";
        
        w.hideInput(); // On success, hide input
        // Of course, you can validate input data, prompt user if there's an issue, and ask for input again
    };
    w.showInput(imsg);
}
```

2. Show hint and ask user to choose

```cpp
#include "neko/ui/mainwindow.hpp"

#include "neko/function/exec.hpp"

#include "neko/schema/wmsg.hpp"

void example(){
    ui::MainWindow w(exec::getConfigObj());
    ui::hintMsg hmsg;
    hmsg.title = "Hint";
    hmsg.msg = "Exit the program now?";
    hmsg.poster = "";
    hmsg.buttonNum = 2;
    hmsg.callback = [=](bool check){
        if (check){
            std::cout<< "User exited!\n";
            QApplication::quit();
            return;
        }
        // The hint window will automatically hide after selection
        // Do nothing if cancelled
    }
    w.showHint(hmsg);
}
```

3. Network request

```cpp
#include "neko/network/network.hpp"

void example(){

    // ping test network
    auto ping = []{
        neko::networkBase::Opt opt = neko::networkBase::Opt::onlyRequest; // This requests the URL and gets HTTP code, but not content. Suitable for ping

        int code = 0;
        neko::network<std::string>::Args args;
        args.url = "https://example.com";
        args.fileName = "";
        args.code = &code; // HTTP code of response
        args.resBreakPoint = false;
        args.range = nullptr; 
        args.userAgent = "NekoLc/0.0.1";
        args.header = nullptr; 
        args.data = nullptr;
        args.id = "testing network - 1";
        args.config = networkBase::Dconfig; // Use default config
        args.writeCallback = &networkBase::WriteCallbackString; // Default std::string callback
        args.headerCallback = &networkBase::WriteCallbackString;

        neko::network<std::string>::Do(opt,args); // Blocks until request completes, Do is for requests without return value

        if (code == 200) {
            std::cout<< "Request succeeded!\n";
        }
        std::cout<< "Request failed!\n";
    }();

    // Download to file
    auto downloadFile = []{
        neko::networkBase::Opt opt = neko::networkBase::Opt::downloadFile; // Download file

        int code = 0;
        neko::network<std::string>::Args args;
        args.url = "https://example.com";
        args.fileName = "index.html"; // Downloaded file name
        args.code = &code;
        args.resBreakPoint = false; // Resume breakpoint, generally for large files or unstable network
        args.range = nullptr;  // Range request, useful for multi-threaded download
        args.id = "testing network - 2";
        args.writeCallback = &networkBase::WriteCallbackFile;  // Write to file callback, when Opt:downloadFile is used, this is automatically used, but the set callback is invalid.

        neko::network<std::string>::Do(opt,args); // Blocks until download completes

        if (code == 200) {
            std::cout<< "Download complete!\n";
        }
        std::cout<< "Download failed!\n";
    }();

    // Request value to variable
    auto get = []{
        neko::network<std::string> net;
        neko::networkBase::Opt opt = neko::networkBase::Opt::getContent; // Get request result

        int code = 0;
        decltype(net)::Args args;
        args.url = "https://example.com";
        args.code = &code;
        args.id = "testing network - 3";

        std::string res = net.get(opt,args);

        if (code == 200) {
            std::cout<< "Access succeeded! Result: "<<res<<"\n";
        }
        std::cout<< "Access failed!\n";
    }();

    // Custom return type
    auto customGet = []{
        // Define return type, e.g. QByteArray
        neko::network<QByteArray> net;
        neko::networkBase::Opt opt = neko::networkBase::Opt::getContent; // Get request result

        int code = 0;
        decltype(net)::Args args;
        args.url = "https://example.com";
        args.code = &code;
        args.id = "testing network - 4";
        args.writeCallback = [](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t {
            QByteArray *res = static_cast<QByteArray *>(userp);
            QByteArray->append(contents, size * nmemb);
            return size * nmemb;
        }; // Custom callback logic, you can also display progress in window

        QByteArray res = net.get(opt,args);

        // ...
    }();
    
    // Post value to variable
    auto post = []{
        neko::network<std::string> net;
        neko::networkBase::Opt opt = neko::networkBase::Opt::postText; // post text

        int code = 0;
        decltype(net)::Args args;
        args.url = "https://example.com";
        args.code = &code;
        args.data = "{\"core\":\"v0.0.0.1\",\"res\":\"v1.0.0.1\"}";
        args.header = "Content-Type: application/json";
        args.id = "testing network - 5";

        std::string res = net.get(opt,args); // Server response for post

        net.Do(opt,args); // You can also just post without getting response

        // ...
    }();
}

```

4. Download file and show progress

```cpp
#include "neko/ui/mainwindow.hpp"

#include "neko/function/exec.hpp"

#include "neko/network/network.hpp"

#include "neko/schema/wmsg.hpp"

void example(){
    ui::MainWindow w(exec::getConfigObj());
    neko::network net;

    ui::loadMsg lmsg;
    lmsg.type = ui::loadMsg::Type::Progress;
    lmsg.process = "ready download file..";
    lmsg.progressVal = 0;
    lmsg.progressMax = 1;
    w.showLoad(lmsg); // Show loading window and download progress bar
    int code = 0;
    
    w.setLoadingNow("download file...");
    
    // Note: Here, progress is based only on the number of files
    // For more accurate progress, you need to get the total size of files to download, then handle in callback, calculate and display.
    // Not implemented here

    net.Do(
        neko::networkBase::Opt::downloadFile,
        {
            "https://example.com",
            "index2.html",
            &code
        }
    );

    if (code == 200){
        // Download complete, set progress
        w.setLoadingNow("download over");
        w.setLoadingVal(1);
    } else {
        // Download failed, exit
        QApplication::quit();
    }

    // All tasks complete, switch back to main page
    w.showPage(ui::MainWindow::pageState::index);
}

```