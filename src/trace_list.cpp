// Author: Lixiang

#include "trace_list.h"

inline long long int abs_sub(unsigned long long int a,
                             unsigned long long int b) {
  return (long long)a - (long long)b;
}

void TraceList::erase_before(std::deque<TraceNode> &L,
                             const unsigned long long int &id) {
#ifndef DEBUG_ALL
  while (!L.empty() && L.front().id < id - INTERVAL) L.pop_front();
#endif
}
void TraceList::add_next(std::deque<TraceNode> &L, TraceNode tn) {
  L.push_back(tn);
}

bool TraceList::check_static_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  if (it_meta->second.lastaddr == addr || it_meta->second.lastaddr_2 == addr)
    return true;
  return false;
}

bool TraceList::check_stride_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  auto offset = abs_sub(it_meta->second.lastaddr, addr);
  if (it_meta->second.offset_stride != 0) {
    if (offset == it_meta->second.offset_stride) {
      it_meta->second.stride_confidence++;
    } else {
      it_meta->second.stride_confidence--;
      if (it_meta->second.stride_confidence == 0)
        it_meta->second.offset_stride = offset;
    }
    if (it_meta->second.is_stride()) return true;
  } else {
    it_meta->second.offset_stride = offset;
  }
  return false;
}

bool TraceList::check_pointer_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    std::unordered_map<unsigned long long int, std::deque<TraceNode>>::iterator
        &it_val) {
  if (it_meta->second.lastpc != 0) return true;
  if (it_meta->second.lastpc_candidate.empty()) {
    for (auto it = it_val->second.rbegin(); it != it_val->second.rend(); it++) {
      it_meta->second.lastpc_candidate.insert(it->pc);
    }
  } else {
    for (auto it = it_val->second.rbegin(); it != it_val->second.rend(); it++) {
      if (it_meta->second.lastpc_candidate.find(it->pc) !=
          it_meta->second.lastpc_candidate.end()) {
        it_meta->second.lastpc_candidate.clear();
        it_meta->second.lastpc = it->pc;
        return true;
      }
    }
  }
  return false;
}

bool TraceList::check_pointerA_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  long long offset_now = (long long)addr - (long long)it_meta->second.lastvalue;
  if (offset_now > 65536) return false;
  if (addr == it_meta->second.lastaddr || addr == it_meta->second.lastaddr_2)
    return false;
  if (it_meta->second.pointerA_offset_candidate == -1) {
    it_meta->second.pointerA_offset_candidate = offset_now;
    it_meta->second.pointerA_confidence = 0;
  } else {
    if (it_meta->second.pointerA_offset_candidate == offset_now) {
      it_meta->second.pointerA_confidence++;
      if (it_meta->second.pointerA_confidence >= POINTER_A_THERSHOLD) {
        return true;
      }
    } else {
      it_meta->second.pointerA_confidence--;
      if (it_meta->second.pointerA_confidence < 0) {
        it_meta->second.pointerA_offset_candidate = offset_now;
        // fprintf(stderr, "%llx\n", it_meta->second.pointerA_offset_candidate);
        it_meta->second.pointerA_confidence = 0;
      }
    }
  }
  return false;
}

bool TraceList::check_pointerB_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta) {
  if (pc2meta[it_meta->second.lastpc].is_stride()) return true;
  return false;
}

bool TraceList::check_indirect_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  if (addr == it_meta->second.lastaddr) return false;
  if (it_meta->second.is_stride()) return false;
  for (auto it_trace = traceHistory.rbegin(); it_trace != traceHistory.rend();
       it_trace++) {
    auto &trace = *it_trace;
    if (trace.pc == it_meta->first) break;  // find loop
    if (pc2meta[trace.pc].is_stride()) {
      auto it = it_meta->second.pc_value_candidate.find(trace.pc);
      long long offset_now;
      if (it != it_meta->second.pc_value_candidate.end()) {
        if (addr != it->second.addr && trace.value != it->second.value &&
            abs_sub(addr, it->second.addr) %
                    abs_sub(trace.value, it->second.value) ==
                0) {
          offset_now = abs_sub(addr, it->second.addr) /
                       abs_sub(trace.value, it->second.value);
          if (offset_now != 4 && offset_now != 8 && offset_now != 16) continue;
          // if (offset_now % 4 != 0) continue;
          if (it->second.offset == 0) {
            it->second.offset = offset_now;
            it->second.confidence = 0;
          } else if (it->second.offset == offset_now) {
            it->second.confidence++;
          } else {
            it->second.confidence--;
            if (it->second.confidence < 0) {
              it->second.offset = offset_now;
              it->second.confidence = 0;
            }
          }
        }
        if (it->second.confidence >= INDIRECT_THERSHOLD) {
          it_meta->second.pc_value =
              PCmeta::pc_value_meta(trace.pc, addr - trace.value * offset_now);
          it_meta->second.pc_value.offset = offset_now;
          it_meta->second.pc_value_candidate.clear();
          return true;
        }
      } else {
        it_meta->second.pc_value_candidate[trace.pc] =
            PCmeta::pc_value_meta(trace.value, addr);
      }
    }
  }
  return false;
}

