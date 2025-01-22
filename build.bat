@echo off
setlocal

set /p QT_PATH="input Qt find path: "
set /p PKG_PATH="input pkg find path"

copy CmakeListsCopy.txt CmakeLists.txt

echo using qt path: %QT_PATH% build...
if not "%PKG_PATH%"=="" (
  echo using pkg path : %PKG_PATH% build...
  cmake . -B./build -DCMAKE_PREFIX_PATH="%QT_PATH%;%PKG_PATH%"
) else (
  cmake . -B./build -DCMAKE_PREFIX_PATH="%QT_PATH%"
)

cmake --build ./build

echo build Okay