#!/bin/bash

read -p "Please enter the Package path (e.g /to/path/vcpkg/installd/arm64-macos)" PKG_PATH
read -p "Please enter the Qt library path (e.g /to/path/Qt/6.6.1/clang_64/)" QT_PATH
read -p "Please enter the Install path (default ~/NekoLc/)" INSTALL_PATH

if [ -z "$INSTALL_PATH" ]; then
  INSTALL_PATH=~/NekoLc/
fi

if [ -z "$QT_PATH" ] || [ -z "$PKG_PATH" ]; then
  echo "Error: Package path and Qt library path cannot be empty."
  exit 1
fi

if [ ! -d "$PKG_PATH" ]; then
  echo "Error: Package path does not exist."
  exit 1
fi

if [ ! -d "$QT_PATH" ]; then
  echo "Error: Qt library path does not exist."
  exit 1
fi

cp CmakeListsCopy.txt CmakeLists.txt

echo "Configuring build with CMake..."

cmake . -B ./build \
  -DCMAKE_BUILD_TYPE=Release \
  -DNEKOLC_QT_DIR=$QT_PATH \
  -DNEKOLC_VCPKG_DIR=$PKG_PATH

if [ $? -ne 0 ]; then
  echo "CMake configuration failed."
  exit 1
fi

echo "Building NekoLc..."

cmake --build ./build --config Release --parallel

if [ $? -ne 0 ]; then
  echo "Build failed."
  exit 1
fi


echo "Build successful, installing to $INSTALL_PATH..."

cmake --install ./build --config Release --prefix $INSTALL_PATH

read -p "Do you want to use the export tool? (y/n): " USE_EXPORT_TOOL
if [[ "$USE_EXPORT_TOOL" == "y" || "$USE_EXPORT_TOOL" == "Y" ]]; then

  echo "Exporting libraries..."
  if command -v otool &> /dev/null; then
    bash ../scripts/exlib_otool.sh $pwd/build/NekoLc
  else
    bash ../scripts/exlib_forldd.sh $pwd/build/NekoLc
  fi  

else
  echo "Skipping export."
fi

echo "Build and install completed successfully!"
echo "You can find the installed files in $INSTALL_PATH"
