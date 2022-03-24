#include <algorithm>
#include <functional>
#include <queue>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "tracereader.h"

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

const uint32_t INTERVAL = 32;

class TraceList {
  struct TraceNode {
    uint64_t id;
    uint64_t pc;
    uint64_t addr;
    uint64_t value;
    bool isWrite;
    TraceNode() {}
    TraceNode(uint64_t _p, uint64_t _a, uint64_t _v, bool _i, uint64_t _id)
        : pc(_p), addr(_a), value(_v), isWrite(_i), id(_id) {}
  };

  struct PCmeta {
    // INDIRECT
    std::unordered_map<uint64_t, std::pair<uint64_t, uint64_t>>
        pc_value_candidate;

    // CHAIN
    std::set<uint64_t> offset_candidate;
    uint64_t offset;

    // POINTER_A
    std::set<uint64_t> lastpc_candidate;
    uint64_t lastpc;

    // STATIC & STRIDE
    uint64_t lastaddr;

    // common
    PATTERN pattern;
    uint64_t count;
    bool confirm;
    PCmeta() {
      offset = lastaddr = lastpc = confirm = 0;
      pattern = PATTERN::OTHER;
      count = 1;
    }
  };
  std::unordered_map<uint64_t, std::deque<TraceNode>> value2trace;
  std::deque<TraceNode> traceHistory;

  uint64_t pattern_count[PATTERN_NUM];
  std::unordered_map<uint64_t, PCmeta> pc2meta;
#ifdef ENABLE_TIMER
  uint64_t total_time;
#endif

  void erase_before(std::deque<TraceNode> &L, const uint64_t &id) {
#ifndef DEBUG_ALL
    while (!L.empty() && L.front().id < id - INTERVAL) L.pop_front();
#endif
  }
  void add_next(std::deque<TraceNode> &L, TraceNode tn) { L.push_back(tn); }

  bool check_static_pattern(
      std::unordered_map<uint64_t, PCmeta>::iterator &it_meta, uint64_t &pc,
      uint64_t &addr) {
    if (pc2meta[pc].lastaddr == addr) return true;
    return false;
  }
  bool check_stride_pattern(
      std::unordered_map<uint64_t, PCmeta>::iterator &it_meta, uint64_t &pc,
      uint64_t &addr) {
    auto offset = abs(it_meta->second.lastaddr - addr);
    if (offset <= 16) std::cerr << offset << std::endl;
    if (offset == 4 || offset == 8 || offset == 16 || offset == 32) return true;
    return false;
  };
  bool check_pointerA_pattern(
      std::unordered_map<uint64_t, PCmeta>::iterator &it_meta,
      std::unordered_map<uint64_t, std::deque<TraceNode>>::iterator &it_val,
      uint64_t &addr) {
    if (it_meta->second.lastpc_candidate.empty()) {
      for (auto it = it_val->second.rbegin(); it != it_val->second.rend();
           it++) {
        it_meta->second.lastpc_candidate.insert(it->pc);
      }
    } else {
      for (auto it = it_val->second.rbegin(); it != it_val->second.rend();
           it++) {
        if (it_meta->second.lastpc_candidate.find(it->pc) !=
            it_meta->second.lastpc_candidate.end()) {
          it_meta->second.lastpc_candidate.clear();
          it_meta->second.lastpc = it->pc;
          it_meta->second.confirm = true;
          return true;
        }
      }
    }
    return false;
  }
  bool check_pointerB_pattern(
      std::unordered_map<uint64_t, PCmeta>::iterator &it_meta, uint64_t &addr) {
    if (pc2meta[it_meta->second.lastpc].pattern == PATTERN::STRIDE) return true;
    return false;
  }
  bool check_indirect_pattern(
      std::unordered_map<uint64_t, PCmeta>::iterator &it_meta, uint64_t &addr) {
    if (it_meta->second.offset_candidate.empty()) {
      for (auto trace : traceHistory) {
        if (pc2meta[trace.pc].pattern == PATTERN::STRIDE) {
          it_meta->second.pc_value_candidate[trace.pc] =
              std::make_pair(trace.value, addr);
        }
      }
    } else {
      for (auto trace : traceHistory) {
        if (pc2meta[trace.pc].pattern == PATTERN::STRIDE) {
          auto it = it_meta->second.pc_value_candidate.find(trace.pc);
          if (it != it_meta->second.pc_value_candidate.end() &&
              addr != it->second.second && trace.value != it->second.first &&
              (addr - it->second.second) % (trace.value - it->second.first) ==
                  0) {
            it_meta->second.pc_value_candidate.clear();
            it_meta->second.confirm = true;
            return true;
          }
        }
      }
    }
  }
  bool check_chain_pattern(
      std::unordered_map<uint64_t, PCmeta>::iterator &it_meta, uint64_t &addr) {
    if (it_meta->second.offset_candidate.empty()) {
      for (auto trace : traceHistory) {
        if (pc2meta[trace.pc].pattern == PATTERN::POINTER_A) {
          it_meta->second.offset_candidate.insert(abs(trace.value - addr));
        }
      }
    } else {
      for (auto trace : traceHistory) {
        if (pc2meta[trace.pc].pattern == PATTERN::POINTER_A &&
            it_meta->second.offset_candidate.find(abs(trace.value - addr)) !=
                it_meta->second.offset_candidate.end()) {
          it_meta->second.offset_candidate.clear();
          it_meta->second.offset = abs(trace.value - addr);
          it_meta->second.confirm = true;
          return true;
        }
      }
    }
    return false;
  }

