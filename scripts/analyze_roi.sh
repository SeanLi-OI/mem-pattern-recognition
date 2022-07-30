#!/bin/bash

mpr_dir=/home/lixiang/mem-pattern-recognition
app_dir=/home/zhoujiapeng

# app=pagerank
# bin=${app_dir}/pagerank
# args="0 1 3000 2999"

# app=seq-csr
# bin=${app_dir}/seq-csr
# args="-s 16 -e 16"

# app=libquantum
# bin=${app_dir}/libquantum_base.amd64-m64-gcc42-nn
# args="100 8"

# app=NPB3
# bin=${app_dir}/NPB3.3.1/NPB3.3-SER/bin/cg.A.x
# args=""

# app=health
# bin=/home/lixiang/olden/health/run
# args="8 300 4"

app=$1
bin=$2
args=$3

cd ${mpr_dir}
mkdir -p perf-test/${app}
cd perf-test/${app}

# perf record -g ${bin} ${args}
sudo perf record -e cache-misses ${bin} ${args}
sudo perf script -D > perf.data.txt
sudo chown lixiang:lixiang perf.data.txt
# ${mpr_dir}/build/perf-parse -cycle=${mpr_dir}/perf-test/${app}/perf.data.txt -output=${mpr_dir}/perf-test/${app}/roi_raw.txt -roi=${mpr_dir}/perf-test/${app}/roi.txt
${mpr_dir}/build/perf-parse -miss=${mpr_dir}/perf-test/${app}/perf.data.txt -output=${mpr_dir}/perf-test/${app}/roi_raw.txt -roi=${mpr_dir}/perf-test/${app}/roi.txt
