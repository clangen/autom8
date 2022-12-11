#!/bin/sh

VERSION=$1

if [ -z "$VERSION" ]; then
  echo "usage: ./archive-win64.sh <version>"
  exit
fi

OUTPUT="__dist/autom8_win64_$VERSION"

rm -rf "$OUTPUT"

#mkdir -p "$OUTPUT/plugins"
mkdir -p "$OUTPUT/themes"
mkdir -p "$OUTPUT/locales"
cp __output64/Release/autom8-curses.exe "$OUTPUT"
cp __output64/Release/*.dll "$OUTPUT"
#cp __output64/Release/plugins/*.dll "$OUTPUT/plugins"
cp __output64/Release/themes/*.json "$OUTPUT/themes"
cp __output64/Release/locales/*.json "$OUTPUT/locales"
pushd $OUTPUT
7z a -tzip "autom8_win64_$VERSION.zip" ./* -mx=9
mv "autom8_win64_$VERSION.zip" ..
popd

rm -rf "$OUTPUT"
