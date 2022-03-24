#!/bin/bash
# app=loop
# cd tracer
# ./make_tracer.sh
# cd ..
# pin -t tracer/obj-intel64/mpr_tracer.so -- test-cases/${app} <test-cases/${app}.in ${app}.trace
# xz ${app}.trace

apps=()
apps+=("loop")
apps+=("loop2")
apps+=("indirect")
# apps+=("502.gcc_r")
# apps+=("505.mcf_r")
# apps+=("520.omnetpp_r")
# apps+=("605.mcf_s")

trace_path=/home/lixiang/mem-pattern-recognition/trace
# trace_path=/mnt/centos00-home/lixiang/trace

for app in "${apps[@]}"
do
    echo $app
    cd /home/lixiang/mem-pattern-recognition/test-cases/${app}
    date +"%T.%N"
    pin -t /home/lixiang/mem-pattern-recognition/tracer/obj-intel64/mpr_tracer_v2.so -- ./run.sh ${trace_path}/${app}.trace
    # pin -t /home/lixiang/mem-pattern-recognition/tracer/obj-intel64/mpr_tracer.so --  ./mcf_r inp.in  > inp.out 2>> inp.err /home/lixiang/mem-pattern-recognition/trace/${app}.trace
    # pin -t /home/lixiang/mem-pattern-recognition/tracer/obj-intel64/mpr_tracer.so -- ./loop2 <loop.in /home/lixiang/mem-pattern-recognition/trace/${app}.trace
    # pin -t /home/lixiang/mem-pattern-recognition/tracer/obj-intel64/mpr_tracer.so -- ./mcf_s_base.mytest-m64 inp.in > inp.out 2>> inp.err /mnt/centos00-home/lixiang/trace/${app}.trace
    date +"%T.%N"
    cd ${trace_path}
    gzip ${app}.trace -f
done
# cd /home/lixiang/mem-pattern-recognition/test-cases/520.omnetpp_r
# ls -lh
# pin -t /home/lixiang/mem-pattern-recognition/tracer/obj-intel64/mpr_tracer.so -- ./omnetpp_r_base.mytest-m64 -c General -r 0 > omnetpp.General-0.out 2>> omnetpp.General-0.err /home/lixiang/mem-pattern-recognition/trace/520.omnetpp_r.trace
