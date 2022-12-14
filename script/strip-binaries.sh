#!/bin/sh
DIR=$1
if [ -z "$DIR" ]; then
  echo "[strip] directory not specified, using current working directory"
  DIR=`pwd`
fi
echo "[strip] resolved directory: ${DIR}/__output/"
strip $DIR/__output/* 2> /dev/null
echo "[strip] finished"
