#!/bin/bash

PLATFORM=$(uname)

if [[ "$PLATFORM" == 'Linux' ]]; then
    echo "[patch-rpath] patch Linux .so files..."
    patchelf --set-rpath "\$ORIGIN" __output/autom8
    patchelf --set-rpath "\$ORIGIN" __output/autom8d
    patchelf --set-rpath "\$ORIGIN" __output/libautom8.so
fi

if [[ "$PLATFORM" == 'Darwin' ]]; then
    echo "[patch-rpath] patch macOS binaries..."
    install_name_tool -add_rpath "@executable_path/" __output/autom8
    install_name_tool -add_rpath "@executable_path/" __output/autom8d
    install_name_tool -add_rpath "@executable_path/" __output/libautom8.dylib
fi

exit 0