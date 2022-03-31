#ifndef TRACE_LIST_H
#define TRACE_LIST_H

#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>

#define DEBUG_VALUE
// #define DEBUG_HISTORY
#define ENABLE_TIMER

#ifdef ENABLE_TIMER
#include <chrono>
#endif

const int PATTERN_NUM = 8;
const std::string PATTERN_NAME[] = {"fresh\t",  "static\t", "stride\t",
                                    "pointerA", "pointerB", "indirect",
                                    "chain\t",  "other\t"};
enum PATTERN : uint16_t {
  FRESH,
  STATIC,
  STRIDE,
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

const uint32_t INTERVAL = 32;

class TraceList {
  struct TraceNode {
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

  struct PCmeta {
    // INDIRECT
    std::unordered_map<
        unsigned long long int,
        std::pair<unsigned long long int, unsigned long long int>>
        pc_value_candidate;

    // CHAIN
    std::set<unsigned long long int> offset_candidate;
    unsigned long long int offset;

    // POINTER_A
    std::set<unsigned long long int> lastpc_candidate;
    unsigned long long int lastpc;

    // STATIC & STRIDE
    unsigned long long int lastaddr;

    // common
    PATTERN pattern;
    unsigned long long int count;
    bool confirm;
    PCmeta() {
      offset = lastaddr = lastpc = confirm = 0;
      pattern = PATTERN::OTHER;
      count = 1;
    }
  };
  std::unordered_map<unsigned long long int, std::deque<TraceNode>> value2trace;
  std::deque<TraceNode> traceHistory;

  unsigned long long int pattern_count[PATTERN_NUM];
  std::unordered_map<unsigned long long int, PCmeta> pc2meta;
#ifdef ENABLE_TIMER
  unsigned long long int total_time;
#endif

  void erase_before(std::deque<TraceNode> &L, const unsigned long long int &id);
  void add_next(std::deque<TraceNode> &L, TraceNode tn);

  bool check_static_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &pc, unsigned long long int &addr);
  bool check_stride_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      unsigned long long int &pc, unsigned long long int &addr);
  bool check_pointerA_pattern(
      std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
      std::unordered_map<unsigned long long int,
                         std::deque<TraceNode>>::iterator &it_val,
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
  void add_trace(unsigned long long int pc, unsigned long long int addr,
                 unsigned long long int value, bool isWrite,
                 unsigned long long int id);
  void printStats(int totalCnt);
};

#endif