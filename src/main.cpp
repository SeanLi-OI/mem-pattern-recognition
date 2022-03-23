#include <algorithm>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>

#include "tracereader.h"

#define DEBUG_PC
#define DEBUG_ADDR
// #define DEBUG_HISTORY
#define ENABLE_TIMER

#ifdef ENABLE_TIMER
#include <chrono>
#endif

const int PATTERN_NUM = 7;
const std::string PATTERN_NAME[] = {"fresh\t",   "static\t", "stride\t",
                                    "pointer\t", "indirect", "chain\t",
                                    "other\t"};
// const int pattern_mapper[] = {0, 3, 4, 5, 1, 2, 6};
const int pattern_mapper[] = {0, 1, 2, 3, 4, 5, 6};

const uint32_t INTERVAL = 512;

class TraceList {
  struct TraceNode {
    uint64_t id;
    uint64_t pc;
    uint64_t addr;
    uint64_t value;
    bool isRead;
    uint16_t pattern;
    uint64_t last_addr;
    uint64_t base_addr;
    TraceNode() {}
    TraceNode(uint64_t _p, uint64_t _a, uint64_t _v, bool _i, uint64_t _id)
        : pc(_p), addr(_a), value(_v), isRead(_i), id(_id) {
      pattern = PATTERN_NUM - 1;
      last_addr = 0;
      base_addr = addr;
    }
  };

  std::unordered_map<uint64_t, std::deque<TraceNode>> pc2trace;
  std::unordered_map<uint64_t, std::deque<TraceNode>> value2trace;
  std::deque<TraceNode> traceHistory;

  uint64_t pattern_count[PATTERN_NUM];
  std::unordered_map<uint64_t, uint16_t> pc2pattern;
#ifdef ENABLE_TIMER
  uint64_t total_time;
#endif

  void erase_before(std::deque<TraceNode> &L, const uint64_t &id) {
#ifndef DEBUG_ALL
    while (!L.empty() && L.front().id < id - INTERVAL) L.pop_front();
#endif
  }
  void add_next(std::deque<TraceNode> &L, TraceNode tn) { L.push_back(tn); }

 public:
  TraceList() {
    for (int i = 0; i < PATTERN_NUM; i++) pattern_count[i] = 0;
#ifdef ENABLE_TIMER
    total_time = 0;
#endif
  }
  void add_trace(uint64_t pc, uint64_t addr, uint64_t value, bool isRead,
                 uint64_t id) {
    auto it_pc = pc2trace.find(pc);
    {  // pc2trace
      if (it_pc != pc2trace.end()) {
#ifndef DEBUG_PC
        erase_before(it_pc->second, id);
#endif
      } else {
        pc2trace[pc] = std::deque<TraceNode>();
        it_pc = pc2trace.find(pc);
      }
    }
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

    std::vector<std::function<bool()>> check_pattern;
    TraceNode tn(pc, addr, value, isRead, id);
    {  // check patterns
      std::function<bool()> check_fresh_access = [&]() {
        return it_pc->second.empty();
        // return false;
      };
      std::function<bool()> check_static_pattern = [&]() {
        auto it = it_pc->second.rbegin();
        // for (auto it = it_pc->second.rbegin(); it != it_pc->second.rend();
        //      it++) {
        if (it->addr == addr) return true;
        // }
        return false;
      };
      std::function<bool()> check_stride_pattern = [&]() {
        auto it = it_pc->second.rbegin();
        if (abs(it->addr - addr) == 8) {
          tn.base_addr = it->base_addr;
          return true;
        }
        return false;
      };
      std::function<bool()> check_pointer_pattern = [&]() {
        if (it_val->second.size() == 0) return false;
        // return true;
        auto it = it_val->second.rbegin();
        // for (auto it = it_val->second.rbegin(); it != it_val->second.rend();
        //      it++) {
        if (it != it_val->second.rend() && it->pattern >= 2) return true;
        // }
        return false;
      };

      std::function<bool()> check_indirect_pattern = [&]() {
        auto it2 = it_pc->second.rbegin();
        auto offset = abs(addr - it2->base_addr);
        // auto it = traceHistory.rbegin();
        for (auto it = traceHistory.rbegin(); it != traceHistory.rend(); it++) {
          if (it->value != 0 && offset % it->value == 0 &&
              // (offset / it->value == 8 || offset / it->value == 4) &&
              it->pattern >= 2) {
            // std::cerr << offset << " " << it->value << std::endl;
            tn.base_addr = addr - offset;
            return true;
          }
        }
        return false;
      };
      std::function<bool()> check_chain_pattern = [&]() { return false; };
      check_pattern.push_back(check_fresh_access);
      check_pattern.push_back(check_static_pattern);
      check_pattern.push_back(check_stride_pattern);
      check_pattern.push_back(check_pointer_pattern);
      check_pattern.push_back(check_indirect_pattern);
      check_pattern.push_back(check_chain_pattern);
    }
#ifdef ENABLE_TIMER
    auto t1 = std::chrono::high_resolution_clock::now();
#endif
    for (uint16_t i = 0; i < check_pattern.size(); i++) {
      int j = pattern_mapper[i];
      if (check_pattern[j]()) {
        pattern_count[j]++;
        tn.pattern = j;
        break;
      }
    }
#ifdef ENABLE_TIMER
    auto t2 = std::chrono::high_resolution_clock::now();
    total_time +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
#endif
    if (tn.pattern == PATTERN_NUM - 1) pattern_count[PATTERN_NUM - 1]++;
    if (pc2pattern.find(pc) == pc2pattern.end())
      pc2pattern[pc] = tn.pattern;
    else
      pc2pattern[pc] = std::max(tn.pattern, pc2pattern[pc]);
    add_next(it_pc->second, tn);
    {
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
    std::cout << "Total Access: " << totalCnt << std::endl;
    for (int i = 0; i < PATTERN_NUM; i++) {
      std::cout << PATTERN_NAME[i] << "\t" << pattern_count[i] << std::endl;
    }
    std::vector<int> tmp_pattern_count(PATTERN_NUM, 0);
    for (auto kv : pc2pattern) {
      tmp_pattern_count[kv.second]++;
    }
    std::cout << "Total Unique PC: " << pc2pattern.size() << std::endl;
    for (int i = 0; i < PATTERN_NUM; i++) {
      std::cout << PATTERN_NAME[i] << "\t" << tmp_pattern_count[i] << std::endl;
    }
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
    for (u_int16_t i = 0; i < NUM_INSTR_SOURCES; i++) {
      if (inst.source_memory[i] != 0) {
        // printf("%lld %lld %lld READ\n", inst.ip, inst.source_memory[i],
        //        inst.source_memory_value[i]);
        traceList.add_trace(inst.ip, inst.source_memory[i],
                            inst.source_memory_value[i], 1, ++id);
      }
    }
    for (u_int16_t i = 0; i < NUM_INSTR_DESTINATIONS; i++) {
      if (inst.destination_memory[i] != 0) {
        // printf("%lld %lld %lld WRITE\n", inst.ip, inst.destination_memory[i],
        //        inst.destination_memory_value[i]);
        traceList.add_trace(inst.ip, inst.destination_memory[i],
                            inst.destination_memory_value[i], 0, ++id);
      }
    }
  }
  traceList.printStats(id);
  return 0;
}