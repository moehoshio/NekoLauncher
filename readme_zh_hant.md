# Neko Launcher

[正體中文](readme_zh_hant.md) [简体中文](readme_zh_hans.nd) [English](readme.md)  
Neko Launcher (NekoLc) 是一個現代化、跨平台、多語言支援的自動更新啟動器解決方案。  
它可以啟動你想要的任何目標，目前的模板已經可以成功啟動 Java 版 Minecraft。  
它包含自動內容更新與自我更新（不論是你的內容還是 Neko Core 本身），也支援自動安裝你的內容。  
如果你還在尋找自動更新方案，或是被自動化更新困擾（用戶不會操作、缺乏自動化更新管理方案……），那麼可以試試這個。  
本專案仍在開發中，歡迎任何建設性的想法。  
預覽：  
![img](resource/img/img1.png)  
gif:  
![img2](resource/img/img2.gif)  

## 歡迎成為貢獻者

目前尚有以下部分尚未完善：

- **UI**：轉場動畫、主題設定/樣式自訂，以及更好的美術設計。

- **其他**：更多模板範例，考慮拖曳機制以視覺化自訂介面，甚至可能加入音樂播放功能？

當然，非常感謝你能提供任何協助或想法讓專案更加完善。如果你有興趣，歡迎提交 issue。

## 支援平台

我使用 Qt6 建構 GUI，但其實並未使用任何特殊 API。  
你應該也可以用 Qt5 來建構：  
Ubuntu 18.04  
macOS 10.13  
Windows 7  

主程式也採用跨平台方式實作，因此大多數平台都能支援。  
換句話說，你也可以不用 GUI，核心依然能正常運作。

## 建構方式

先決條件：  
std >= c++20  
Qt5.6.0 ?（6.6 與 6.8 為測試版本）  
cmake 3.20 以上（3.29 為測試版本）  
libcurl 8.1 為測試版本  
openssl 3.4 為測試版本  
boost.process 1.86.0 為測試版本  

專案已內建以下相依套件：  

simpleIni 4.20  
nlohmann/json 3.11.3  
loguru 2.1  
threadpool for 2021-3507796

滿足先決條件後，請繼續：

```shell
git clone https://github.com/moehoshio/NekoLauncher.git && cd NekoLauncher

# Unix
chmod +x ./build.sh && ./build.sh

# Windows
build.bat

# 或手動建構

cp CmakeListsCopy.txt CmakeLists.txt

# 填寫你的路徑
cmake . -B./build -DCMAKE_PREFIX_PATH="qt 路徑與套件" -DLIBRARY_DIRS="套件路徑(選填)" -DQt6="qt 路徑(選填)" -DCMAKE_BUILD_TYPE="Debug 或 Release (選填)"

cmake --build ./build --config Release
```

此外，工作目錄還需要包含以下檔案：  
必須有的資料夾：lang/、img/  
必須有的檔案：config.ini、cacert.pem、img/loading.gif、img/ico.png、lang/en.json。（這不包含你的動態連結程式庫）

### 貢獻與自訂

如果你想使用本專案，可能需要根據需求進行修改，因為它高度自訂化。  

如果你只是想讓內容自動更新，其實不會太困難。

使用我們提供的模板方法：修改 `src/include/nekodefine.hpp` 最上方的 `launcherMode` 變數。  

目前已提供 Minecraft Java 的模板，將變數改為 `"minecraft"` 即可。  

如果你想要完全自訂邏輯：可以在任何地方寫你的函式，然後在 `src/include/neko/core/launcher.hpp` 的 `launcher` 函式中呼叫它。  

你可能還需要編輯 `src/include/nekodefine.hpp` 裡的版本號與伺服器連結。  

例如，如果你想啟動 `example.exe`：

```cpp
inline void launcher(std::function<void(const ui::hintMsg &)> hintFunc,std::function<void()> onStart, std::function<void(int)> onExit) {
    // 寫下你的內容

    // shell: example.exe -args ...
    std::string command = std::string("example.exe ") + "-你可能需要一些參數 " + "args...";
    launcherProcess(command,onStart,onExit);
}
```

更多詳細資訊請參考：  
[dev.md](doc/dev.md)

完成上述步驟後，你還需要部署你的伺服器。簡單範例可參考：  
[server.md](doc/server.md)。
