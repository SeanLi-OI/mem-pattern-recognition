#ifndef PATTERN_LIST_H
#define PATTERN_LIST_H

#include <assert.h>

#include <unordered_map>

#include "pc_meta.h"

class PatternList {
 private:
  std::unordered_map<unsigned long long, PCmeta> pc2meta;
  std::unordered_map<unsigned long long, unsigned long long> next_addr;
  unsigned long long hit_count[PATTERN_NUM], total_count[PATTERN_NUM];

 public:
  PatternList(const char filename[]);
  void add_trace(unsigned long long int pc, unsigned long long int addr,
                 unsigned long long int value, bool isWrite,
                 unsigned long long int id, const int inst_id);
  void printStats(int totalCnt, const char filename[]);
};

#endif