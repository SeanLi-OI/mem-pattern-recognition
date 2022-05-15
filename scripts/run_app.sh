#!/bin/bash

# Written By Lixiang

mpr_dir=/home/lixiang/mem-pattern-recognition
champsim_dir=/home/lixiang/champsim
bench_dir=${mpr_dir}/test-cases
trace_dir=${mpr_dir}/trace
result_dir=${mpr_dir}/result
RUN_UNION_TRACE=false
RUN_MPR=true
RUN_CHAMPSIM=false
RUN_VALIDATE=true
RUN_PARSER=true


app=$1
binary_file=${bench_dir}/${app}/${app}
champsim_trace_file=${trace_dir}/champsim/${app}.champsim.trace
mpr_trace_file=${trace_dir}/mpr/${app}.mpr.trace
# skip_len=750000000000
skip_len=0
trace_len=300000000

# Get trace from union tracer
if [ "$RUN_UNION_TRACE" = true ] ; then
    # Build app
    gcc -O0 -g ${binary_file}.c -o ${binary_file}
    objdump -S ${binary_file} > ${binary_file}.asm
    # app_input=
    if [ -f "$champsim_trace_file" ] ; then
        rm "$champsim_trace_file"
    fi
    if [ -f "$mpr_trace_file" ] ; then
        rm "$mpr_trace_file"
    fi
    pin -t ${mpr_dir}/tracer/obj-intel64/union_tracer.so -o ${champsim_trace_file} -m ${mpr_trace_file} -t ${trace_len} -s ${skip_len} -- ${binary_file} # <${app_input}
    gzip -c ${champsim_trace_file} > ${champsim_trace_file}.gz
    if [ -f "$champsim_trace_file" ] ; then
        rm "$champsim_trace_file"
    fi
fi

# Get pattern from MPR
stat_file=${result_dir}/${app}/${app}.stat
pattern_file=${result_dir}/${app}/${app}.pattern
if [ "$RUN_MPR" = true ] ; then
    mkdir -p ${result_dir}/${app}
    echo ${mpr_dir}/build/mpr --analyze -trace=${mpr_trace_file} -stat=${stat_file} -pattern=${pattern_file} 2>${result_dir}/${app}/mpr_err.txt
    ${mpr_dir}/build/mpr --analyze -trace=${mpr_trace_file} -stat=${stat_file} -pattern=${pattern_file} 2>${result_dir}/${app}/mpr_err.txt &
fi

# Get miss from Champsim
miss_file=${result_dir}/${app}/${app}.miss
if [ "$RUN_CHAMPSIM" = true ] ; then
    mkdir -p ${result_dir}/${app}
    ${champsim_dir}/bin/champsim --warmup_instructions 0 --simulation_instructions ${trace_len} ${champsim_trace_file}.gz 2>${miss_file} &
fi
wait
echo "Analyze $app done."

result_file=${result_dir}/${app}/${app}.res
if [ "$RUN_VALIDATE" = true ] ; then
    mkdir -p ${result_dir}/${app}
    ${mpr_dir}/build/mpr --validate -trace=${mpr_trace_file} -pattern=${pattern_file} -result=${result_file} 2>${result_dir}/${app}/valid_err.txt &
fi

if [ "$RUN_PARSER" = true ] ; then
    out_file=${result_dir}/${app}/${app}.csv
    ${mpr_dir}/build/pattern2line ${miss_file} ${pattern_file} ${out_file} ${binary_file} 2>${result_dir}/${app}/parse_err.txt &
fi

wait
echo "Run $app done."