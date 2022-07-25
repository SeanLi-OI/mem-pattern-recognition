#ifndef PATTERN_LIST_H
#define PATTERN_LIST_H

#include <unordered_map>

#include "instruction.h"
#include "pc_meta.h"

class PatternList {
 private:
  std::unordered_map<unsigned long long, PCmeta> pc2meta;
  std::unordered_map<unsigned long long, unsigned long long> next_addr;
  std::unordered_map<unsigned long long, unsigned long long> indirect_base_addr;
  std::unordered_map<unsigned long long, unsigned long long> pc2id;
  std::unordered_map<unsigned long long, unsigned long long> miss_count;
  unsigned long long hit_count[PATTERN_NUM];
  unsigned long long total_count[PATTERN_NUM];
  unsigned long long all_count[PATTERN_NUM];

 public:
  PatternList(const char filename[]);
  void add_trace(unsigned long long int pc, unsigned long long int addr,
                 unsigned long long int value, bool isWrite,
                 unsigned long long int &id, const int inst_id);
  void printStats(unsigned long long totalCnt, const char filename[]);
};

void valid_trace(PatternList &patternList, unsigned long long &id,
                 unsigned long long int ip, MemRecord &r, bool isWrite,
                 const int inst_id);

#endif