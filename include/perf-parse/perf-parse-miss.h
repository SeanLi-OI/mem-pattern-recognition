//Author: Lixiang

#ifndef PERF_PARSE_MISS_H
#define PERF_PARSE_MISS_H
#include <glog/logging.h>

#include <sstream>

#include "perf-parse/perf-parse-common.h"
void parse_miss(std::string str) {
  auto key_pos = str.find("cache-misses");
  if (key_pos == std::string::npos) return;
  /******Get PC_base **********/
  auto pos = key_pos + 13;
  while (pos < str.length() && !((str[pos] >= '0' && str[pos] <= '9') ||
                                 (str[pos] >= 'a' && str[pos] <= 'f')))
    pos++;
  LOG_IF(FATAL, pos == str.length());
  std::istringstream in(str.substr(pos));
  unsigned long long pc, pc_offset, pc_base, count;
  std::string str_offset;
  in >> std::hex >> pc >> str_offset;
  /******Get PC_offset ********/
  pos = str_offset.find("+");
  if (pos == std::string::npos) return;
  in.str(str_offset.substr(pos + 1));
  in >> std::hex >> pc_offset;
  pc_base = pc - pc_offset;

  /******Get Miss Count********/
  pos = key_pos - 1;
  while (pos >= 0 && !(str[pos] >= '0' && str[pos] <= '9')) pos--;
  LOG_IF(FATAL, pos < 0);
  while (pos >= 0 && str[pos] >= '0' && str[pos] <= '9') pos--;
  in.clear();
  in.str(str.substr(pos + 1));
  in >> std::dec >> count;

  auto it = pc_cnt.find(pc_base);
  totalCnt += count;
  if (it == pc_cnt.end()) {
    pc_cnt[pc_base] =
        PC_block(pc_offset, count, str_offset.substr(0, str_offset.find("+")));
  } else {
    it->second.max_offset = std::max(it->second.max_offset, pc_offset);
    it->second.counter += count;
  }
}
#endif
