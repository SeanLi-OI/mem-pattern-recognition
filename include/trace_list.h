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

// #define DEBUG_HISTORY
#define ENABLE_TIMER

#ifdef ENABLE_TIMER
#include <chrono>
#endif

const int PATTERN_NUM = 9;
const std::string PATTERN_NAME[] = {"fresh\t",  "static\t", "stride\t",
                                    "pointer",  "pointerA", "pointerB",
                                    "indirect", "chain\t",  "other\t"};
enum PATTERN : uint16_t {
  FRESH,
  STATIC,
  STRIDE,
  pointer,
  POINTER_A,
  POINTER_B,
  INDIRECT,
  CHAIN,
  OTHER
};
template <typename E>
constexpr auto to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

inline unsigned long long int abs_sub(unsigned long long int a,
                                      unsigned long long int b) {
  return a > b ? a - b : b - a;
}

const uint32_t INTERVAL = 128;
const uint16_t STRIDE_THERSHOLD = 32;
const uint16_t POINTER_A_THERSHOLD = 32;
const uint16_t INDIRECT_THERSHOLD = 32;
const uint16_t PATTERN_THERSHOLD = 32;
// const uint16_t CHAIN_THERSHOLD = 32;

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

  class PCmeta {  // Metadata for each PC
   public:
    // INDIRECT
    struct pc_value_meta {
      unsigned long long int value;
      unsigned long long int addr;
      int offset;
      int confidence;
      pc_value_meta() {}
      pc_value_meta(unsigned long long int _v, unsigned long long int _a)
          : value(_v), addr(_a) {
        offset = 0;
        confidence = 0;
      }
    };
    std::unordered_map<unsigned long long int, pc_value_meta>
        pc_value_candidate;

    // CHAIN
    // std::unordered_map<unsigned long long int, int>
    //     offset_candidate;  // offset, confidence
    std::set<unsigned long long int> offset_candidate;
    unsigned long long int offset;

    // pointer
    std::set<unsigned long long int> lastpc_candidate;
    unsigned long long int lastpc;

    // pointerA
    unsigned long long int pointerA_offset_candidate;
    unsigned long long int lastvalue;
    int pointerA_confidence;

    // STATIC & STRIDE
    unsigned long long int lastaddr;

    // STRIDE
    unsigned long long int offset_stride;
    int stride_confidence;

    // common
    PATTERN pattern;
    unsigned long long int count;
    bool confirm;
    long long pattern_confidence[PATTERN_NUM];
    // std::vector<int> inst_id_list;
    PCmeta() {
      offset = lastaddr = lastpc = confirm = offset_stride = stride_confidence =
          0;
      pattern = PATTERN::OTHER;
      for (int i = 0; i < PATTERN_NUM; i++) pattern_confidence[i] = 0;
      count = 1;
    }
    bool is_stride() { return stride_confidence >= STRIDE_THERSHOLD; }
  };
  std::unordered_map<unsigned long long int, std::deque<TraceNode>> value2trace;
  std::deque<TraceNode> traceHistory;

  unsigned long long int pattern_count[PATTERN_NUM];
  std::unordered_map<unsigned long long int, PCmeta> pc2meta;
#ifdef ENABLE_TIMER
  unsigned long long int total_time;
#endif

  std::fstream out;

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
  void add_outfile(char filename[]) {
    out.open(filename, std::ios::out);
    assert(out);
  }
  void add_trace(unsigned long long int pc, unsigned long long int addr,
                 unsigned long long int value, bool isWrite,
                 unsigned long long int id, const int inst_id);
  void printStats(int totalCnt, char *filename);
};

#endif