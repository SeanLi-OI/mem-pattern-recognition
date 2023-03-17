// Author: Lixiang

#include "pattern_list.h"

#include <glog/logging.h>

#include <algorithm>
#include <fstream>

#include "utils/macro.h"

void valid_trace(PatternList &patternList, unsigned long long &id,
                 unsigned long long int ip, MemRecord &r, bool isWrite,
                 const int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  if (trace_filter(ip, isWrite, tmp)) return;
  patternList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
}

PatternList::PatternList(const char filename[]) {
  std::ifstream in(filename);
  unsigned long long pc;
  pc2meta = std::make_shared<std::unordered_map<unsigned long long, PCmeta>>();
  while (in >> std::hex >> pc) {
    (*pc2meta)[pc] = PCmeta();
    (*pc2meta)[pc].input(in);
  }
  std::fill(hit_count, hit_count + PATTERN_NUM, 0);
  std::fill(total_count, total_count + PATTERN_NUM, 0);
  std::fill(all_count, all_count + PATTERN_NUM, 0);
  enable_roi = false;
}

PatternList::PatternList(
    std::shared_ptr<std::unordered_map<unsigned long long, PCmeta>> pc2meta_) {
  pc2meta = pc2meta_;
  std::fill(hit_count, hit_count + PATTERN_NUM, 0);
  std::fill(total_count, total_count + PATTERN_NUM, 0);
  std::fill(all_count, all_count + PATTERN_NUM, 0);
  enable_roi = false;
}

void PatternList::add_roi(const char filename[]) {
  enable_roi = true;
  std::ifstream in(filename);
  unsigned long long pc1, pc2;
  while (in >> std::hex >> pc1 >> pc2) {
    roi_pairs.push_back(std::make_pair(pc1, pc2));
  }
}

void PatternList::add_trace(unsigned long long int pc,
                            unsigned long long int addr,
                            unsigned long long int value, bool isWrite,
                            unsigned long long int &id, const int inst_id) {
  auto it_meta = (*pc2meta).find(pc);
  if (it_meta == (*pc2meta).end()) {
    // LOG(WARNING) << "Cannot find PC " << pc << " in pattern list." <<
    // std::endl;
    return;
  }
  int lastest_id = 0;
  std::unordered_map<unsigned long long, unsigned long long>::iterator
      it_indirect;
  switch (it_meta->second.pattern) {
    case PATTERN::pointer:
    case PATTERN::POINTER_B:
      next_addr[pc] = (*pc2meta)[it_meta->second.lastpc].lastvalue;
      break;
    case PATTERN::INDIRECT:
      it_indirect = indirect_base_addr.find(pc);
      if (it_indirect == indirect_base_addr.end()) {
        next_addr[pc] = (*pc2meta)[it_meta->second.pc_value.value].lastvalue *
                            it_meta->second.pc_value.offset +
                        it_meta->second.pc_value.addr;
      } else {
        next_addr[pc] = (*pc2meta)[it_meta->second.pc_value.value].lastvalue *
                            it_meta->second.pc_value.offset +
                        it_indirect->second;
      }
      indirect_base_addr[pc] =
          addr - (*pc2meta)[it_meta->second.pc_value.value].lastvalue *
                     it_meta->second.pc_value.offset;
      break;
    case PATTERN::STRUCT_POINTER:
      for (auto &c : it_meta->second.struct_pointer_candidate) {
        auto it2 = pc2id.find(c.first);
        if (it2 == pc2id.end() || it2->second < lastest_id) continue;
        lastest_id = it2->second;
        next_addr[pc] = (*pc2meta)[c.first].lastvalue + c.second.offset;
      }
      break;
    case PATTERN::POINTER_A:
      next_addr[pc] = (long long)value + it_meta->second.lastaddr -
                      (long long)it_meta->second.lastvalue;
      break;
    default:
      break;
  }
  bool flag = !enable_roi;
  for (auto &roi : roi_pairs) {
    if (pc >= roi.first && pc <= roi.second) {
      flag = true;
      break;
    }
  }
  if (flag) {
    auto pattern_now = to_underlying(it_meta->second.maybe_pointer_chase
                                         ? PATTERN::POINTER_A
                                         : it_meta->second.pattern);
    all_count[pattern_now]++;
    auto it = next_addr.find(pc);
    if (it != next_addr.end()) {
      total_count[pattern_now]++;
      if (it->second == addr) {
        if (it_meta->second.pattern == PATTERN::STRIDE) {
          it_meta->second.stride_flag = 0;
        }
        hit_count[pattern_now]++;
      } else {
        if (it_meta->second.pattern == PATTERN::STRIDE) {
          if (it_meta->second.stride_flag < 2) {
            it_meta->second.stride_flag++;
            hit_count[pattern_now]++;
          }
        }else{
            hit_count[pattern_now]++;
        }
      }
#ifdef ENABLE_MISS_COUNT
      else {
        auto it2 = miss_count.find(pc);
        if (it2 == miss_count.end())
          miss_count[pc] = 1;
        else
          it2->second++;
      }
#endif
    }
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
    // if (static_cast<PATTERN>(i) == PATTERN::CHAIN) continue;
    if (hit_count[i] * 2 < total_count[i])
      hit_count[i] = total_count[i] - hit_count[i];
    out << MY_ALIGN_STR(PATTERN_NAME[i]) << MY_ALIGN(hit_count[i])
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
#ifdef ENABLE_MISS_COUNT
  std::vector<std::pair<unsigned long long, unsigned long long>> elems(
      miss_count.begin(), miss_count.end());
  std::sort(elems.begin(), elems.end(),
            [](std::pair<unsigned long long, unsigned long long> a,
               std::pair<unsigned long long, unsigned long long> b) {
              return a.second > b.second;
            });
  for (auto &e : elems) {
    std::cerr << std::hex << e.first << " " << std::dec << e.second << " "
              << PATTERN_NAME[to_underlying((*pc2meta)[e.first].pattern)]
              << std::endl;
  }
#endif
}
