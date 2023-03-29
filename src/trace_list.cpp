// Author: Lixiang

#include "trace_list.h"

#include <glog/logging.h>

#include <filesystem>

#include "utils/macro.h"

#define INTERVAL 64

inline long long int abs_sub(unsigned long long int a,
                             unsigned long long int b) {
  return (long long)a - (long long)b;
}

void add_trace(TraceList &traceList, unsigned long long &id,
               unsigned long long int ip, MemRecord &r, bool isWrite,
               const int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  if (trace_filter(ip, isWrite, tmp)) return;
  traceList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
}

void TraceList::erase_before(std::deque<TraceNode> &L,
                             const unsigned long long int &id) {
#ifndef DEBUG_ALL
  while (!L.empty() && id - L.front().id > INTERVAL) {
    L.pop_front();
  }
#endif
}
void TraceList::add_next(std::deque<TraceNode> &L, TraceNode tn) {
  L.push_back(tn);
}

bool TraceList::check_static_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr, unsigned long long int &value) {
  // if (it_meta->first == 0x4019d6)
  //   std::cerr << std::hex << addr << " " << std::hex <<
  //   it_meta->second.lastaddr
  //             << " " << std::dec << it_meta->second.static_tmp << " "
  //             <<
  //             it_meta->second.is_not_pattern[to_underlying(PATTERN::STATIC)]
  //             << std::endl;
  if (it_meta->second.lastaddr ==
      addr) {  // || it_meta->second.lastvalue == value) {
    if (it_meta->second.static_tmp.Positive()) {
      it_meta->second.maybe_pattern[to_underlying(PATTERN::STATIC)] = true;
    }
    return true;
  } else {
    if (it_meta->second.static_tmp.test())
      it_meta->second.static_tmp.Negative();
    else if (it_meta->second.maybe_pattern[to_underlying(PATTERN::STATIC)] ==
             true)
      it_meta->second.is_not_pattern[to_underlying(PATTERN::STATIC)] = true;
    return false;
  }
}

bool TraceList::check_stride_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  if (it_meta->second.lastaddr_2 == 0) return false;
  long long int offset = abs_sub(it_meta->second.lastaddr_2, addr);
  if (it_meta->second.offset_stride != 0) {
    if (offset == it_meta->second.offset_stride) {
      if (it_meta->second.stride_tmp.Positive())
        it_meta->second.maybe_pattern[to_underlying(PATTERN::STRIDE)] = true;
      it_meta->second.stride_flag = false;
      return true;
    } else {
      if (it_meta->second.maybe_pattern[to_underlying(PATTERN::STRIDE)] ==
          true) {
        if (it_meta->second.stride_tmp.test()) {
          if (it_meta->second.stride_flag) {
            it_meta->second.stride_tmp.Negative();
          } else
            it_meta->second.stride_flag = true;
        } else
          it_meta->second.is_not_pattern[to_underlying(PATTERN::STRIDE)] = true;
      }
    }
  } else {
    it_meta->second.offset_stride = offset;
  }
  return false;
}

bool TraceList::check_pointer_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    std::unordered_map<unsigned long long int, std::deque<TraceNode>>::iterator
        &it_val) {
  // if (it_meta->second.lastpc != 0) return true;
  if (it_meta->second.is_not_pattern[PATTERN::pointer]) return false;
  if (it_meta->second.lastpc_candidate.empty()) {
    for (auto it = it_val->second.rbegin(); it != it_val->second.rend(); it++) {
      it_meta->second.lastpc_candidate.insert(it->pc);
    }
    it_meta->second.maybe_pattern[PATTERN::pointer] = true;
  } else {
    std::set<unsigned long long int> tmp;
    for (auto it = it_val->second.rbegin(); it != it_val->second.rend(); it++) {
      if (it_meta->second.lastpc_candidate.find(it->pc) !=
          it_meta->second.lastpc_candidate.end()) {
        // it_meta->second.lastpc_candidate.clear();
        tmp.insert(it->pc);
        // it_meta->second.lastpc = it->pc;
        // return true;
      }
    }
    it_meta->second.lastpc_candidate = tmp;
    if (tmp.empty()) {
      it_meta->second.is_not_pattern[PATTERN::pointer] = true;
      // it_meta->second.is_not_pattern[PATTERN::POINTER_B] = true;
    } else {
      it_meta->second.lastpc = *tmp.begin();
      return true;
    }
  }
  return false;
}