bool TraceList::check_chain_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  for (auto it_trace = traceHistory.rbegin(); it_trace != traceHistory.rend();
       it_trace++) {
    auto &trace = *it_trace;
    if (trace.pc == it_meta->first) break;  // find loop
    if (pc2meta[trace.pc].is_pointerA() && abs_sub(trace.value, addr) >= 2 &&
        abs_sub(trace.value, addr) <= 65536) {
      auto it = it_meta->second.chain_candidate.find(trace.pc);
      if (it == it_meta->second.chain_candidate.end()) {
        it_meta->second.chain_candidate[trace.pc] =
            std::make_pair(abs_sub(trace.value, addr), 0);
      } else {
        if (it->second.first == abs_sub(trace.value, addr)) {
          it->second.second++;
          if (it->second.second >= CHAIN_THERSHOLD) {
            it_meta->second.chain_candidate.clear();
            return true;
          }
        } else {
          it->second.second--;
          if (it->second.second < 0) {
            it->second = std::make_pair(abs_sub(trace.value, addr), 0);
          }
        }
      }
    }
  }
  return false;
}

void TraceList::add_trace(unsigned long long int pc,
                          unsigned long long int addr,
                          unsigned long long int value, bool isWrite,
                          unsigned long long int id, const int inst_id) {
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
#ifdef ENABLE_TIMER
    auto t1 = std::chrono::high_resolution_clock::now();
#endif
    if (!it_meta->second.confirm) {
      for (bool X = true; X; X = false) {
        if (check_static_pattern(it_meta, addr)) {
          it_meta->second.pattern_confidence[to_underlying(PATTERN::STATIC)]++;
          if (it_meta->second
                  .pattern_confidence[to_underlying(PATTERN::STATIC)] >=
              STATIC_THERSHOLD) {
            it_meta->second.pattern = PATTERN::STATIC;
            it_meta->second.confirm = true;
            break;
          }
        }
        if (check_stride_pattern(it_meta, addr)) {
          it_meta->second.pattern_confidence[to_underlying(PATTERN::STRIDE)]++;
        }
        if (check_pointer_pattern(it_meta, it_val)) {
          it_meta->second.pattern_confidence[to_underlying(PATTERN::pointer)]++;
          if (check_pointerB_pattern(it_meta)) {
            it_meta->second.pattern = PATTERN::POINTER_B;
            it_meta->second.confirm = true;
            break;
          }
        }
        if (check_pointerA_pattern(it_meta, addr)) {
          it_meta->second.pattern = PATTERN::POINTER_A;
          it_meta->second.confirm = true;
          break;
        }
        // if (check_chain_pattern(it_meta, addr)) {
        //   it_meta->second.pattern = PATTERN::CHAIN;
        //   it_meta->second.confirm = true;
        //   break;
        // }
        if (check_indirect_pattern(it_meta, addr)) {
          it_meta->second.pattern = PATTERN::INDIRECT;
          it_meta->second.confirm = true;
          break;
        }
      }
    }
#ifdef ENABLE_TIMER
    auto t2 = std::chrono::high_resolution_clock::now();
    total_time +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
#endif
  }
  {
    it_meta->second.count++;
    it_meta->second.lastaddr_2 = it_meta->second.lastaddr;
    it_meta->second.lastaddr = addr;
    // it_meta->second.inst_id_list.push_back(inst_id);
    it_meta->second.lastvalue = value;
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
  if (tn.value < 2147483647) {
    add_next(traceHistory, tn);
  }
}

void TraceList::printStats(unsigned long long totalCnt, const char filename[]) {
  std::vector<unsigned long long int> accessCount(PATTERN_NUM, 0),
      pcCount(PATTERN_NUM, 0);
  for (auto &[pc, meta] : pc2meta) {
    if (meta.pattern == PATTERN::OTHER) {
      int champ_confidence = 0;
      for (int i = 0; i < PATTERN_NUM - 1; i++) {
        if (static_cast<PATTERN>(i) == PATTERN::FRESH ||
            static_cast<PATTERN>(i) == PATTERN::CHAIN ||
            static_cast<PATTERN>(i) == PATTERN::INDIRECT ||
            static_cast<PATTERN>(i) == PATTERN::POINTER_A ||
            static_cast<PATTERN>(i) == PATTERN::POINTER_B)
          continue;
        if (meta.pattern_confidence[i] > champ_confidence) {
          meta.pattern = static_cast<PATTERN>(i);
          champ_confidence = meta.pattern_confidence[i];
        }
      }
      if (meta.pattern == PATTERN::OTHER) {
        if (meta.count < PATTERN_THERSHOLD) meta.pattern = PATTERN::FRESH;
      }
    }
    accessCount[to_underlying(meta.pattern)] += meta.count;
    pcCount[to_underlying(meta.pattern)]++;
  }
  if (out) {
    for (auto &[pc, meta] : pc2meta) {
      out << std::hex << pc << " ";
      meta.output(out);
    }
    out.close();
  }
  std::ofstream fout(filename);
  fout << "==================================" << std::endl;
  fout << "Total Access\t" << totalCnt << std::endl;
  for (int i = 0; i < PATTERN_NUM; i++) {
    fout << PATTERN_NAME[i] << "\t" << accessCount[i] << std::endl;
  }
  fout << "==================================" << std::endl;
  fout << "Total PC\t" << pc2meta.size() << std::endl;
  for (int i = 0; i < PATTERN_NUM; i++) {
    fout << PATTERN_NAME[i] << "\t" << pcCount[i] << std::endl;
  }
  fout << "==================================" << std::endl;

#ifdef ENABLE_TIMER
  fout << "Total time: " << total_time / 1000000000 << "s "
       << total_time % 1000000000 / 1000000 << " ms"
       << total_time % 1000000 / 1000 << " us" << total_time % 1000 << " ns"
       << std::endl;
  total_time /= totalCnt;
  fout << "Per access time: " << total_time / 1000000000 << "s "
       << total_time % 1000000000 / 1000000 << " ms"
       << total_time % 1000000 / 1000 << " us" << total_time % 1000 << " ns"
       << std::endl;
#endif
}