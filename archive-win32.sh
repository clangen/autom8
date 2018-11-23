#!/bin/sh

VERSION=$1

if [ -z "$VERSION" ]; then
  echo "usage: ./archive-win32.sh <version>"
  exit
fi

OUTPUT="__output/dist/autom8_win32_$VERSION"

rm -rf "$OUTPUT"

#mkdir -p "$OUTPUT/plugins"
mkdir -p "$OUTPUT/themes"
mkdir -p "$OUTPUT/locales"
cp __output/Release/autom8-curses.exe "$OUTPUT" 
cp __output/Release/*.dll "$OUTPUT" 
#cp __output/Release/plugins/*.dll "$OUTPUT/plugins"
cp __output/Release/themes/*.json "$OUTPUT/themes"
cp __output/Release/locales/*.json "$OUTPUT/locales"
pushd $OUTPUT
7z a -tzip "autom8_win32_$VERSION.zip" ./* -mx=9
mv "autom8_win32_$VERSION.zip" ..
popd

rm -rf "$OUTPUT"