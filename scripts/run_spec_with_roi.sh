#!/bin/bash
mpr_dir=/home/lixiang/mem-pattern-recognition
trace_dir=/mnt/centos00-home/lixiang/roi_test/trace
result_dir=${mpr_dir}/roi_test/result
champsim_dir=/home/lixiang/ChampSim
RUN_UNION_TRACE=$6
RUN_MPR=$7
RUN_CHAMPSIM=$8
RUN_VALIDATE=$9
RUN_PARSER=${10}
RUN_DEBUG=${11}

app=$1
app_binary=$2
app_input=$3
roi_file=$4
binary_file=${app_binary}
champsim_trace_file=${trace_dir}/champsim/${app}.champsim.trace
mpr_trace_file=${trace_dir}/mpr/${app}.mpr.trace
skip_len=50000000000
trace_len=$5 # 500000000

# Get trace from union tracer
if [ "$RUN_UNION_TRACE" = true ] ; then
    mkdir -p ${trace_dir}/mpr
    mkdir -p ${trace_dir}/champsim
    if [ -f "$champsim_trace_file" ] ; then
        rm "$champsim_trace_file"
    fi
    if [ -f "$mpr_trace_file" ] ; then
        rm "$mpr_trace_file"
    fi
    pin -t /home/lixiang/mem-pattern-recognition/tracer/obj-intel64/union_tracer_with_roi.so -o ${champsim_trace_file} -m ${mpr_trace_file} -t ${trace_len} -s ${skip_len} -r ${roi_file} -- ${binary_file} ${app_input}
    gzip -c ${mpr_trace_file} > ${mpr_trace_file}.gz &
    gzip -c ${champsim_trace_file} > ${champsim_trace_file}.gz &
    wait
    if [ -f "$mpr_trace_file" ] ; then
        rm "$mpr_trace_file"
    fi
    if [ -f "$champsim_trace_file" ] ; then
        rm "$champsim_trace_file"
    fi
fi

# Get pattern from MPR
stat_file=${result_dir}/${app}/${app}.stat
pattern_file=${result_dir}/${app}/${app}.pattern
if [ "$RUN_MPR" = true ] ; then
    mkdir -p ${result_dir}/${app}
    ${mpr_dir}/build/mpr --analyze -trace=${mpr_trace_file}.gz -stat=${stat_file} -pattern=${pattern_file} -len=${trace_len} 2>${result_dir}/${app}/mpr_err.txt &
fi

# Get miss from Champsim
miss_file=${result_dir}/${app}/${app}.miss
if [ "$RUN_CHAMPSIM" = true ] ; then
    mkdir -p ${result_dir}/${app}
    ${champsim_dir}/bin/champsim --warmup_instructions 0 --simulation_instructions ${trace_len} ${champsim_trace_file}.gz 2>${miss_file} &
fi
wait
if [[ "$RUN_MPR" = true || "$RUN_CHAMPSIM" = true ]]; then
    echo "Analyze $app done."
fi

result_file=${result_dir}/${app}/${app}.res
if [ "$RUN_VALIDATE" = true ] ; then
    mkdir -p ${result_dir}/${app}
    ${mpr_dir}/build/mpr --validate -trace=${mpr_trace_file}.gz -pattern=${pattern_file} -result=${result_file} -len=${trace_len} 2>${result_dir}/${app}/valid_err.txt &
fi


if [ "$RUN_PARSER" = true ] ; then
    out_file=${result_dir}/${app}/${app}.csv
    ${mpr_dir}/build/pattern2line ${miss_file} ${pattern_file} ${out_file} ${binary_file} 2>${result_dir}/${app}/parse_err.txt &
fi
wait

if [[ "$RUN_VALIDATE" = true || "$RUN_PARSER" = true ]]; then
    echo "Parse $app done."
fi

if [ "$RUN_DEBUG" = true ] ; then
    mkdir -p ${result_dir}/${app}
    ${mpr_dir}/build/mpr --debug -trace=${mpr_trace_file}.gz -pattern=${pattern_file} -result=${result_file} -len=${trace_len} 2>${result_dir}/${app}/debug_err.txt
fi