bool TraceList::check_pointerA_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr, unsigned long long int &value,
    bool &isWrite) {
  long long offset_now = (long long)addr - (long long)value;
  if (!isWrite) {
    if (it_meta->second.lastvalue == 0) return false;
    offset_now = (long long)addr - (long long)it_meta->second.lastvalue;
  } else {
    if (value == 0) return false;
  }
  if (addr == it_meta->second.lastaddr) return false;
  // if (it_meta->first == 0x40484f) {
  // std::cerr << offset_now << " " << it_meta->second.pointerA_offset_candidate
  //           << " "
  //           <<
  //           it_meta->second.maybe_pattern[to_underlying(PATTERN::POINTER_A)]
  //           << " " << it_meta->second.pointerA_tmp << " "
  //           <<
  //           it_meta->second.is_not_pattern[to_underlying(PATTERN::POINTER_A)]
  //           << std::endl;
  // }
  if (it_meta->second.pointerA_offset_candidate == -1) {
    it_meta->second.pointerA_offset_candidate = offset_now;
  } else {
    // std::cerr << it_meta->second.pointerA_tmp << std::endl;
    if (it_meta->second.pointerA_offset_candidate == offset_now) {
      if (it_meta->second.pointerA_tmp.Positive())
        it_meta->second.maybe_pattern[to_underlying(PATTERN::POINTER_A)] = true;
    } else {
      if (it_meta->second.pointerA_tmp.test())
        it_meta->second.pointerA_tmp.Negative();
      else if (it_meta->second
                   .maybe_pattern[to_underlying(PATTERN::POINTER_A)] == true)
        it_meta->second.is_not_pattern[to_underlying(PATTERN::POINTER_A)] =
            true;
    }
  }
  return false;
}

bool TraceList::check_pointerB_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta) {
  if (pc2meta[it_meta->second.lastpc].is_pattern(PATTERN::STRIDE)) return true;
  return false;
}

