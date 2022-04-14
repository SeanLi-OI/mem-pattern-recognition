#!/bin/bash
make clean
make

apps=()
apps+=("stride")
apps+=("pointera")
apps+=("pointerb")
apps+=("indirect")
apps+=("chain")

bench_dir=/home/lixiang/mem-pattern-recognition/test-cases

for app in "${apps[@]}"
do

    ./run_app.sh ${app} >${bench_dir}/${app}/${app}.log &
done
wait
