#!/bin/sh
make clean 2> /dev/null
rm Makefile 2> /dev/null
find . -name __output -type d -exec rm -r "{}" \; 2> /dev/null
find . -name CMakeCache.txt -delete
find . -name cmake_install.cmake -delete
find . -name CMakeFiles -type d -exec rm -r "{}" \; 2> /dev/null