bool TraceList::check_indirect_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  if (addr == it_meta->second.lastaddr) return false;
  if (it_meta->second.is_pattern(PATTERN::STRIDE)) return false;
  if (it_meta->second.is_not_pattern[to_underlying(PATTERN::INDIRECT)])
    return false;
  std::unordered_map<unsigned long long int, PCmeta::pc_value_meta> tmp;
  for (auto it_trace = traceHistory.rbegin(); it_trace != traceHistory.rend();
       it_trace++) {
    auto &trace = *it_trace;
    if (trace.pc == it_meta->first) break;  // find loop
    if (pc2meta[trace.pc].is_pattern(PATTERN::STRIDE)) {
      auto it = it_meta->second.pc_value_candidate.find(trace.pc);
      long long offset_now;

      if (it_meta->second.meeted_pc.find(trace.pc) !=
          it_meta->second.meeted_pc.end()) {
        if (it != it_meta->second.pc_value_candidate.end()) {
          if (addr != it->second.addr && trace.value != it->second.value &&
              abs_sub(addr, it->second.addr) %
                      abs_sub(trace.value, it->second.value) ==
                  0) {
            offset_now = abs_sub(addr, it->second.addr) /
                         abs_sub(trace.value, it->second.value);
            if (offset_now != 2 && offset_now != 4 && offset_now != 8 &&
                offset_now != 16) {
              if (!it->second.confidence.Negative()) {
                tmp[trace.pc] = it->second;
              }
              continue;
            }
            if (it->second.offset == 0) {
              it->second.offset = offset_now;
              it->second.confidence.reset();
              tmp[trace.pc] = it->second;
            } else if (it->second.offset == offset_now) {
              it->second.confidence.Positive();
              tmp[trace.pc] = it->second;
            } else {
              if (it->second.confidence.Negative()) {
                it->second.offset = offset_now;
                it->second.confidence.reset();
              }
              tmp[trace.pc] = it->second;
            }
          } else {
            if (addr == it->second.addr && trace.value == it->second.value) {
              offset_now = it->second.offset;
              it->second.confidence.Positive();
              tmp[trace.pc] = it->second;
            }
          }
          if (it->second.confidence.test()) {
            it_meta->second.pc_value = PCmeta::pc_value_meta(
                trace.pc, addr - trace.value * offset_now);
            it_meta->second.pc_value.offset = offset_now;
            // it_meta->second.pc_value_candidate.clear();
            it_meta->second.maybe_pattern[to_underlying(PATTERN::INDIRECT)] =
                true;
            tmp[trace.pc] = it->second;
            // return true;
          }
        }
      } else {
        tmp[trace.pc] = PCmeta::pc_value_meta(trace.value, addr);
        it_meta->second.meeted_pc.insert(trace.pc);
      }
    }
  }
  it_meta->second.pc_value_candidate = tmp;
  if (tmp.empty()) {
    if (it_meta->second.maybe_pattern[to_underlying(PATTERN::INDIRECT)] ==
        true) {
      it_meta->second.is_not_pattern[to_underlying(PATTERN::INDIRECT)] = true;
    }
  }
  return !tmp.empty();
}

bool TraceList::check_struct_pointer_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  std::unordered_map<unsigned long long int, PCmeta::struct_pointer_meta> tmp(
      it_meta->second.struct_pointer_candidate);
  if (it_meta->second.is_pattern(PATTERN::STRIDE)) return false;
  bool flag = false;
  for (auto it_trace = traceHistory.rbegin(); it_trace != traceHistory.rend();
       it_trace++) {
    auto &trace = *it_trace;
    if (trace.pc == it_meta->first) {
      break;  // find loop
    }
    auto it = it_meta->second.struct_pointer_candidate.find(trace.pc);
    long long offset_now = abs_sub(addr, trace.value);
    if (offset_now <= 0 || offset_now >= 32768) continue;
    if (it_meta->second.meeted_pc_sp.find(trace.pc) !=
        it_meta->second.meeted_pc_sp.end()) {
      if (it != it_meta->second.struct_pointer_candidate.end()) {
        if (!flag) {
          if (it->second.flag) {
            it->second.confidence /= 2;
            if (it->second.confidence <= 2) {
              tmp.erase(tmp.find(trace.pc));
            }
          } else {
            it->second.flag = true;
          }
          flag = true;
        }
        // if (it_meta->first == 0x50a544) {
        //   LOG(INFO) << std::hex << trace.pc << " " << std::dec <<
        //   offset_now
        //             << " " << it->second.offset << std::endl;
        // }
        if (it->second.offset == 0) {
          it->second.offset = offset_now;
          it->second.confidence = 8;
          tmp[trace.pc] = it->second;
        } else {
          if (offset_now == it->second.offset) {
            it->second.confidence++;
            it->second.flag = false;
            tmp[trace.pc] = it->second;
          }
          // if ((it_meta->first >> 1) == 0x12c86 && (trace.pc >> 1) == 0x12c82)
          //   std::cerr << std::dec << offset_now << " " <<
          //   it->second.confidence
          //             << std::endl;
          if (it->second.confidence >= STRUCT_POINTER_THERSHOLD) {
            it_meta->second.last_pc_sp = trace.pc;
            it_meta->second.offset_sp = offset_now;
            it_meta->second
                .maybe_pattern[to_underlying(PATTERN::STRUCT_POINTER)] = true;
          }
        }
      }
    } else {
      tmp[trace.pc] = PCmeta::struct_pointer_meta(trace.pc);
      it_meta->second.meeted_pc_sp.insert(trace.pc);
    }
  }

  if (tmp.empty() &&
      it_meta->second.maybe_pattern[to_underlying(PATTERN::STRUCT_POINTER)] ==
          true)
    it_meta->second.is_not_pattern[to_underlying(PATTERN::STRUCT_POINTER)] =
        true;
  if ((it_meta->first >> 1) == 0x12c86)
    std::cerr
        << it_meta->second.maybe_pattern[to_underlying(PATTERN::STRUCT_POINTER)]
        << it_meta->second
               .is_not_pattern[to_underlying(PATTERN::STRUCT_POINTER)]
        << std::endl;
  // if (it_meta->first == 0x40183e) {
  //   for (auto &t : tmp) {
  //     LOG(INFO) << std::hex << t.first << std::endl;
  //   }
  //   LOG(INFO)
  //       << "Fin "
  //       <<
  //       it_meta->second.maybe_pattern[to_underlying(PATTERN::STRUCT_POINTER)]
  //       << it_meta->second
  //              .is_not_pattern[to_underlying(PATTERN::STRUCT_POINTER)]
  //       << std::endl;
  // }
  it_meta->second.struct_pointer_candidate = tmp;
  return false;
}

