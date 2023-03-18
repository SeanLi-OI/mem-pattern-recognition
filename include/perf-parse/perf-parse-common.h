//Author: Lixiang

#ifndef PERF_PARSE_COMMON_H
#define PERF_PARSE_COMMON_H

#include <string>
#include <unordered_map>

struct PC_block {
  unsigned long long max_offset;
  unsigned long long counter;
  std::string func_name;
  PC_block() { max_offset = counter = 0; }
  PC_block(unsigned long long _offset, unsigned long long _count,
           std::string _func_name = "")
      : max_offset(_offset), counter(_count), func_name(_func_name) {}
};
std::unordered_map<unsigned long long, PC_block> pc_cnt;
unsigned long long totalCnt;

#endif