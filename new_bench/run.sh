#!/bin/bash
trace_dir=/mnt/centos00-home/riscv-recovery/
mpr_dir=/data/lixiang/mem-pattern-recognition/
result_dir=${mpr_dir}/new_bench/result/

lock_file=/home/lixiang/.my_lock_file
max_concurrent=10
sema=0

apps=()

get_apps_manually() {
    # apps+=("blender_r_base.prefetch-m64")
    # apps+=("bwaves_r_base.prefetch-m64")
    # apps+=("cactusBSSN_r_base.prefetch-m64")
    # apps+=("cam4_r_base.prefetch-m64")
    # apps+=("cpugcc_r_base.prefetch-m64")
    # apps+=("cpuxalan_r_base.prefetch-m64")
    # apps+=("deepsjeng_r_base.prefetch-m64")
    # apps+=("exchange2_r_base.prefetch-m64")
    apps+=("fotonik3d_r_base.prefetch-m64")
    # apps+=("lbm_r_base.prefetch-m64")
    apps+=("ldecod_r_base.prefetch-m64")
    # apps+=("leela_r_base.prefetch-m64")
    apps+=("mcf_r_base.prefetch-m64")
    apps+=("nab_r_base.prefetch-m64")
    # apps+=("namd_r_base.prefetch-m64")
    # apps+=("perlbench_r_base.prefetch-m64")
    # apps+=("povray_r_base.prefetch-m64")
    apps+=("roms_r_base.prefetch-m64")
    # apps+=("xz_r_base.prefetch-m64")
}

get_apps_from_dir() {
    for file in $(ls ${trace_dir}); do
        if [ ! -d ${result_dir}/${file} ]; then
            apps+=("$file")
        fi
    done
    for app in "${apps[@]}"; do
        echo $app
    done
}

get_all_apps_from_dir() {
    for file in $(ls ${trace_dir}); do
        apps+=("$file")
    done
    for app in "${apps[@]}"; do
        echo $app
    done
}

len=100

add() {
    if [ ${sema} -lt ${max_concurrent} ]; then
        sema=$(($sema + 1))
        return 1
    fi
    return 0
}
decrease() {
    sema=$((${sema} - 1))
}

init_lock() {
    if [ -f ${lock_file} -o -d ${lock_file} ]; then
        rm -r ${lock_file}
    fi
}

get_lock() {
    while true; do
        mkdir "${lock_file}" 2>/dev/null
        if [ $? -eq 0 ]; then
            add
            if [ $? -eq 1 ]; then
                rm -rf ${lock_file}
                break
            fi
            rm -rf ${lock_file}
        fi
        sleep 10
    done
}

release_lock() {
    while true; do
        mkdir "${lock_file}" 2>/dev/null
        if [ $? -eq 0 ]; then
            decrease
            rm -rf ${lock_file}
            break
        fi
        sleep 10
    done
}

do_analyze() {
    get_lock
    echo "Start job $2 $1"
    mkdir -p ${result_dir}/${app}/${id}
    if [ -f ${result_dir}/${app}/${id}/mpr.res ]; then
        echo "Already finished job $2 $1"
    else
        trace_path=${trace_dir}/${app}/${id}/roi_trace.gz
        if [ ! -f ${trace_path} ]; then
            trace_path=${trace_dir}/${app}/${id}/trace.log.gz
            if [ ! -f ${trace_path} ]; then
                echo "Cannot find trace ${trace_path} for job $2 $1"
                release_lock
                return
            fi
        fi
        ${mpr_dir}/build/mpr \
            --analyze \
            --validate \
            -trace=${trace_path} \
            -pattern=${result_dir}/${app}/${id}/mpr.pattern \
            -stat=${result_dir}/${app}/${id}/mpr.stat \
            -result=${result_dir}/${app}/${id}/mpr.res \
            >${result_dir}/${app}/${id}/mpr.stdout \
            2>${result_dir}/${app}/${id}/mpr.stderr
        ret=$?
        echo "Finish job $2 $1 with code $?"
    fi
    release_lock
}

