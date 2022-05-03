// Author: Lixiang

#include "pc_meta.h"

#include <assert.h>

#include <iostream>

void PCmeta::input(std::ifstream &in) {
  {
    std::string pattern_name;
    in >> pattern_name >> count;
    auto it = pattern_table.find(pattern_name);
    if (it == pattern_table.end()) {
      std::cerr << "Cannot find pattern " << pattern_name << std::endl;
      assert(it != pattern_table.end());
    }
    pattern = it->second;
  }
  switch (pattern) {
    case PATTERN::STATIC:
      in >> std::hex >> lastaddr >> std::hex >> lastaddr_2;
      break;
    case PATTERN::POINTER_A:
      in >> pointerA_offset_candidate;
      break;
    case PATTERN::POINTER_B:
      in >> std::hex >> lastpc;
      break;
    case PATTERN::pointer:
      in >> std::hex >> lastpc;
      break;
    case PATTERN::INDIRECT:
      in >> std::hex >> pc_value.value >> std::hex >> pc_value.addr >>
          std::dec >> pc_value.offset;
      break;
    default:
      break;
  }
}

void PCmeta::output(std::ofstream &out) {
  out << PATTERN_NAME[to_underlying(pattern)] << " " << std::dec << count;
  switch (pattern) {
    case PATTERN::STATIC:
      out << " " << std::hex << lastaddr << " " << std::hex << lastaddr_2
          << std::endl;
      break;
    case PATTERN::POINTER_A:
      out << " " << pointerA_offset_candidate << std::endl;
      break;
    case PATTERN::POINTER_B:
      out << " " << std::hex << lastpc << std::endl;
      break;
    case PATTERN::pointer:
      out << " " << std::hex << lastpc << std::endl;
      break;
    case PATTERN::INDIRECT:
      out << " " << std::hex << pc_value.value << " " << std::hex
          << pc_value.addr << " " << std::dec << pc_value.offset << std::endl;
      break;
    default:
      out << std::endl;
  }
}