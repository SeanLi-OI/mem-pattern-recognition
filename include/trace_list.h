#ifndef TRACE_LIST_H
#define TRACE_LIST_H

#include <assert.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "pattern.h"
#include "pc_meta.h"

// #define DEBUG_HISTORY
#define ENABLE_TIMER

#ifdef ENABLE_TIMER
#include <chrono>
#endif

class TraceList {
  struct TraceNode {  // Single Memory Access
    unsigned long long int id;
    unsigned long long int pc;
    unsigned long long int addr;
    unsigned long long int value;
    bool isWrite;
    TraceNode() {}
    TraceNode(unsigned long long int _p, unsigned long long int _a,
              unsigned long long int _v, bool _i, unsigned long long int _id)
        : pc(_p), addr(_a), value(_v), isWrite(_i), id(_id) {}
  };

  std::unordered_map<unsigned long long int, std::deque<TraceNode>> value2trace;
  std::deque<TraceNode> traceHistory;

  unsigned long long int pattern_count[PATTERN_NUM];
  std::unordered_map<unsigned long long int, PCmeta> pc2meta;
#ifdef ENABLE_TIMER
  unsigned long long int total_time;
#endif

  std::ofstream out;

  void erase_before(std::deque<TraceNode> &L, const unsigned long long int &id);
  void add_next(std::deque<TraceNode> &L, TraceNode tn);

  bool check_static_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &pc, unsigned long long int &addr);
  bool check_stride_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &pc, unsigned long long int &addr);
  bool check_pointer_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      std::unordered_map<unsigned long long int,
                         std::deque<TraceNode>>::iterator &it_val,
      unsigned long long int &addr);
  bool check_pointerA_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &addr);
  bool check_pointerB_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &addr);
  bool check_indirect_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &addr);
  bool check_chain_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &addr);

 public:
  TraceList() {
    for (int i = 0; i < PATTERN_NUM; i++) pattern_count[i] = 0;
#ifdef ENABLE_TIMER
    total_time = 0;
#endif
  }
  void add_outfile(const char filename[]) {
    out.open(filename);
    assert(out);
  }
  void add_trace(unsigned long long int pc, unsigned long long int addr,
                 unsigned long long int value, bool isWrite,
                 unsigned long long int id, const int inst_id);
  void printStats(int totalCnt, const char filename[]);
};

#endif