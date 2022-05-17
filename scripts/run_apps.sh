#!/bin/bash
./build.sh

apps=()
apps+=("stride")
apps+=("pointera")
apps+=("pointerb")
apps+=("indirect")
# apps+=("chain")
apps+=("struct_pointer")

bench_dir=/home/lixiang/mem-pattern-recognition/test-cases

for app in "${apps[@]}"
do
    ./scripts/run_app.sh ${app} >${bench_dir}/${app}/${app}.log &
done
wait

