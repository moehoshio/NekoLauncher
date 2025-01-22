#!/bin/bash

read -p "Please enter the Qt library path: " QT_PATH
read -p "Please enter the optional package (pkg) path (if any, otherwise leave blank): " PKG_PATH

cp CmakeListsCopy.txt CmakeLists.txt

echo "Building with Qt library path: $QT_PATH ..."
if [ -n "$PKG_PATH" ]; then
  echo "Building with optional package path: $PKG_PATH ..."
  cmake . -B./build -DCMAKE_PREFIX_PATH="$QT_PATH:$PKG_PATH"
else
  cmake . -B./build -DCMAKE_PREFIX_PATH="$QT_PATH"
fi

cmake --build ./build

echo "Build completed, attempting to export dependencies..."

cd ./build

if command -v otool &> /dev/null; then
  bash ../scripts/exlib_otool.sh $pwd/build/NekoLc
else
  bash ../scripts/exlib_forldd.sh $pwd/build/NekoLc
fi
