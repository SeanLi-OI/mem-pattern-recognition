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
    case PATTERN::POINTER_A:
      in >> pointerA_offset_candidate;
      break;
  }
}

void PCmeta::output(std::ofstream &out) {
  out << PATTERN_NAME[to_underlying(pattern)] << " " << std::dec << count;
  switch (pattern) {
    case PATTERN::POINTER_A:
      out << " " << pointerA_offset_candidate << std::endl;
      break;
    default:
      out << std::endl;
  }
}