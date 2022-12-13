#!/bin/sh

MAJOR=$1
MINOR=$2
PATCH=$3
COMMIT_HASH=`git rev-parse --short HEAD 2> /dev/null | sed "s/\(.*\)/#\1/"`

if [ -z "$MAJOR" ] || [ -z "$MINOR" ] || [ -z "$PATCH" ]; then
  echo "usage: update-version.sh <major> <minor> <patch>"
  exit
fi

VERSION_H="src/libautom8/include/autom8/version.h"

sed -Ei.bak "s/(\s*)(#define VERSION_MAJOR )(.*)/\1\2${MAJOR}/g" $VERSION_H
sed -Ei.bak "s/(\s*)(#define VERSION_MINOR )(.*)/\1\2${MINOR}/g" $VERSION_H
sed -Ei.bak "s/(\s*)(#define VERSION_PATCH )(.*)/\1\2${PATCH}/g" $VERSION_H
sed -Ei.bak "s/(\s*)(#define VERSION_COMMIT_HASH )(.*)/\1\2\"${COMMIT_HASH}\"/g" $VERSION_H
sed -Ei.bak "s/(\s*)(#define VERSION )(.*)/\1\2\"${MAJOR}.${MINOR}.${PATCH}\"/g" $VERSION_H

# visual studio resource files are utf16-le, so sed can't operate on them
# directly. convert to utf8, process, then back to utf16-le
iconv -f utf-16le -t utf-8 src/app/autom8.rc > autom8.rc.utf8
sed -Ei.bak "s/(\s*)(FILEVERSION )(.*)/\1\2${MAJOR}\,${MINOR}\,${PATCH}\,0/g" autom8.rc.utf8
sed -Ei.bak "s/(\s*)(PRODUCTVERSION )(.*)/\1\2${MAJOR}\,${MINOR}\,${PATCH}\,0/g" autom8.rc.utf8
sed -Ei.bak "s/(\s*)(VALUE \"FileVersion\", )(.*)/\1\2\"${MAJOR}.${MINOR}.${PATCH}.0\"/g" autom8.rc.utf8
sed -Ei.bak "s/(\s*)(VALUE \"ProductVersion\", )(.*)/\1\2\"${MAJOR}.${MINOR}.${PATCH}.0\"/g" autom8.rc.utf8
iconv -f utf-8 -t utf-16le autom8.rc.utf8 > autom8.rc.utf16
cp autom8.rc.utf16 src/libautom8/autom8.rc
rm autom8.rc.*

sed -Ei.bak "s/(\s*)(set \(autom8_project_VERSION_MAJOR )(.*)/\1\2${MAJOR}\)/g" CMakeLists.txt
sed -Ei.bak "s/(\s*)(set \(autom8_project_VERSION_MINOR )(.*)/\1\2${MINOR}\)/g" CMakeLists.txt
sed -Ei.bak "s/(\s*)(set \(autom8_project_VERSION_PATCH )(.*)/\1\2${PATCH}\)/g" CMakeLists.txt

# ugh. there's a way to tell sed not to backup, but it's different on gnu and
# bsd sed variants. this is easier than trying to switch the args dynamically.
rm "${VERSION_H}.bak"
rm CMakeLists.txt.bak
