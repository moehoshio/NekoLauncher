#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: $0 <path_to_executable> <destination_directory>"
  exit 1
fi

PROGRAM=$1
DEST_DIR=$2


if [ ! -f "$PROGRAM" ]; then
    echo "Error: Program not found archive PROGRAM"
    exit 1
fi

mkdir -p $DEST_DIR

ldd "$PROGRAM" | grep "=>" | awk '{print $3}' | while read -r LIB; do
    if [ -f "$LIB" ]; then
        echo "Copying $LIB to $DEST_DIR..."
        cp "$LIB" "$DEST_DIR"
    else
        echo "Warning: Library $LIB not found"
    fi
done

echo "Dependency library has been copied to $DEST_DIR"
