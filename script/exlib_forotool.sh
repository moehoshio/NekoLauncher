# #!/bin/bash

# if [ $# -ne 2 ]; then
#   echo "Usage: $0 <path_to_executable> <destination_directory>"
#   exit 1
# fi

# PROGRAM=$1
# DEST_DIR=$2

# if [ ! -f "$PROGRAM" ]; then
#     echo "Error: Program not found archive PROGRAM"
#     exit 1
# fi

# mkdir -p $DEST_DIR

# for lib in $(otool -L $PROGRAM | awk '{print $1}' | tail -n +2); do
#   if ["$lib" == /System/*]; then
#     echo "Skipping system library: $lib"
#     continue
#   fi

#   BASENAME=$(basename "$lib")
#   echo "Copying $lib to $DEST_DIR/$BASENAME..."
#   cp "$lib" "$DEST_DIR/$BASENAME"

#   echo "Updating $PROGRAM to use @executable_path/$DEST_DIR/$BASENAME"
#   install_name_tool -change "$lib" "@executable_path/$DEST_DIR/$BASENAME" "$PROGRAM"
# done

# echo "All dependencies copied and paths updated."