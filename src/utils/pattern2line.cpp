// Author: Lixiang

#include <assert.h>

#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "utils/macro.h"
#include "pc_meta.h"

const std::string sep = "\t";

// Execute shell command and get stdout
std::string exec(const char* cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

// Int to Hex String
template <typename T>
std::string int_to_hex(T i) {
  std::stringstream stream;
  stream << std::hex << i;
  return stream.str();
}

std::unordered_map<unsigned long long, std::pair<PCmeta, unsigned>> pc2meta;

std::string get_file_line(const char file_name[], int line_no) {
  std::ifstream fin(file_name);
  std::string s;
  if (!fin) {
    std::cerr << file_name << std::endl;
  }
  assert(fin);
  line_no--;
  while (getline(fin, s) && line_no) line_no--;
  assert(line_no == 0);
  return s;
}
std::string pc2line(char bin_file[], unsigned long long pc) {
  return "";
  auto ret =
      exec(("addr2line -Cfie " + std::string(bin_file) + " " + int_to_hex(pc))
               .c_str());
  if (ret[0] == '?') return "";
  int len = ret.length(), offset1, offset2, offset3;
  for (offset1 = 0; offset1 < len && ret[offset1] != '\n'; offset1++)
    ;
  assert(offset1 < len);
  for (offset2 = ++offset1; offset2 < len && ret[offset2] != ':'; offset2++)
    ;
  assert(offset2 < len);
  for (offset3 = ++offset2; offset3 < len && isdigit(ret[offset3]); offset3++)
    ;
  assert(offset3 <= len);
  std::string file_name = ret.substr(offset1, offset2 - offset1 - 1);
  std::string line_no = ret.substr(offset2, offset3 - offset2);
  if (file_name[0] == '?' || line_no.length() == 0) return "";
  return get_file_line(file_name.c_str(), std::stoi(line_no));
}

int main(int argc, char* argv[]) {
  std::ifstream in1, in2;
  std::ofstream out;
  assert(argc > 4);
  in1.open(argv[1]);
  assert(in1);
  in2.open(argv[2]);
  assert(in2);

  unsigned long long pc, addr;
  std::string pattern;
  int instr_id;
  while (in2 >> std::hex >> pc) {
    std::cout << std::hex << pc << std::endl;
    pc2meta[pc] = std::make_pair(PCmeta(), 0);
    pc2meta[pc].first.input(in2);
  }
  while (in1 >> std::dec >> instr_id >> std::hex >> pc >> std::hex >> addr) {
    auto it = pc2meta.find(pc);
    if (it == pc2meta.end()) {
      std::cerr << "Cannot find PC:" << std::hex << pc << " in " << argv[2]
                << std::endl;
      continue;
    }
    assert(it != pc2meta.end());
    it->second.second++;
  }
  in1.close();
  in2.close();
  out.open(argv[3], std::ios::out);
  assert(out);
  // out << "PC" << sep << "Count" << sep << "Miss Count" << sep << "Pattern"
  //     << std::endl;
  for (auto& [pc, meta] : pc2meta) {
#ifdef ENABLE_PC_DIFF
    auto ret = pc2line(argv[4], pc >> 2);
#else
    auto ret = pc2line(argv[4], pc);
#endif
    out << std::hex << pc << sep << std::dec << meta.first.count << sep
        << std::dec << meta.second << sep
        << PATTERN_NAME[to_underlying(meta.first.pattern)] << sep << ret
        << std::endl;
  }
  return 0;
}