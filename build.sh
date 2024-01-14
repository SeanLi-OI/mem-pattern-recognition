#!/bin/bash
mpr_dir=`pwd`

# build glog
cd ${mpr_dir}/external/glog/
mkdir build && cd build
cmake3 ../ -DBUILD_SHARED_LIBS=OFF
make -j

# build gflags
cd ${mpr_dir}/external/gflags/
mkdir build && cd build
cmake3 ../
make -j

# build mimalloc
cd ${mpr_dir}/external/mimalloc/
mkdir out && cd out
cmake3 ../
make -j

cd ${mpr_dir}
mkdir build && cd build
cmake3 ../
make ${MAKEFLAGS}