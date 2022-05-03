#!/bin/bash
if [[ -z "$1" && "$1" = "total" ]]; then
    if [ -d "build" ] ; then
        rm -rf build
    fi
    mkdir build
fi
cd build
if [[ -z "$1" && "$1" = "total" ]]; then
    cmake3 ../
fi
make -j8