do_merge() {
    get_lock
    if [ $2 -eq 0 ]; then
        ${mpr_dir}/build/merge-result \
            --analyze_result \
            --validate_result \
            -input_dir=${mpr_dir}/new_bench/result/$1 \
            -trace_dir=${trace_dir}/$1 \
            -pattern=${mpr_dir}/new_bench/result/$1/mpr.pattern \
            -stat=${mpr_dir}/new_bench/result/$1/mpr.stat \
            -result=${mpr_dir}/new_bench/result/$1/mpr.res \
            >${mpr_dir}/new_bench/result/$1/mpr.stdout \
            2>${mpr_dir}/new_bench/result/$1/mpr.stderr

        echo ${mpr_dir}/build/merge-result --analyze_result --validate_result -input_dir=${mpr_dir}/new_bench/result/$1 -trace_dir=${trace_dir}/$1 -pattern=${mpr_dir}/new_bench/result/$1/mpr.pattern -stat=${mpr_dir}/new_bench/result/$1/mpr.stat -result=${mpr_dir}/new_bench/result/$1/mpr.res " exit with code" $?
    else
        ${mpr_dir}/build/merge-result \
            --analyze_result \
            --validate_result \
            -input_dir=${mpr_dir}/new_bench/result/$1 \
            -trace_dir=${trace_dir}/$1 \
            -len=$len \
            -pattern=${mpr_dir}/new_bench/result/$1/mpr.pattern \
            -stat=${mpr_dir}/new_bench/result/$1/mpr.stat \
            -result=${mpr_dir}/new_bench/result/$1/mpr.res \
            >${mpr_dir}/new_bench/result/$1/mpr.stdout \
            2>${mpr_dir}/new_bench/result/$1/mpr.stderr

        echo ${mpr_dir}/build/merge-result --analyze_result --validate_result -input_dir=${mpr_dir}/new_bench/result/$1 -trace_dir=${trace_dir}/$1 -len=$len -pattern=${mpr_dir}/new_bench/result/$1/mpr.pattern -stat=${mpr_dir}/new_bench/result/$1/mpr.stat -result=${mpr_dir}/new_bench/result/$1/mpr.res " exit with code" $?
    fi
    release_lock
}
do_merge_only_validate() {
    if [ ! -f ${result_dir}/${app}/${id}/mpr.res ]; then
        echo ${mpr_dir}/build/merge-result --validate_result -input_dir=${mpr_dir}/new_bench/result/$1 -trace_dir=${trace_dir}/$1 -len=$len -pattern=${mpr_dir}/new_bench/result/$1/mpr.pattern -stat=${mpr_dir}/new_bench/result/$1/mpr.stat -result=${mpr_dir}/new_bench/result/$1/mpr.res --enable_roi -roi=${mpr_dir}/perf-test/${app}/roi.txt
        ${mpr_dir}/build/merge-result \
            --validate_result \
            -input_dir=${mpr_dir}/new_bench/result/$1 \
            -trace_dir=${trace_dir}/$1 \
            -len=$len \
            -pattern=${mpr_dir}/new_bench/result/$1/mpr.pattern \
            -stat=${mpr_dir}/new_bench/result/$1/mpr.stat \
            -result=${mpr_dir}/new_bench/result/$1/mpr.res \
            --enable_roi \
            -roi=${mpr_dir}/perf-test/${app}/roi.txt \
            >${mpr_dir}/new_bench/result/$1/mpr.stdout \
            2>${mpr_dir}/new_bench/result/$1/mpr.stderr
    fi
}

job_v1() {
    init_lock
    for app in "${apps[@]}"; do
        if [ ! -d ${result_dir}/${app}/all ]; then
            for id in $(seq 1 $len); do
                do_analyze $id $app &
                sleep 5
            done
        else
            do_analyze "all" $app &
            sleep 5
        fi
    done
    wait
}

job_v2() {
    for app in "${apps[@]}"; do
        if [ ! -d ${result_dir}/${app}/all ]; then
            # do_merge $app 1
            echo "skip ${app}"
        else
            do_merge $app 0
        fi
    done
}

job_all() {
    init_lock
    for app in "${apps[@]}"; do
        if [ ! -d ${result_dir}/${app}/all ]; then
            for id in $(seq 1 $len); do
                do_analyze $id $app &
                sleep 5
            done
            wait
            do_merge $app 0
        else
            do_analyze "all" $app
            do_merge $app 1
        fi
        # do_merge_only_validate $app
    done
    ${mpr_dir}/build/parse-merged-result \
        --result_dir=${mpr_dir}/new_bench/result/ \
        --output=${mpr_dir}/new_bench/parse.res \
        1>${mpr_dir}/new_bench/parse.stat \
        2>${mpr_dir}/new_bench/parse.err
}

# get_apps_manually
# get_apps_from_dir
get_all_apps_from_dir

job_v1 >${mpr_dir}/new_bench/run.log 2>${mpr_dir}/new_bench/run.err
# job_v2 >${mpr_dir}/new_bench/run.log 2>${mpr_dir}/new_bench/run.err
# job_all >${mpr_dir}/new_bench/run.log 2>${mpr_dir}/new_bench/run.err
