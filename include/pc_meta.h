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
  std::set<unsigned long long int> meeted_pc;
  pc_value_meta pc_value;  // value(PC), addr(base_addr), offset(offset)

  // STRUCT_POINTER
  struct struct_pointer_meta {
    unsigned long long int pc;
    long long offset;
    int confidence;
    bool flag;
    struct_pointer_meta() {}
    struct_pointer_meta(unsigned long long int _p) : pc(_p) {
      flag = 0;
      offset = 0;
      confidence = 0;
    }
  };
  std::unordered_map<unsigned long long int, struct_pointer_meta>
      struct_pointer_candidate;  // PC-> <PC, offset>
  std::set<unsigned long long int> meeted_pc_sp;
  long long offset_sp;
  unsigned long long last_pc_sp;

  // pointer
  std::set<unsigned long long int> lastpc_candidate;
  unsigned long long int lastpc;

  // pointerA
  long long int pointerA_offset_candidate;
  unsigned long long int lastvalue;
  long long pointerA_tmp;

  // STATIC & STRIDE
  unsigned long long int lastaddr, lastaddr_2;
  long long static_tmp;

  // STRIDE
  long long int offset_stride;
  long long stride_tmp;

  // common
  PATTERN pattern;
  unsigned long long int count;
  bool confirm;
  long long pattern_confidence[PATTERN_NUM];
  bool is_not_pattern[PATTERN_NUM];
  bool maybe_pattern[PATTERN_NUM];
  bool maybe_pointer_chase;
  // std::vector<int> inst_id_list;
  PCmeta() {
    lastaddr = lastaddr_2 = 0;
    lastpc = 0;
    confirm = 0;
    offset_stride = 0;
    stride_tmp = 0;
    pointerA_tmp = 0;
    static_tmp = 0;
    pointerA_offset_candidate = -1;
    maybe_pointer_chase = 0;
    pattern = PATTERN::OTHER;
    for (int i = 0; i < PATTERN_NUM; i++) {
      pattern_confidence[i] = 0;
      maybe_pattern[i] = false;
      is_not_pattern[i] = false;
    }
    count = 1;
  }
  bool is_pattern(PATTERN pattern) {
    return maybe_pattern[to_underlying(pattern)] &&
           !is_not_pattern[to_underlying(pattern)];
  }
  void input(std::ifstream &in);
  void output(std::ofstream &out);
};

#endif