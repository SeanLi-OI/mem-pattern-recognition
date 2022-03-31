#!/bin/bash

make clean
make

apps=()
# apps+=("loop")
# apps+=("loop2")
# apps+=("indirect")
# apps+=("502.gcc_r")
# apps+=("505.mcf_r")
# apps+=("520.omnetpp_r")
apps+=("605.mcf_s")

# trace_path=/home/lixiang/mem-pattern-recognition/trace
trace_path=/mnt/centos00-home/lixiang/trace

for app in "${apps[@]}"
do
    ./bin/mpr ${trace_path}/${app}.trace.gz 2>stderr
done