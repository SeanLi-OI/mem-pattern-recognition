#ifndef PC_META_H
#define PC_META_H

#include <fstream>
#include <set>
#include <unordered_map>

#include "pattern.h"

class PCmeta {  // Metadata for each PC
 public:
  // INDIRECT
  struct pc_value_meta {
    unsigned long long int value;
    unsigned long long int addr;
    long long offset;
    int confidence;
    pc_value_meta() {}
    pc_value_meta(unsigned long long int _v, unsigned long long int _a)
        : value(_v), addr(_a) {
      offset = 0;
      confidence = 0;
    }
  };
  std::unordered_map<unsigned long long int, pc_value_meta> pc_value_candidate;
  pc_value_meta pc_value;  // value(PC), addr(base_addr), offset(offset)

  // CHAIN
  std::unordered_map<unsigned long long int, std::pair<long long int, int>>
      chain_candidate;  // PC -> <offset, condifence>

  // pointer
  std::set<unsigned long long int> lastpc_candidate;
  unsigned long long int lastpc;

  // pointerA
  long long int pointerA_offset_candidate;
  unsigned long long int lastvalue;
  int pointerA_confidence;

  // STATIC & STRIDE
  unsigned long long int lastaddr;
  unsigned long long int lastaddr_2;

  // STRIDE
  long long int offset_stride;
  int stride_confidence;

  // common
  PATTERN pattern;
  unsigned long long int count;
  bool confirm;
  long long pattern_confidence[PATTERN_NUM];
  // std::vector<int> inst_id_list;
  PCmeta() {
    lastaddr = 0;
    lastaddr_2 = 0;
    lastpc = 0;
    confirm = 0;
    offset_stride = 0;
    stride_confidence = 0;
    pointerA_offset_candidate = -1;
    pattern = PATTERN::OTHER;
    for (int i = 0; i < PATTERN_NUM; i++) pattern_confidence[i] = 0;
    count = 1;
  }
  bool is_stride() { return stride_confidence >= STRIDE_THERSHOLD; }
  bool is_pointerA() { return pattern == PATTERN::pointer; }
  void input(std::ifstream &in);
  void output(std::ofstream &out);
};

#endif