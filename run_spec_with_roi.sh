#!/bin/bash
mpr_dir=/home/lixiang/mem-pattern-recognition
trace_dir=/mnt/centos00-home/lixiang/roi_test/trace
result_dir=${mpr_dir}/roi_test/result
champsim_dir=/home/lixiang/ChampSim
RUN_UNION_TRACE=true
RUN_MPR=true
RUN_CHAMPSIM=true
RUN_PARSER=true

app=$1
app_binary=$2
app_input=$3
roi_file=$4
binary_file=${app_binary}
champsim_trace_file=${trace_dir}/champsim/${app}.champsim.trace
mpr_trace_file=${trace_dir}/mpr/${app}.mpr.trace
skip_len=0
trace_len=100000

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
    gzip -c ${mpr_trace_file} > ${mpr_trace_file}.gz
    if [ -f "$mpr_trace_file" ] ; then
        rm "$mpr_trace_file"
    fi
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
    ${mpr_dir}/bin/mpr ${mpr_trace_file}.gz ${stat_file} ${pattern_file} 2>${result_dir}/${app}/mpr_err.txt &
fi

# Get miss from Champsim
miss_file=${result_dir}/${app}/${app}.miss
if [ "$RUN_CHAMPSIM" = true ] ; then
    mkdir -p ${result_dir}/${app}
    ${champsim_dir}/bin/champsim --warmup_instructions 0 --simulation_instructions ${trace_len} ${champsim_trace_file}.gz 2>${miss_file} &
fi
wait
echo "Analyze $app done."

if [ "$RUN_PARSER" = true ] ; then
    out_file=${result_dir}/${app}/${app}.csv
    ${mpr_dir}/result/parse ${miss_file} ${pattern_file} ${out_file} ${binary_file} 2>${result_dir}/${app}/parse_err.txt
fi

echo "Run $app done."