//Author: Lixiang

#ifndef PERF_PARSE_CYCLE_H
#define PERF_PARSE_CYCLE_H
#include <sstream>

#include "perf-parse/perf-parse-common.h"
void parse_cycle(std::string str) {
  std::istringstream in;
  std::string str_offset;
  unsigned long long pc, pc_offset, pc_base;
  in.str(str);
  in >> std::hex >> pc >> str_offset;
  if (pc == 0 || str_offset[0] == '[') return;
  std::size_t pos = str_offset.find("+");
  if (pos == std::string::npos) return;
  in.str(str_offset.substr(pos + 1));
  in >> std::hex >> pc_offset;
  pc_base = pc - pc_offset;
  auto it = pc_cnt.find(pc_base);
  totalCnt++;
  if (it == pc_cnt.end()) {
    pc_cnt[pc_base] =
        PC_block(pc_offset, 1, str_offset.substr(0, str_offset.find("+")));
  } else {
    it->second.max_offset = std::max(it->second.max_offset, pc_offset);
    it->second.counter++;
  }
}
#endif
