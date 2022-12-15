#!/bin/bash

DIR=$1

echo "[stage-static-assets] gathering files..."

mkdir -p "$DIR/__output/themes"
cp -rfp "$DIR/src/app/data/themes/"*.json "$DIR/__output/themes"

mkdir -p "$DIR/__output/locales"
cp -rfp "$DIR/src/app/data/locales/"*.json "$DIR/__output/locales"

echo "[stage-static-assets] done"