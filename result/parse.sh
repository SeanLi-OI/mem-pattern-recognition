#!/bin/bash
DEBUG=1

if [[ "$DEBUG" -eq "1" ]]
then
    ARGS="-O0 -g"
else
    ARGS="-O3"
fi
MISS_FILE=/home/lixiang/ChampSim/result/605.mcf_s.miss.txt
PATTERN_FILE=/home/lixiang/mem-pattern-recognition/result/605.mcf_s.pattern
OUT_FILE=/home/lixiang/mem-pattern-recognition/result/605.mcf_s.miss.csv
BINARY_FILE=/home/lixiang/mem-pattern-recognition/test-cases/605.mcf_s/mcf_s_base.mytest-m64
ERR_FILE=/home/lixiang/mem-pattern-recognition/result/parse.err
g++ ${ARGS} -std=c++17 parse.cpp -o parse && 
./parse ${MISS_FILE} ${PATTERN_FILE} ${OUT_FILE} ${BINARY_FILE} 2>${ERR_FILE}