bool TraceList::check_locality_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  long long int offset = abs_sub(it_meta->second.lastaddr, addr);
  it_meta->second.monoQueue.push(addr);
  if (!it_meta->second.offset_history.check(offset) &&
      it_meta->second.monoQueue.get() <= LOCALITY_T) {
    it_meta->second.pattern_confidence[to_underlying(PATTERN::LOCALITY)]
        .Positive();
    if (it_meta->second.pattern_confidence[to_underlying(PATTERN::LOCALITY)]
            .get() > LOCALITY_THRESHOLD) {
      it_meta->second.maybe_pattern[to_underlying(PATTERN::LOCALITY)] = true;
    }
    return true;
  } else {
    if (it_meta->second.pattern_confidence[to_underlying(PATTERN::LOCALITY)]
            .get() > 0)
      it_meta->second.pattern_confidence[to_underlying(PATTERN::LOCALITY)]
          .Dec();
    else if (it_meta->second.maybe_pattern[to_underlying(PATTERN::LOCALITY)] ==
             true)
      it_meta->second.is_not_pattern[to_underlying(PATTERN::LOCALITY)] = true;
    return false;
  }
}

bool TraceList::check_random_pattern(
    std::unordered_map<unsigned long long int, PCmeta>::iterator &it_meta,
    unsigned long long int &addr) {
  long long int offset = abs_sub(it_meta->second.lastaddr, addr);
  if (offset >= RANDOM_T && !it_meta->second.offset_history.push(offset)) {
    it_meta->second.pattern_confidence[to_underlying(PATTERN::RANDOM)]
        .Positive();
    if (it_meta->second.pattern_confidence[to_underlying(PATTERN::RANDOM)]
            .get() > RANDOM_THERSHOLD) {
      it_meta->second.maybe_pattern[to_underlying(PATTERN::RANDOM)] = true;
    }
    return true;
  } else {
    if (it_meta->second.pattern_confidence[to_underlying(PATTERN::RANDOM)]
            .get() > 0)
      it_meta->second.pattern_confidence[to_underlying(PATTERN::RANDOM)].Dec();
    else if (it_meta->second.maybe_pattern[to_underlying(PATTERN::RANDOM)] ==
             true)
      it_meta->second.is_not_pattern[to_underlying(PATTERN::RANDOM)] = true;
    return false;
  }
}

#define HOT_REGION_INST_THRESHOLD 32
#define HOT_REGION_LEN_THRESHOLD \
  (hot_region_size / HOT_REGION_INST_THRESHOLD / 4)
