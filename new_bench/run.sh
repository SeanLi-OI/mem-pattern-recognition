#!/bin/bash
trace_dir=/mnt/centos00-home/songyifei/source/gem5-gp/recovery
mpr_dir=/home/lixiang/mem-pattern-recognition
result_dir=${mpr_dir}/new_bench/result

lock_file=/home/lixiang/.my_lock_file
max_concurrent=20
sema=0

apps=()

get_apps_manually() {
  # apps+=("cpugcc_r_base.prefetch-m64")
  # apps+=("mcf_r_base.prefetch-m64")
  apps+=("cpuxalan_r_base.prefetch-m64")
  apps+=("perlbench_r_base.prefetch-m64")
}

get_apps_from_dir() {
  for file in $(ls ${trace_dir}); do
    apps+=("$file")
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
    if mkdir "${lock_file}" 2>/dev/null; then
      add
      if [ $? -eq 1 ]; then
        rm -rf ${lock_file}
        break
      fi
      rm -rf ${lock_file}
    fi
    sleep 1
  done
}

release_lock() {
  while true; do
    if mkdir "${lock_file}" 2>/dev/null; then
      decrease
      rm -rf ${lock_file}
      break
    fi
    sleep 1
  done
}

do_analyze() {
  get_lock
  echo "Start job $2 $1"
  mkdir -p ${result_dir}/${app}/${id}
  if [ -f ${result_dir}/${app}/${id}/mpr.res ]; then
    echo "Already finished job $2 $1"
  else
    ${mpr_dir}/build/mpr \
      --analyze \
      --validate \
      -trace=${trace_dir}/${app}/${id}/roi_trace.gz \
      -pattern=${result_dir}/${app}/${id}/mpr.pattern \
      -stat=${result_dir}/${app}/${id}/mpr.stat \
      -result=${result_dir}/${app}/${id}/mpr.res \
      >${result_dir}/${app}/${id}/mpr.stdout \
      2>${result_dir}/${app}/${id}/mpr.stderr
    echo "Finish job $2 $1 with code $?"
  fi
  release_lock
}

do_merge() {

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

}

job_v1() {
  init_lock
  for app in "${apps[@]}"; do
    for id in $(seq 1 $len); do
      do_analyze $id $app &
    done
  done
  wait
}

job_v2() {
  for app in "${apps[@]}"; do
    do_merge $app
  done
}

job_all() {
  init_lock
  for app in "${apps[@]}"; do
    for id in $(seq 1 $len); do
      do_analyze $id $app &
    done
    wait
    do_merge $app
  done
}

get_apps_manually
# get_apps_from_dir

# job_v1 >${mpr_dir}/new_bench/run.log 2>${mpr_dir}/new_bench/run.err
# job_v2 >${mpr_dir}/new_bench/run.log 2>${mpr_dir}/new_bench/run.err
job_all >${mpr_dir}/new_bench/run.log 2>${mpr_dir}/new_bench/run.err
