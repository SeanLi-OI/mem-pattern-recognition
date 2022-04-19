# Demo

以五种访存模式生成器为基础
跑通：（1）生成trace（2）分析pattern（3）模拟miss（4）最终输出统计信息
这一整个pipeline
```bash
# 需要预先build champsim，并将champsim所在文件夹填写在run_app.sh中
cd tracer
./make_union_tracer.sh # build union_tracer
cd ../result
g++ -O3 -std=c++11 parse.cpp -o parse # build parse
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
pin -t /home/lixiang/mem-pattern-recognition/tracer/obj-intel64/union_tracer.so -o ${champsim_trace_file} -m ${mpr_trace_file} -t ${trace_len} -- ${binary_file} <${app_input}
gzip -c ${champsim_trace_file} > ${champsim_trace_file}.gz # champsim只支持gz/xz压缩文件
```

通过mpr分析各PC的类别
```bash
stat_file= # 统计不同类别的PC数和指令数
pattern_file= # 分别给出每个PC的类别，以及执行次数
/home/lixiang/mem-pattern-recognition/bin/mpr ${mpr_trace_file} ${stat_file} ${pattern_file} 2>err.txt
```

(optional) 通过champsim获取miss情况
```bash
miss_file= # 输出miss信息
/home/lixiang/ChampSim/bin/champsim --warmup_instructions 0 --simulation_instructions ${trace_len} ${champsim_trace_file} 2>${miss_file}
```

(optional) 通过parser分析PC类别以及miss情况并给出对应源代码
```bash
out_file= # 输出最终的csv文件（注意csv文件以'\t'分隔而非','，并不能直接打开，可以拷贝到excel，再进行分列）
/home/lixiang/mem-pattern-recognition/result/parse ${miss_file} ${pattern_file} ${out_file} ${binary_file} 2>err.txt
```

最终看的就是stat_file和out_file。

# Misc

整个项目大概就这些，编译链接由Makefile驱动，本来想把魔改的champsim作为submodule搞进来，但懒得搞了，就这样凑合先用把
```
mem-Pattern-Recognition/
├── include
│   ├── circular_buffer.hpp
│   ├── instruction.h
│   ├── trace_list.h
│   └── tracereader.h
├── Makefile
├── README.md
├── result
│   └── parse.cpp
├── test-cases
│   ├── chain
│   │   └── chain.cpp
│   ├── indirect
│   │   └── indirect.cpp
│   ├── pointera
│   │   └── pointera.cpp
│   ├── pointerb
│   │   └── pointerb.cpp
│   └── stride
│       └── stride.cpp
├── src
│   ├── main.cpp
│   ├── trace_list.cpp
│   └── tracereader.cpp
└── tracer
    ├── clean_tracer.sh
    ├── makefile
    ├── makefile.rules
    ├── make_union_tracer.sh
    └── union_tracer.cpp
```