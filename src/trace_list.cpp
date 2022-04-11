#include "trace_list.h"

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
    unsigned long long int &pc, unsigned long long int &addr) {
  if (pc2meta[pc].lastaddr == addr) return true;
  return false;
}

bool TraceList::check_stride_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &pc, unsigned long long int &addr) {
  auto offset = abs_sub(it_meta->second.lastaddr, addr);
  if (it_meta->second.cannot_be_stride) return false;
  if (it_meta->second.offset_stride != 0) {
    if (offset == it_meta->second.offset_stride)
      return true;
    else
      it_meta->second.cannot_be_stride = true;
  } else {
    it_meta->second.offset_stride = offset;
  }
  // std::cerr << offset << std::endl;
  // if (offset == 4 || offset == 8 || offset == 16 || offset == 32) return
  // true;
  return false;
};

bool TraceList::check_pointerA_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    std::unordered_map<unsigned long long int, std::deque<TraceNode>>::iterator
        &it_val,
    unsigned long long int &addr) {
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
        it_meta->second.confirm = true;
        return true;
      }
    }
  }
  return false;
}

bool TraceList::check_pointerB_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  if (pc2meta[it_meta->second.lastpc].is_stride()) return true;
  return false;
}

bool TraceList::check_indirect_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  if (it_meta->second.pc_value_candidate.empty()) {
    for (auto trace : traceHistory) {
      if (pc2meta[trace.pc].is_stride()) {
        it_meta->second.pc_value_candidate[trace.pc] =
            std::make_pair(trace.value, addr);
      }
    }
  } else {
    for (auto trace : traceHistory) {
      if (pc2meta[trace.pc].is_stride()) {
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
  return false;
}

bool TraceList::check_chain_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  if (it_meta->second.offset_candidate.empty()) {
    for (auto trace : traceHistory) {
      if (pc2meta[trace.pc].pattern == PATTERN::POINTER_A) {
        it_meta->second.offset_candidate.insert(abs_sub(trace.value, addr));
      }
    }
  } else {
    for (auto trace : traceHistory) {
      if (pc2meta[trace.pc].pattern == PATTERN::POINTER_A &&
          it_meta->second.offset_candidate.find(abs_sub(trace.value, addr)) !=
              it_meta->second.offset_candidate.end()) {
        it_meta->second.offset_candidate.clear();
        it_meta->second.offset = abs_sub(trace.value, addr);
        it_meta->second.confirm = true;
        return true;
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
    if (it_meta->second.pattern == PATTERN::OTHER ||
        it_meta->second.confirm == false) {
      PATTERN pattern;
      for (bool X = true; X; X = false) {
        if (check_static_pattern(it_meta, pc, addr)) {
          it_meta->second.pattern = PATTERN::STATIC;
          break;
        }
        if (check_stride_pattern(it_meta, pc, addr)) {
          it_meta->second.maybe_stride = true;
        }
        if (check_pointerA_pattern(it_meta, it_val, addr)) {
          it_meta->second.pattern = PATTERN::POINTER_A;
          break;
        }
        if (check_indirect_pattern(it_meta, addr)) {
          it_meta->second.pattern = PATTERN::INDIRECT;
          break;
        }
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
  {
    it_meta->second.count++;
    it_meta->second.lastaddr = addr;
    it_meta->second.inst_id_list.push_back(inst_id);
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

void TraceList::printStats(int totalCnt, char *filename) {
  std::vector<unsigned long long int> accessCount(PATTERN_NUM, 0),
      pcCount(PATTERN_NUM, 0);
  for (auto meta : pc2meta) {
    if (meta.second.is_stride())
      meta.second.pattern = PATTERN::STRIDE;
    else if (meta.second.count < 10 && meta.second.pattern == PATTERN::OTHER)
      meta.second.pattern = PATTERN::FRESH;
    accessCount[to_underlying(meta.second.pattern)] += meta.second.count;
    pcCount[to_underlying(meta.second.pattern)]++;
  }
  if (out) {
    for (auto meta : pc2meta) {
      out << std::hex << meta.first << " "
          << PATTERN_NAME[to_underlying(meta.second.pattern)] << " " << std::dec
          << meta.second.count << std::endl;
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
            << total_time % 1000000 / 1000 << " us" << total_time % 1000
            << " ns" << std::endl;
  total_time /= totalCnt;
  fout << "Per access time: " << total_time / 1000000000 << "s "
            << total_time % 1000000000 / 1000000 << " ms"
            << total_time % 1000000 / 1000 << " us" << total_time % 1000
            << " ns" << std::endl;
#endif
}