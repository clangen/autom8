#!/bin/bash

DIR=$1
if [ -z "$DIR" ]; then
  echo "[patch-rpath] directory not specified, using current working directory"
  DIR=`pwd`
fi

PLATFORM=$(uname)

if [[ "$PLATFORM" == 'Linux' ]]; then
    echo "[patch-rpath] patch Linux .so files..."
    patchelf --set-rpath "\$ORIGIN" $DIR/__output/autom8
    patchelf --set-rpath "\$ORIGIN" $DIR/__output/autom8d
    patchelf --set-rpath "\$ORIGIN" $DIR/__output/libautom8.so
    patchelf --set-rpath "\$ORIGIN" $DIR/__output/libf8n.so
    patchelf --set-rpath "\$ORIGIN" $DIR/__output/libcursespp.so
fi

if [[ "$PLATFORM" == 'Darwin' ]]; then
    echo "[patch-rpath] patch macOS binaries..."
    install_name_tool -add_rpath "@executable_path/" $DIR/__output/autom8
    install_name_tool -add_rpath "@executable_path/" $DIR/__output/autom8d
    install_name_tool -add_rpath "@executable_path/" $DIR/__output/libautom8.dylib
    install_name_tool -add_rpath "@executable_path/" $DIR/__output/libf8n.dylib
    install_name_tool -add_rpath "@executable_path/" $DIR/__output/libcursespp.dylib
fi

echo "[patch-rpath] finished"

exit 0