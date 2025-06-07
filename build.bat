@echo off
setlocal

set /p QT_PATH="input Qt find path (e.g C:/Qt/6.6.1/msvc2019_64): "
set /p PKG_PATH="input Package find path (e.g C:/dev/vcpkg/installed/x64-windows): "
set /p INSTALL_PATH="input install path (default User/NekoLc): "

if "%INSTALL_PATH%"=="" set INSTALL_PATH="%USERPROFILE%/NekoLc"

if "%PKG_PATH%"=="" || "%QT_PATH%"=="" (
  echo "pkg path or qt path is empty, please input again"
  exit /b 1
)

if not exist "%QT_PATH%" (
  echo "qt path not exist, please input again"
  exit /b 1
)

if not exist "%PKG_PATH%" (
  echo "pkg path not exist, please input again"
  exit /b 1
)

copy CmakeListsCopy.txt CmakeLists.txt

echo "using pkg path : %PKG_PATH%"
echo "using qt path : %QT_PATH%"
echo "using install path : %INSTALL_PATH%"

echo "Configuring build with CMake..."
cmake . -B./build -DLIBRARY_DIRS="%PKG_PATH%" -DQt6="%QT_PATH%" -DCMAKE_BUILD_TYPE="Release"
if errorlevel 1 (
    echo "Faild to CMake configure"
    exit /b 1
)


echo "Start Build..."
cmake --build ./build --config Release --parallel
if errorlevel 1 (
    echo "Faild to Build"
    exit /b 1
)

echo "build Okay to install"
cmake --install ./build --config Release --prefix ~/NekoLc
if errorlevel 1 (
    echo "Faild to install"
    exit /b 1
)

echo "Build and install completed successfully!"
echo "You can find the installed files in %INSTALL_PATH%"
echo "Please press any key to exit..."

pause

explorer "%INSTALL_PATH%"
exit /b 0