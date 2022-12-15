#!/bin/bash

DIR=$1
BUILD_TYPE=$2
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo "[post-build] started..."

if [[ "$BUILD_TYPE" == 'Release' ]]; then
    echo "[post-build] BUILD_TYPE=${BUILD_TYPE}, stripping binaries"
    $SCRIPT_DIR/strip-binaries.sh $DIR
else
    echo "[post-build] BUILD_TYPE=${BUILD_TYPE}, not stripping"
fi

$SCRIPT_DIR/patch-rpath.sh $DIR
$SCRIPT_DIR/stage-static-assets.sh $DIR

echo "[post-build] finished"
