#!/bin/bash

result=/data/lixiang/mem-pattern-recognition/new_bench/result
total_result=${result}/../total

mkdir -p ${total_result}

for app in `ls ${result}`
do
    echo ${result}/${app}/mpr.stat 
    if [ -f ${result}/${app}/mpr.stat ]; then
        cp ${result}/${app}/mpr.stat ${total_result}/${app}.stat
        echo ${total_result}/${app}.stat
    fi
done