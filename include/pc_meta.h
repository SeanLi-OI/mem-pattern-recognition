//Author: Lixiang

#ifndef PC_META_H
#define PC_META_H

#include <fstream>
#include <set>
#include <unordered_map>

#include "conf_counter.h"
#include "pattern.h"
#include "utils/LRUqueue.h"
#include "utils/monoQueue.h"

class PCmeta {  // Metadata for each PC
 public:
  // INDIRECT
#ifdef OLD_INDIRECT_PATTERN
  struct pc_value_meta {
    unsigned long long int value;
    unsigned long long int addr;
    long long offset;
    ConfCounter confidence;
    pc_value_meta() {}
    pc_value_meta(unsigned long long int _v, unsigned long long int _a)
        : value(_v), addr(_a) {
      offset = 0;
    }
  };
  std::unordered_map<unsigned long long int, pc_value_meta> pc_value_candidate;
  std::set<unsigned long long int> meeted_pc;
  pc_value_meta pc_value;  // value(PC), addr(base_addr), offset(offset)
#else
  struct pc_value_meta {
    unsigned long long int pc, prev_pc;
    long long offset;
    ConfCounter confidence;
    pc_value_meta() {}
    pc_value_meta(unsigned long long int _p, long long _o,
                  unsigned long long int _pp)
        : pc(_p), offset(_o), prev_pc(_pp) {
      confidence.reset();
    }
  };
  std::unordered_map<unsigned long long int, pc_value_meta> pc_value_candidate;
  std::set<unsigned long long int> meeted_pc;
  unsigned long long int lastvalue_2;
  pc_value_meta pc_value;
#endif

  // STRUCT_POINTER
  struct struct_pointer_meta {
    unsigned long long int pc;
    long long offset;
    int confidence;
    bool flag;
    struct_pointer_meta() {}
    struct_pointer_meta(unsigned long long int _p) : pc(_p) {
      flag = false;
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
  ConfCounter pointerA_tmp;

  // STATIC & STRIDE & HEAP
  unsigned long long int lastaddr, lastaddr_2;
  ConfCounter static_tmp;

  // HEAP
  unsigned long long int base_addr_for_heap, common_ratio_for_heap, idx_for_heap;
  short heap_flag;

  // STRIDE
  long long int offset_stride;
  ConfCounter stride_tmp;
  short stride_flag;

  // LOCALITY
  MonoQueue<long long int, LOCALITY_LEN> monoQueue;

  // RANDOM
  LRUqueue<long long int, RANDOM_LEN> offset_history;

  // common
  PATTERN pattern;
  unsigned long long int count;
  bool confirm;
  ConfCounter pattern_confidence[PATTERN_NUM];
  bool is_not_pattern[PATTERN_NUM];
  bool maybe_pattern[PATTERN_NUM];
  bool maybe_pointer_chase;

  // for merge
  std::string file_id;
  // std::vector<int> inst_id_list;
  PCmeta() {
    lastaddr = lastaddr_2 = 0;
    lastpc = 0;
    confirm = 0;
    offset_stride = 0;
    pointerA_offset_candidate = -1;
    maybe_pointer_chase = 0;
    pattern = PATTERN::OTHER;
    stride_flag = false;
    base_addr_for_heap = 0;
    common_ratio_for_heap = 0;
    idx_for_heap = 0;
    for (int i = 0; i < PATTERN_NUM; i++) {
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