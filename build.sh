#!/bin/bash
if [ "$1" = "total" ]; then
    rm -rf build
fi
mkdir -p build
cd build
if [ "$1" = "total" ]; then
    cmake3 ../
fi
make -j8