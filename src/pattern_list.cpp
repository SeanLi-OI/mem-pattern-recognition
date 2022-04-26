// Author: Lixiang

#include "pattern_list.h"

#include <fstream>
#include <iostream>

PatternList::PatternList(const char filename[]) {
  std::ifstream in(filename);
  unsigned long long pc;
  while (in >> std::hex >> pc) {
    pc2meta[pc] = PCmeta();
    pc2meta[pc].input(in);
  }
  std::fill(hit_count, hit_count + PATTERN_NUM, 0);
  std::fill(total_count, total_count + PATTERN_NUM, 0);
}

void PatternList::add_trace(unsigned long long int pc,
                            unsigned long long int addr,
                            unsigned long long int value, bool isWrite,
                            unsigned long long int id, const int inst_id) {
  auto it_meta = pc2meta.find(pc);
  if (it_meta == pc2meta.end()) {
    std::cerr << "Cannot find PC " << pc << " in pattern list." << std::endl;
    return;
  }
  auto it = next_addr.find(pc);
  if (it != next_addr.end()) {
    total_count[to_underlying(it_meta->second.pattern)]++;
    if (it->second == addr) hit_count[to_underlying(it_meta->second.pattern)]++;
  }
  if (it_meta->second.pattern == PATTERN::POINTER_A) {
    next_addr[pc] = value + it_meta->second.pointerA_offset_candidate;
  }
}
void PatternList::printStats(int totalCnt, const char filename[]) {
  std::ofstream out(filename);
  int hit_sum = 0, total_sum = 0;
  for (int i = 0; i < PATTERN_NUM; i++) {
    out << hit_count[i] << " " << total_count[i] << std::endl;
    hit_sum += hit_count[i];
    total_sum += total_count[i];
  }
  out << "Fresh : " << totalCnt - total_sum << std::endl;
  out << "Hit   : " << hit_sum << std::endl;
  out << "Miss  : " << total_sum - hit_sum << std::endl;
}