 public:
  TraceList() {
    for (int i = 0; i < PATTERN_NUM; i++) pattern_count[i] = 0;
#ifdef ENABLE_TIMER
    total_time = 0;
#endif
  }
  void add_trace(uint64_t pc, uint64_t addr, uint64_t value, bool isWrite,
                 uint64_t id) {
    TraceNode tn(pc, addr, value, isWrite, id);
    auto it_val = value2trace.find(addr);
    {  // value2trace
      if (it_val != value2trace.end()) {
#ifndef DEBUG_ADDR
        erase_before(it_val->second, id);
#endif
      } else {
        value2trace[addr] = std::deque<TraceNode>();
        it_val = value2trace.find(addr);
      }
    }
#ifndef DEBUG_HISTORY
    erase_before(traceHistory, id);
#endif
    auto it_meta = pc2meta.find(pc);
    if (it_meta == pc2meta.end()) {
      pc2meta[pc] = PCmeta();
      it_meta = pc2meta.find(pc);
    } else {
      it_meta->second.count++;
#ifdef ENABLE_TIMER
      auto t1 = std::chrono::high_resolution_clock::now();
#endif
      if (it_meta->second.pattern == PATTERN::OTHER) {
        PATTERN pattern;
        for (bool X = true; X; X = false) {
          if (check_static_pattern(it_meta, pc, addr)) {
            it_meta->second.pattern = PATTERN::STATIC;
            break;
          }
          if (check_stride_pattern(it_meta, pc, addr)) {
            it_meta->second.pattern = PATTERN::STRIDE;
            break;
          }
          if (check_pointerA_pattern(it_meta, it_val, addr)) {
            it_meta->second.pattern = PATTERN::POINTER_A;
            break;
          }
          // if (check_indirect_pattern(it_meta, addr)) {
          //   it_meta->second.pattern = PATTERN::INDIRECT;
          //   break;
          // }
          if (check_chain_pattern(it_meta, addr)) {
            it_meta->second.pattern = PATTERN::CHAIN;
            break;
          }
        }
      }
      if (it_meta->second.pattern == PATTERN::POINTER_A) {
        if (check_pointerB_pattern(it_meta, addr)) {
          it_meta->second.pattern = PATTERN::POINTER_B;
          it_meta->second.confirm = true;
        }
      }
#ifdef ENABLE_TIMER
      auto t2 = std::chrono::high_resolution_clock::now();
      total_time +=
          std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
#endif
    }
    {  // Add Next
      it_val = value2trace.find(value);
      {  // value2trace
        if (it_val == value2trace.end()) {
          value2trace[value] = std::deque<TraceNode>();
          it_val = value2trace.find(value);
        }
      }
      add_next(it_val->second, tn);
    }
    if (tn.value > 0 && tn.value < 2147483647) {
      add_next(traceHistory, tn);
    }
  }
  void printStats(int totalCnt) {
    std::vector<uint64_t> accessCount(PATTERN_NUM, 0), pcCount(PATTERN_NUM, 0);
    for (auto meta : pc2meta) {
      if (meta.second.count == 1) meta.second.pattern = PATTERN::FRESH;
      accessCount[to_underlying(meta.second.pattern)] += meta.second.count;
      pcCount[to_underlying(meta.second.pattern)]++;
    }
    std::cout << "==================================" << std::endl;
    std::cout << "Total Access\t" << totalCnt << std::endl;
    for (int i = 0; i < PATTERN_NUM; i++) {
      std::cout << PATTERN_NAME[i] << "\t" << accessCount[i] << std::endl;
    }
    std::cout << "==================================" << std::endl;
    std::cout << "Total PC\t" << pc2meta.size() << std::endl;
    for (int i = 0; i < PATTERN_NUM; i++) {
      std::cout << PATTERN_NAME[i] << "\t" << pcCount[i] << std::endl;
    }
    std::cout << "==================================" << std::endl;

#ifdef ENABLE_TIMER
    std::cout << "Total time: " << total_time / 1000000000 << "s "
              << total_time % 1000000000 / 1000000 << " ms"
              << total_time % 1000000 / 1000 << " us" << total_time % 1000
              << " ns" << std::endl;
    total_time /= totalCnt;
    std::cout << "Per access time: " << total_time / 1000000000 << "s "
              << total_time % 1000000000 / 1000000 << " ms"
              << total_time % 1000000 / 1000 << " us" << total_time % 1000
              << " ns" << std::endl;
#endif
  }
};

int main(int argc, char *argv[]) {
  auto traces = get_tracereader(argv[1], 1, 0);
  auto traceList = TraceList();
  int id = 0;
  bool isend = false;
  while (true) {
    auto inst = traces->get(isend);
    if (isend) break;
    if (inst.address != 0)
      traceList.add_trace(inst.ip, inst.address, inst.value, inst.isWrite,
                          ++id);
  }
  traceList.printStats(id);
  return 0;
}