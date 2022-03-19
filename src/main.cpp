#include <functional>
#include <queue>
#include <string>
#include <unordered_map>

#include "tracereader.h"

#define DEBUG
#define ENABLE_TIMER

#ifdef ENABLE_TIMER
#include <time.h>
timespec diff(timespec start, timespec end) {
  timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp;
}
timespec sum(timespec a, timespec b) {
  timespec temp;
  if (a.tv_nsec + b.tv_nsec >= 1000000000) {
    temp.tv_sec = a.tv_sec + b.tv_sec + 1;
    temp.tv_nsec = a.tv_nsec + b.tv_nsec - 1000000000;
  } else {
    temp.tv_sec = a.tv_sec + b.tv_sec;
    temp.tv_nsec = a.tv_nsec + b.tv_nsec;
  }
  return temp;
}
timespec div(timespec t, int n) {
  uint64_t nsec = (t.tv_sec * 1000000000 + t.tv_nsec) / n;
  timespec temp;
  temp.tv_sec = nsec / 1000000000;
  temp.tv_nsec = nsec % 1000000000;
  return temp;
}
#endif

const int PATTERN_NUM = 7;
const std::string PATTERN_NAME[] = {"fresh\t",   "static\t", "stride\t",
                                    "pointer\t", "indirect", "chain\t",
                                    "other\t"};

const uint32_t INTERVAL = 16;

class TraceList {
  struct TraceNode {
    uint64_t id;
    uint64_t pc;
    uint64_t addr;
    uint64_t value;
    bool isRead;
    uint16_t pattern;
    uint64_t last_addr;
    TraceNode() {}
    TraceNode(uint64_t _p, uint64_t _a, uint64_t _v, bool _i, uint64_t _id)
        : pc(_p), addr(_a), value(_v), isRead(_i), id(_id) {
      pattern = PATTERN_NUM - 1;
      last_addr = 0;
    }
  };

  std::unordered_map<uint64_t, std::deque<TraceNode>> pc2trace;
  std::unordered_map<uint64_t, std::deque<TraceNode>> value2trace;
  std::deque<TraceNode> traceHistory;
  uint64_t pattern_count[PATTERN_NUM];
  timespec total_time;
  void erase_before(std::deque<TraceNode> &L, const uint64_t &id) {
    while (!L.empty() && L.front().id < id - INTERVAL) L.pop_front();
  }
  void add_next(std::deque<TraceNode> &L, TraceNode tn) { L.push_back(tn); }

 public:
  TraceList() {
    for (int i = 0; i < PATTERN_NUM; i++) pattern_count[i] = 0;
    total_time.tv_nsec = total_time.tv_sec = 0;
  }
  void add_trace(uint64_t pc, uint64_t addr, uint64_t value, bool isRead,
                 uint64_t id) {
    auto it_pc = pc2trace.find(pc);
    {  // pc2trace
      if (it_pc != pc2trace.end())
        erase_before(it_pc->second, id);
      else {
        pc2trace[pc] = std::deque<TraceNode>();
        it_pc = pc2trace.find(pc);
      }
    }
    auto it_val = value2trace.find(addr);
    {  // value2trace
      if (it_val != pc2trace.end())
        erase_before(it_val->second, id);
      else {
        value2trace[addr] = std::deque<TraceNode>();
        it_val = value2trace.find(addr);
      }
    }
    erase_before(traceHistory, id);

    std::vector<std::function<bool()>> check_pattern;
    TraceNode tn(pc, addr, value, isRead, id);
    {  // check patterns
      std::function<bool()> check_fresh_access = [&]() {
        return it_pc->second.empty();
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
        for (auto it = it_pc->second.rbegin(); it != it_pc->second.rend();
             it++) {
          if (abs(it->addr - addr) == 8) return true;
        }
        return false;
      };
      std::function<bool()> check_pointer_pattern = [&]() {
        for (auto it = it_val->second.rbegin(); it != it_val->second.rend();
             it++) {
          if (it->pattern == 2) return true;
        }
        return false;
      };
      std::function<bool()> check_indirect_pattern = [&]() {
        for (auto it = traceHistory.rbegin(); it != traceHistory.rend(); it++) {
        }
        return false;
      };
      std::function<bool()> check_chain_pattern = [&]() { return false; };
      check_pattern.push_back(check_fresh_access);
      check_pattern.push_back(check_static_pattern);
      check_pattern.push_back(check_stride_pattern);
      check_pattern.push_back(check_pointer_pattern);
      // check_pattern.push_back(check_indirect_pattern);
      // check_pattern.push_back(check_chain_pattern);
    }
#ifdef ENABLE_TIMER
    timespec time1, time2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
#endif
    for (uint16_t i = 0; i < check_pattern.size(); i++) {
      if (check_pattern[i]()) {
        pattern_count[i]++;
        tn.pattern = i;
        break;
      }
    }
#ifdef ENABLE_TIMER
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    total_time = sum(total_time, diff(time1, time2));
#endif
    pattern_count[PATTERN_NUM]++;
    add_next(it_pc->second, tn);
    {
      it_val = value2trace.find(value);
      {  // value2trace
        if (it_val == pc2trace.end()) {
          value2trace[value] = std::deque<TraceNode>();
          it_val = value2trace.find(value);
        }
      }
      add_next(it_val->second, tn);
    }
    add_next(traceHistory, tn);
    // {
    //   auto it = pc2trace.find(pc);
    //   if (it != pc2trace.end()) {
    //     bool is1 = 0, is2 = 0, is3 = 0;
    //     bool isloop1 = 0, isloop2 = 0, isloop3 = 0;
    //     for (auto trace = it->second.rbegin(); trace != it->second.rend();
    //          trace++) {
    //       if (id - trace->id > 32) break;
    //       if (abs(trace->addr - addr) == 8) {
    //         auto it2 = isLoop1.find(trace->addr);
    //         if (it2 == isLoop1.end()) {
    //           isloop1 = 1;
    //           isLoop1[trace->addr] = 1;
    //           isLoop1[addr] = 1;
    //         }
    //         is1 = 1;
    //       } else {
    //         if (abs(trace->addr - addr) % 8 == 0) {
    //           auto it2 = isLoop3.find(trace->addr);
    //           if (it2 == isLoop3.end()) {
    //             isloop3 = 1;
    //             isLoop3[trace->addr] = 1;
    //             isLoop3[addr] = 1;
    //           }
    //         }
    //         is3 = 1;
    //       }
    //       if (trace->value == addr) {
    //         auto it2 = isLoop2.find(addr);
    //         if (it2 == isLoop2.end()) {
    //           isloop2 = 1;
    //           isLoop2[addr] = 1;
    //           isLoop2[value] = 1;
    //         }
    //         is2 = 1;
    //       }
    //     }
    //     cnt1 += is1;
    //     cnt2 += is2;
    //     cnt3 += is3;
    //     cntLoop1 += isloop1;
    //     cntLoop2 += isloop2;
    //     cntLoop3 += isloop3;
    //   } else {
    //     pc2trace[pc] = std::vector<TraceNode>();
    //   }
    //   pc2trace[pc].emplace_back(TraceNode(pc, addr, value, isRead));
    // }
  }
  void printStats(int totalCnt) {
    for (int i = 0; i < PATTERN_NUM; i++) {
      std::cout << PATTERN_NAME[i] << "\t" << pattern_count[i] << std::endl;
    }
#ifdef ENABLE_TIMER
    std::cout << div(total_time, totalCnt).tv_sec << "s "
              << div(total_time, totalCnt).tv_nsec << "ns" << std::endl;
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