void TraceList::check_hot_region(unsigned long long int &region_id,
                                 const unsigned long long &inst_id) {
#ifdef ENABLE_HOTREGION_V1
  auto it = region_cnt.find(region_id);
  if (it == region_cnt.end()) {
    region_cnt[region_id] = 1;
  } else {
    it->second++;
  }
#else
  auto it = region_ht.find(region_id);
  if (it == region_ht.end()) {
    region_ht[region_id] = inst_id;
  } else {
    if (inst_id - it->second < HOT_REGION_INST_THRESHOLD) {
      auto it2 = region_cnt.find(region_id);
      if (it2 == region_cnt.end()) {
        region_cnt[region_id] = 1;
      } else {
        it2->second++;
        if (it2->second >= HOT_REGION_LEN_THRESHOLD)
          hot_region_list.insert(region_id);
      }
    } else {
      auto it2 = region_cnt.find(region_id);
      if (it2 != region_cnt.end()) it2->second = 0;
    }
  }
#endif
}

void TraceList::add_trace(unsigned long long int pc,
                          unsigned long long int addr,
                          unsigned long long int value, bool isWrite,
                          unsigned long long int &id,
                          const unsigned long long inst_id) {
  pc <<= 1;
  if (isWrite) pc |= 1;
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
        if (check_static_pattern(it_meta, addr, value)) {
          // if (!it_meta->second.is_not_static) {
          //   it_meta->second
          //       .pattern_confidence[to_underlying(PATTERN::STATIC)]++;
          //   if (it_meta->second
          //           .pattern_confidence[to_underlying(PATTERN::STATIC)] >=
          //       STATIC_THERSHOLD) {
          //     it_meta->second.pattern = PATTERN::STATIC;
          //     it_meta->second.confirm = true;
          //     break;
          //   }
          // }
        }
        if (check_stride_pattern(it_meta, addr)) {
          // it_meta->second.pattern_confidence[to_underlying(PATTERN::STRIDE)]++;
        }
        if (check_pointer_pattern(it_meta, it_val)) {
          it_meta->second.pattern_confidence[to_underlying(PATTERN::pointer)]
              .Positive();
          // if (it_meta->second
          //         .pattern_confidence[to_underlying(PATTERN::pointer)] >=
          //     POINTER_THERSHOLD) {
          //   if (check_pointerB_pattern(it_meta)) {
          //     it_meta->second.pattern = PATTERN::POINTER_B;
          //     it_meta->second.confirm = true;
          //     break;
          //   }
          // }
        }
        if (check_pointerA_pattern(it_meta, addr, value, isWrite)) {
          // it_meta->second.pattern = PATTERN::POINTER_A;
          // it_meta->second.confirm = true;
          // break;
        }
        // if (check_chain_pattern(it_meta, addr)) {
        //   it_meta->second.pattern = PATTERN::CHAIN;
        //   it_meta->second.confirm = true;
        //   break;
        // }
        if (check_indirect_pattern(it_meta, addr)) {
          // it_meta->second.pattern = PATTERN::INDIRECT;
          // it_meta->second.confirm = true;
          // break;
        }
        if (check_struct_pointer_pattern(it_meta, addr)) {
        }
        if (check_locality_pattern(it_meta, addr)) {
        }
        if (check_random_pattern(it_meta, addr)) {
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
  // if (tn.value < 2147483647) {
  add_next(traceHistory, tn);
  // }
  if (enable_hotregion_) {
    auto region_id = (unsigned long long)addr / hot_region_size;
    check_hot_region(region_id, inst_id);
  }
}

void TraceList::printStats(unsigned long long totalCnt, const char filename[],
                           const char hot_region_file[]) {
  LOG_IF(ERROR, totalCnt == 0) << "Read 0 trace from tracefile" << std::endl;
  std::vector<unsigned long long int> accessCount(PATTERN_NUM, 0),
      pcCount(PATTERN_NUM, 0);
  for (auto &[pc, meta] : pc2meta) {
    if (meta.pattern == PATTERN::OTHER) {
      int champ_confidence = 16;
      for (int i = 0; i < PATTERN_NUM - 1; i++) {
        if (static_cast<PATTERN>(i) == PATTERN::FRESH ||
            static_cast<PATTERN>(i) == PATTERN::POINTER_B ||
            static_cast<PATTERN>(i) == PATTERN::LOCALITY ||
            static_cast<PATTERN>(i) == PATTERN::RANDOM)
          continue;
        if (static_cast<PATTERN>(i) == PATTERN::pointer) {
          if (!meta.lastpc_candidate.empty()) {
            meta.lastpc = *meta.lastpc_candidate.begin();
            if (pc2meta[meta.lastpc]
                    .maybe_pattern[to_underlying(PATTERN::STRIDE)])
              meta.pattern = PATTERN::POINTER_B;
            else
              meta.pattern = PATTERN::pointer;
          }
        } else if (meta.maybe_pattern[i] && !meta.is_not_pattern[i]) {
          meta.pattern = static_cast<PATTERN>(i);
        } else if (meta.pattern_confidence[i].get() > champ_confidence) {
          meta.pattern = static_cast<PATTERN>(i);
          champ_confidence = meta.pattern_confidence[i].get();
        }
      }
      if (meta.pattern == PATTERN::OTHER) {
        for (int i = 0; i < PATTERN_NUM - 1; i++) {
          if (static_cast<PATTERN>(i) == PATTERN::FRESH ||
              static_cast<PATTERN>(i) == PATTERN::INDIRECT ||
              static_cast<PATTERN>(i) == PATTERN::POINTER_B ||
              static_cast<PATTERN>(i) == PATTERN::pointer ||
              static_cast<PATTERN>(i) == PATTERN::POINTER_A ||
              static_cast<PATTERN>(i) == PATTERN::LOCALITY ||
              static_cast<PATTERN>(i) == PATTERN::RANDOM)
            continue;
          if (meta.pattern_confidence[i].get() > champ_confidence) {
            meta.pattern = static_cast<PATTERN>(i);
            champ_confidence = meta.pattern_confidence[i].get();
          }
        }
        if (meta.pattern == PATTERN::OTHER) {
          if (meta.maybe_pattern[to_underlying(PATTERN::LOCALITY)])
            meta.pattern = PATTERN::LOCALITY;
          if (meta.pattern_confidence[PATTERN::RANDOM].get() > RANDOM_THERSHOLD)
            meta.pattern = PATTERN::RANDOM;
          if (meta.count < PATTERN_THERSHOLD) meta.pattern = PATTERN::FRESH;
        }
      }
      if (meta.pattern == PATTERN::OTHER) {
        if (meta.maybe_pattern[to_underlying(PATTERN::LOCALITY)])
          meta.pattern = PATTERN::LOCALITY;
        else if (meta.maybe_pattern[to_underlying(PATTERN::RANDOM)])
          meta.pattern = PATTERN::RANDOM;
      }
    }
  }
  bool flag = true;
  LOG(INFO) << "Start Pointer Chase Correct Cycle" << std::endl;
  while (flag) {
    flag = false;
    for (auto &[pc, meta] : pc2meta) {
      if (meta.maybe_pointer_chase == true) continue;
      if (meta.pattern == PATTERN::pointer) {
        if (pc2meta[meta.lastpc].pattern == PATTERN::POINTER_A ||
            pc2meta[meta.lastpc].maybe_pointer_chase) {
          meta.maybe_pointer_chase = true;
          flag = true;
        }
      }
      if (meta.pattern == PATTERN::STRUCT_POINTER) {
        if (pc2meta[meta.last_pc_sp].pattern == PATTERN::POINTER_A ||
            pc2meta[meta.last_pc_sp].maybe_pointer_chase) {
          meta.maybe_pointer_chase = true;
          flag = true;
        }
      }
    }
  }
  LOG(INFO) << "End Pointer Chase Correct Cycle" << std::endl;
  for (auto &[pc, meta] : pc2meta) {
    if (meta.maybe_pointer_chase) {
      accessCount[to_underlying(PATTERN::POINTER_A)] += meta.count;
      pcCount[to_underlying(PATTERN::POINTER_A)]++;
    } else {
      accessCount[to_underlying(meta.pattern)] += meta.count;
      pcCount[to_underlying(meta.pattern)]++;
    }
  }
  if (out) {
    for (auto &[pc, meta] : pc2meta) {
      out << std::hex << (pc >> 1) << " ";
      meta.output(out);
    }
    out.close();
  }
  std::ofstream fout(filename);
  fout << "==================================" << std::endl;
  fout << "Total Access\t" << totalCnt << std::endl;
  for (int i = 0; i < PATTERN_NUM; i++) {
    fout << MY_ALIGN_STR(PATTERN_NAME[i]) << MY_ALIGN(accessCount[i])
         << PERCENT(accessCount[i], totalCnt) << std::endl;
  }
  fout << "==================================" << std::endl;
  fout << "Total PC\t" << pc2meta.size() << std::endl;
  for (int i = 0; i < PATTERN_NUM; i++) {
    fout << MY_ALIGN_STR(PATTERN_NAME[i]) << MY_ALIGN(pcCount[i])
         << PERCENT(pcCount[i], pc2meta.size()) << std::endl;
  }
  fout << "==================================" << std::endl;
  if (enable_hotregion_) {
    std::ofstream hrout(hot_region_file);
#ifdef ENABLE_HOTREGION_V1
    auto region_cnt_v =
        std::vector<std::pair<unsigned long long, unsigned long long>>(
            region_cnt.begin(), region_cnt.end());
    std::sort(region_cnt_v.begin(), region_cnt_v.end(),
              [&](std::pair<unsigned long long, unsigned long long> A,
                  std::pair<unsigned long long, unsigned long long> B) {
                return A.second > B.second;
              });
    for (auto &[k, v] : region_cnt_v) {
      hrout << std::hex << k << " " << std::hex << (k + hot_region_size) << " "
            << std::dec << MY_ALIGN(v) << PERCENT(v, totalCnt) << std::endl;
      if (v * 100 < totalCnt) break;
    }
#else
    for (auto &region : hot_region_list) {
      hrout << std::hex << region << " " << std::hex
            << (region + hot_region_size) << std::endl;
    }
#endif
  }

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

void TraceList::merge(std::string input_dir, std::string id,
                      unsigned long long &inst_id) {
  auto filename = std::filesystem::path(input_dir)
                      .append(id)
                      .append("mpr.pattern")
                      .string();
  std::ifstream in(filename);
  if (!in.good()) {
    LOG(WARNING) << "Cannot open file " << filename;
    return;
  }
  unsigned long long pc;
  while (in >> std::hex >> pc) {
    auto it = pc2meta.find(pc);
    if (it != pc2meta.end()) {
      auto past_pattern = it->second.pattern;
      auto past_count = it->second.count;
      it->second.input(in);
      LOG_IF(WARNING, past_pattern != it->second.pattern &&
                          past_pattern != PATTERN::OTHER &&
                          it->second.pattern != PATTERN::OTHER &&
                          past_pattern != PATTERN::FRESH &&
                          it->second.pattern != PATTERN::FRESH)
          << "Conflict Found! " << std::hex << pc << " in file " << std::dec
          << it->second.file_id << " ("
          << PATTERN_NAME[to_underlying(past_pattern)]
          << ") is different from file " << id << " ("
          << PATTERN_NAME[to_underlying(it->second.pattern)] << ").";
      if (it->second.pattern == PATTERN::OTHER ||
          it->second.pattern == PATTERN::FRESH)
        it->second.pattern = past_pattern;
      else {
        it->second.file_id = id;
      }
      inst_id += it->second.count;
      it->second.count += past_count;
    } else {
      auto tmp = PCmeta();
      tmp.input(in);
      pc2meta[pc] = tmp;
      inst_id += tmp.count;
    }
  }
}