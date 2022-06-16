// Author: Lixiang

#include "pc_meta.h"

#include <glog/logging.h>

#include <iostream>

void PCmeta::input(std::ifstream &in) {
  {
    std::string pattern_name;
    in >> pattern_name >> count;
    auto it = pattern_table.find(pattern_name);
    LOG_IF(FATAL, it == pattern_table.end())
        << "Cannot find pattern " << pattern_name << std::endl;
    pattern = it->second;
  }
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
    case PATTERN::INDIRECT:
      in >> std::hex >> pc_value.value >> std::hex >> pc_value.addr >>
          std::dec >> pc_value.offset;
      break;
    case PATTERN::STRUCT_POINTER:
      in >> std::hex >> last_pc_sp >> std::dec >> offset_sp >>
          maybe_pointer_chase;
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
    case PATTERN::INDIRECT:
      out << " " << std::hex << pc_value.value << " " << std::hex
          << pc_value.addr << " " << std::dec << pc_value.offset << std::endl;
      break;
    case PATTERN::STRUCT_POINTER:
      out << " " << std::hex << last_pc_sp << " " << std::dec << offset_sp
          << " " << maybe_pointer_chase << std::endl;
      break;
    default:
      out << std::endl;
  }
}