#!/bin/bash

# pagerank 0 1 3000 2999
# seq-csr -s 16 -e 16

mpr_dir=/home/lixiang/mem-pattern-recognition
app_dir=/home/zhoujiapeng
app=pagerank
args="0 1 3000 2999"

cd ${mpr_dir}
mkdir -p perf-test/${app}
cd perf-test/${app}

perf record -g ${app_dir}/${app} ${args}
perf script -D > perf.data.txt
${mpr_dir}/build/perf-parse ${mpr_dir}/perf-test/${app}/perf.data.txt ${mpr_dir}/perf-test/${app}/roi_raw.txt

