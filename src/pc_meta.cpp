// Author: Lixiang

#include "pc_meta.h"

#include <glog/logging.h>

#include <iostream>

void PCmeta::input(std::ifstream &in) {
  {
    std::string pattern_name;
    in >> pattern_name  >> count;
    auto it = pattern_table.find(pattern_name);
    LOG_IF(FATAL, it == pattern_table.end())
        << "Cannot find pattern " << pattern_name << std::endl;
    pattern = it->second;
  }
  struct_pointer_meta tmp;
  int n;
  switch (pattern) {
    case PATTERN::STATIC:
      in >> std::hex >> lastaddr;
      break;
    case PATTERN::POINTER_A:
      in >> std::dec >> pointerA_offset_candidate;
      break;
    case PATTERN::POINTER_B:
      in >> std::hex >> lastpc;
      break;
    case PATTERN::pointer:
      in >> std::hex >> lastpc >> maybe_pointer_chase;
      break;
#ifdef OLD_INDIRECT_PATTERN
    case PATTERN::INDIRECT:
      in >> std::hex >> pc_value.value >> std::hex >> pc_value.addr >>
          std::dec >> pc_value.offset;
      break;
#else
    case PATTERN::INDIRECT:
      in >> std::dec >> pc_value.offset >> std::hex >> pc_value.prev_pc;
      break;
#endif
    case PATTERN::STRUCT_POINTER:
      in >> std::dec >> n;
      while (n--) {
        in >> std::hex >> tmp.pc >> std::dec >> tmp.offset;
        struct_pointer_candidate[tmp.pc] = tmp;
      }
      in >> maybe_pointer_chase;
      break;
    default:
      break;
  }
}

void PCmeta::output(std::ofstream &out) {
  out << PATTERN_NAME[to_underlying(pattern)] << " " << std::dec << count;
  switch (pattern) {
    case PATTERN::STATIC:
      out << " " << std::hex << lastaddr << std::endl;
      break;
    case PATTERN::POINTER_A:
      out << " " << std::dec << pointerA_offset_candidate << std::endl;
      break;
    case PATTERN::POINTER_B:
      out << " " << std::hex << lastpc << std::endl;
      break;
    case PATTERN::pointer:
      out << " " << std::hex << lastpc << " " << maybe_pointer_chase
          << std::endl;
      break;
#ifdef OLD_INDIRECT_PATTERN
    case PATTERN::INDIRECT:
      out << " " << std::hex << pc_value.value << " " << std::hex
          << pc_value.addr << " " << std::dec << pc_value.offset << std::endl;
      break;
#else
    case PATTERN::INDIRECT:
      out << " " << std::dec << pc_value.offset << " " << std::hex
          << pc_value.prev_pc << std::endl;
      break;
#endif
    case PATTERN::STRUCT_POINTER:
      out << " " << std::dec << struct_pointer_candidate.size();
      for (auto &c : struct_pointer_candidate) {
        out << " " << std::hex << c.second.pc << " " << std::dec
            << c.second.offset;
      }
      out << " " << maybe_pointer_chase << std::endl;
      break;
    default:
      out << std::endl;
  }
}