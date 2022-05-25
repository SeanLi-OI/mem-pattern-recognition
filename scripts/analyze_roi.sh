#!/bin/bash

# pagerank 0 1 3000 2999
# seq-csr -s 16 -e 16

mpr_dir=/home/lixiang/mem-pattern-recognition
app_dir=/home/zhoujiapeng

# app=pagerank
# bin=pagerank
# args="0 1 3000 2999"

# app=seq-csr
# bin=seq-csr
# args="-s 16 -e 16"

# app=libquantum
# bin=libquantum_base.amd64-m64-gcc42-nn
# args="100 8"

# app=NPB3
# bin=NPB3.3.1/NPB3.3-SER/bin/cg.A.x
# args=""

cd ${mpr_dir}
mkdir -p perf-test/${app}
cd perf-test/${app}

perf record -g ${app_dir}/${bin} ${args}
perf script -D > perf.data.txt
${mpr_dir}/build/perf-parse ${mpr_dir}/perf-test/${app}/perf.data.txt ${mpr_dir}/perf-test/${app}/roi_raw.txt

