# Code List
``` bash
mem-pattern-recognition
├──include/
│   ├── conf_counter.h // 置信计数器的定义
│   ├── instruction.h // trace格式的定义
│   ├── pattern.h // 各个访存模式的基本信息
│   ├── pattern_list.h // 访存模式验证器的头文件
│   ├── pc_meta.h // 分析器中每个PC的元数据定义
│   ├── trace_list.h // 访存模式识别器的头文件
│   ├── tracereader.h // trace读取器的头文件
│   └── utils
│       ├── LRUqueue.h // LRU队列
│       ├── macro.h // 一些通用的宏定义
│       ├── MCMF.h // 最小费用流算法
│       └── monoQueue.h // 单调队列
├── src
│   ├── main.cpp // 访存模式分析工具主程序
│   ├── pattern_list.cpp // 访存模式验证器的实现
│   ├── pc_meta.cpp // 分析器中每个PC的元数据的输入输出实现
│   ├── trace_list.cpp // 访存模式识别器的实现
│   ├── tracereader.cpp // trace读取器的实现
│   └── utils
│       ├── choose_ckp.cpp // 访存踪迹抓取加速算法
│       ├── macro.cpp // 一些通用的函数实现
│       ├── merge-result.cpp // 合并多个检查点的识别结果
│       ├── parse-merged-result.cpp // 解析合并后结果
│       └── pattern2line.cpp // PC元数据的统一输入输出接口
└── test-cases // 访存模式生成器
│   ├── random // 区域随机型访存模式生成器
│   │   └── random.c
│   ├── indirect // 间接型访存模式生成器
│   │   └── indirect.c
│   ├── pointera // 指针追逐型访存模式生成器
│   │   └── pointera.c
│   ├── pointerb // 指针数组型访存模式生成器
│   │   └── pointerb.c
│   └── stride // 跨步型访存模式生成器
│       └── stride.c
└── tracer // 访存踪迹抓取器
    ├── gem5 // 基于Gem5实现的访存踪迹抓取器
    │   └── ...
    ├── qemu // 基于QEMU+Cannoli实现的访存踪迹抓取器
    │   └── ...
    └── union_tracer_with_roi.cpp // 基于Pintool接口实现的访存踪迹抓取器
```

# Compile from Source

**IMPORTANT: Please make sure your g++ supports c++17 standard.**

``` bash
${mpr_dir}=WHERE_TO_PUT_MPR
MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"

git clone https://github.com/SeanLi-OI/mem-pattern-recognition.git ${mpr_dir}
cd ${mpr_dir}
git submodule update --recursive

# build glog
cd ${mpr_dir}/external/glog/
mkdir build && cd build
cmake3 ../ -DBUILD_SHARED_LIBS=OFF
make ${MAKEFLAGS}

# build gflags
cd ${mpr_dir}/external/gflags/
mkdir build && cd build
cmake3 ../
make ${MAKEFLAGS}

cd ${mpr_dir}
mkdir build && cd build
make ${MAKEFLAGS}
```


# Demo

以五种访存模式生成器为基础
跑通：（1）生成trace（2）分析pattern（3）模拟miss（4）输出模式识别统计信息（5）统计覆盖率与准确率；这一整个pipeline
```bash
# build Champsim (optional)
cd ${mpr_dir}/external/ChampSim
./config.sh champsim_config.json
make

# build union_tracer
cd ../../tracer
./make_union_tracer.sh

# run mpr
cd ../
./scripts/run_apps.sh
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

# ROI extension
* 本项目支持分析应用ROI(参见scripts/analyze_roi.sh)，生成roi_raw.txt（三列，分别为ROI的起始地址，结束地址，以及运行次数，以空格分隔）后，需要人工从中挑选ROI区域给出roi.txt（两列，分别为ROI的起始地址和结束地址，以空格分隔）。

* 本项目同样支持仅抓取ROI区域trace（参见scripts/run_app_with_roi.sh），其中需要上述roi.txt（也可依照格式自定义ROI区域）。



# Misc

整个项目编译链接由CMake驱动
```
mem-pattern-recognition/
├── build.sh
├── CMakeLists.txt
├── external
│   ├── ChampSim
│   └── gflags
├── include
│   ├── instruction.h
│   ├── macro.h
│   ├── pattern.h
│   ├── pattern_list.h
│   ├── pc_meta.h
│   ├── trace_list.h
│   └── tracereader.h
├── README.md
├── scripts
│   ├── analyze_roi.sh
│   ├── make_trace.sh
│   ├── run_app.sh
│   ├── run_apps.sh
│   ├── run_app_with_roi.sh
│   ├── run_binary.sh
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
│       ├── pattern2line.cpp
│       └── perf-parse.cpp
├── test-cases
│   ├── indirect
│   │   └── indirect.c
│   ├── pointera
│   │   └── pointera.c
│   ├── pointerb
│   │   └── pointerb.c
│   ├── stride
│   │   └── stride.c
│   └── struct_pointer
│       └── struct_pointer.c
└── tracer
    ├── clean_tracer.sh
    ├── makefile
    ├── makefile.rules
    ├── make_roi_tracer.sh
    ├── make_union_tracer.sh
    ├── union_tracer.cpp
    └── union_tracer_with_roi.cpp
```

# Contributor

Xiang Li (lixiang99410@gmail.com)
