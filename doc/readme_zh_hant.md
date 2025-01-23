# Neko Launcher

[正體中文](./readme_zh_hant.md) [English](../readme.md)  
Neko Launcher（簡稱 NekoLc 或 NeLC）是一個現代、跨平台、多語言支持的自動更新啟動器解決方案。  
它可以啟動您想要的任何目標，目前的模板可以成功啟動 Java 版 Minecraft。  
它包含自動內容更新和自我更新功能（包括您的內容和 Neko 核心本身），以及自動安裝您的內容功能。  
如果您仍在尋找自動更新解決方案，或者困擾於自動化更新（用戶不知道如何操作，缺乏自動化更新管理解決方案等問題），請嘗試一下它。  
該項目仍在開發中，歡迎任何建設性的意見。  
預覽：  
![img](../res/img/img1.png)

## 成為貢獻者

當前以下方面尚未完成：

- **UI**：過渡動畫、主題設置/樣式自定義、更好的藝術設計。

- **其他**：更多模板示例，考慮拖放機制以可視化自定義界面，也許還會增加音樂播放功能？

當然，我非常感謝您提供的任何幫助或使該項目更加完善的任何想法。如果您感興趣，請隨時提交 issue。

## 支持的平台

我使用 Qt6 構建了 GUI，但它應該沒有使用任何獨有 API。  
您應該也可以使用 Qt5 構建：  
Ubuntu 18.04  
macOS 10.13  
Windows 7  

我們的主程式同樣使用了跨平台方法，因此大多數平台都支持。  
換句話說，您也可以放棄 GUI；其核心應該仍然能正常運行。

## 構建

前提條件：  
std >= c++20  
理論上 Qt5.6.0 ?（我不確定）（測試版本為 6.6 和 6.8）  
Cmake 3.20 或以上版本（測試版本為 3.29）  
libcurl 7.0.0 或以上版本（測試版本為 8.1）  
openssl 3.0.0 或以上版本（測試版本為 3.4）  

已包含在項目依賴項中的內容：  

simpleIni 4.20  
nlohmann/json 3.11.3  
loguru 2.1  
threadpool for 2021-3507796  

滿足前提條件後，繼續：

```shell
git clone https://github.com/moehoshio/NekoLauncher.git && cd NekoLauncher

chmod +x ./build.sh && ./build.sh

# 或者

cp CmakeListsCopy.txt CmakeLists.txt

# 填寫您的路徑
cmake . -B./build -DCMAKE_PREFIX_PATH="qt 路徑和包" -DLIBRARY_DIRS="包路徑（可選）" -DQt6="qt 路徑（可選）" -DCMAKE_BUILD_TYPE="Debug 或 Release（可選）"

cmake --build ./build
```

### 貢獻與自定義

如果您想使用此項目，可能需要進行一些修改以使其可用。因為它是高客製化的。  

如果您只是想自動保持內容更新，那麼應該不會太困難。

使用我們提供的模板方法：修改 `src/include/nekodefine.hpp` 頂部的 `launcherMode` 變數。  

目前我們提供適用於 Java 版 Minecraft 的模板；將變數改為 `"minecraft"`。  

如果您需要完全自定義邏輯：在任意位置編寫函數並在 `src/include/core.hpp` 中的 `launcher` 函數中調用它。  

您可能需要編輯 `src/include/nekodefine.hpp` 或 `src/data/` 中的版本號和伺服器鏈接。  

例如，如果您想啟動 `example.exe`：  

```cpp
inline void neko::launcher(launcherOpt opt, std::function<void(const ui::hintMsg &)> hintFunc = nullptr, std::function<void(bool)> winFunc = nullptr) {
    // 編輯您的內容

    // shell: example.exe -args ...
    std::string command = std::string("example.exe ") + "-可能需要的參數 " + "args...";
    launcherProcess(command.c_str(), opt, winFunc);
}
```

有關更多詳細信息，請參考：  
[dev.md](doc/dev.md)

完成這些步驟後，您需要部署伺服器。可以在以下位置找到一個簡單的示例：  
[server.md](doc/server.md)  
