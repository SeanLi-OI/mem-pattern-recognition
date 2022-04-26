# Demo

以五种访存模式生成器为基础
跑通：（1）生成trace（2）分析pattern（3）模拟miss（4）最终输出统计信息；这一整个pipeline
（当然还有预测准确性这一功能，有需求可以参考Quick Start中的描述进行测试）
```bash
cd external/ChampSim
./config.sh champsim_config.json
make # build Champsim
cd ../../tracer
./make_union_tracer.sh # build union_tracer
./run_apps.sh
```

# Quick Start

Union tracer输出适合champsim和我们mpr工具的两份trace
```bash
# 抓取trace
binary_file= # 应用可执行文件
app_input= # 应用输入文件
champsim_trace_file=# 输出champsim的trace
mpr_trace_file= # 输出mpr的trace
trace_len=300000000 # trace_len指pin抓的指令总数，并不是访存总数
pin -t tracer/obj-intel64/union_tracer.so -o ${champsim_trace_file} -m ${mpr_trace_file} -t ${trace_len} -- ${binary_file} <${app_input}
gzip -c ${champsim_trace_file} > ${champsim_trace_file}.gz # champsim只支持gz/xz压缩文件
```

通过mpr分析各PC的类别
```bash
stat_file= # 统计不同类别的PC数和指令数
pattern_file= # 分别给出每个PC的类别，以及执行次数
build/mpr --analyze -trace=${mpr_trace_file} -stat=${stat_file} -pattern=${pattern_file} 2>${result_dir}/${app}/mpr_err.txt
```

(optional) 通过champsim获取miss情况
```bash
miss_file= # 输出miss信息
external/ChampSim/bin/champsim --warmup_instructions 0 --simulation_instructions ${trace_len} ${champsim_trace_file} 2>${miss_file}
```

(optional) 通过分析pattern信息计算预测准确性
```bash
result_file= # 输出预测准确性统计信息
build/mpr --validate -trace=${mpr_trace_file}.gz -pattern=${pattern_file} -result=${result_file} 2>${result_dir}/${app}/valid_err.txt
```

(optional) 通过parser分析PC类别以及miss情况并给出对应源代码
```bash
out_file= # 输出最终的csv文件（注意csv文件以'\t'分隔而非','，并不能直接打开，可以拷贝到excel，再进行分列）
build/pattern2line ${miss_file} ${pattern_file} ${out_file} ${binary_file} 2>err.txt
```

最终看的就是stat_file和out_file。

# Misc

整个项目大概就这些，编译链接由CMake驱动
```
mem-pattern-recognition/
├── CMakeLists.txt
├── external
│   ├── ChampSim
│   └── gflags
├── include
│   ├── instruction.h
│   ├── pattern.h
│   ├── pattern_list.h
│   ├── pc_meta.h
│   ├── trace_list.h
│   └── tracereader.h
├── README.md
├── scripts
│   ├── build.sh
│   ├── make_trace.sh
│   ├── run_app.sh
│   ├── run_apps.sh
│   ├── run_app_with_roi.sh
│   ├── run_spec.sh
│   ├── run_spec_with_roi.sh
│   └── test_trace.sh
├── src
│   ├── main.cpp
│   ├── pattern_list.cpp
│   ├── pc_meta.cpp
│   ├── trace_list.cpp
│   ├── tracereader.cpp
│   └── utils
│       ├── pattren2line.cpp
│       └── perf-parse.cpp
├── test-cases
│   ├── chain
│   │   └── chain.c
│   ├── indirect
│   │   └── indirect.c
│   ├── pointera
│   │   └── pointera.c
│   ├── pointerb
│   │   └── pointerb.c
│   └── stride
│       └── stride.c
└── tracer
    ├── clean_tracer.sh
    ├── makefile
    ├── makefile.rules
    ├── make_roi_tracer.sh
    ├── make_union_tracer.sh
    ├── union_tracer.cpp
    └── union_tracer_with_roi.cpp
```