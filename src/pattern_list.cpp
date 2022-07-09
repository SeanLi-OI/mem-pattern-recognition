// Author: Lixiang

#include "pattern_list.h"

#include <glog/logging.h>

#include <fstream>

#include "utils/macro.h"

PatternList::PatternList(const char filename[]) {
  std::ifstream in(filename);
  unsigned long long pc;
  while (in >> std::hex >> pc) {
    pc2meta[pc] = PCmeta();
    pc2meta[pc].input(in);
  }
  std::fill(hit_count, hit_count + PATTERN_NUM, 0);
  std::fill(total_count, total_count + PATTERN_NUM, 0);
  std::fill(all_count, all_count + PATTERN_NUM, 0);
}

void PatternList::add_trace(unsigned long long int pc,
                            unsigned long long int addr,
                            unsigned long long int value, bool isWrite,
                            unsigned long long int &id, const int inst_id) {
  auto it_meta = pc2meta.find(pc);
  if (it_meta == pc2meta.end()) {
    LOG(WARNING) << "Cannot find PC " << pc << " in pattern list." << std::endl;
    return;
  }
  int lastest_id = 0;
  switch (it_meta->second.pattern) {
    case PATTERN::pointer:
    case PATTERN::POINTER_B:
      next_addr[pc] = pc2meta[it_meta->second.lastpc].lastvalue;
      break;
    case PATTERN::INDIRECT:
      next_addr[pc] = pc2meta[it_meta->second.pc_value.value].lastvalue *
                          it_meta->second.pc_value.offset +
                      it_meta->second.pc_value.addr;
      break;
    case PATTERN::STRUCT_POINTER:
      for (auto &c : it_meta->second.struct_pointer_candidate) {
        auto it2 = pc2id.find(c.first);
        if (it2 == pc2id.end() || it2->second < lastest_id) continue;
        lastest_id = it2->second;
        next_addr[pc] = pc2meta[c.first].lastvalue + c.second.offset;
      }
      break;
    case PATTERN::POINTER_A:
      next_addr[pc] =
          (long long)value + it_meta->second.pointerA_offset_candidate;
      break;
    default:
      break;
  }
  auto pattern_now = to_underlying(it_meta->second.maybe_pointer_chase
                                       ? PATTERN::POINTER_A
                                       : it_meta->second.pattern);
  all_count[pattern_now]++;
  auto it = next_addr.find(pc);
  if (it != next_addr.end()) {
    total_count[pattern_now]++;
    if (it->second == addr) hit_count[pattern_now]++;
  }
  switch (it_meta->second.pattern) {
    case PATTERN::STATIC:
      // if (addr == it_meta->second.lastaddr)
      //   next_addr[pc] = it_meta->second.lastaddr_2;
      // else
      next_addr[pc] = it_meta->second.lastaddr;
      break;
    case PATTERN::STRIDE:
      if (it_meta->second.lastaddr_2 != 0) {
        if (addr > it_meta->second.lastaddr_2) {
          next_addr[pc] =
              it_meta->second.lastaddr + addr - it_meta->second.lastaddr_2;
        } else {
          next_addr[pc] =
              it_meta->second.lastaddr - (it_meta->second.lastaddr_2 - addr);
        }
      }
      break;
    default:
      break;
  }
  it_meta->second.pointerA_offset_candidate =
      (long long)addr - (long long)value;
  it_meta->second.lastaddr_2 = it_meta->second.lastaddr;
  it_meta->second.lastaddr = addr;
  it_meta->second.lastvalue = value;
  pc2id[pc] = id;
}

void PatternList::printStats(unsigned long long totalCnt,
                             const char filename[]) {
  LOG_IF(ERROR, totalCnt == 0) << "Read 0 trace from tracefile" << std::endl;
  std::ofstream out(filename);
  unsigned long long hit_sum = 0, total_sum = 0, all_sum = 0;
  out << "=================================================" << std::endl;
  out << "                Hit         Predict     Total" << std::endl;
  for (int i = 0; i < PATTERN_NUM; i++) {
    if (static_cast<PATTERN>(i) == PATTERN::CHAIN) continue;
    out << PATTERN_NAME[i] << "\t" << MY_ALIGN(hit_count[i])
        << MY_ALIGN(total_count[i]) << MY_ALIGN(all_count[i])
        << PERCENT(hit_count[i], total_count[i]) << std::endl;
    hit_sum += hit_count[i];
    total_sum += total_count[i];
    all_sum += all_count[i];
  }
  out << "=================================================" << std::endl;
  out << "Cover       : " << MY_ALIGN(total_sum) << PERCENT(total_sum, all_sum)
      << std::endl;
  out << "Hit         : " << MY_ALIGN(hit_sum) << PERCENT(hit_sum, total_sum)
      << std::endl;
  out << "Hit Overall : " << MY_ALIGN(hit_sum) << PERCENT(hit_sum, all_sum)
      << std::endl;
  out << "=================================================" << std::endl;
}