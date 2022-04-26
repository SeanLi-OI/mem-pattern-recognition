#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
struct PC_block {
  unsigned long long max_offset;
  unsigned long long counter;
  PC_block() { max_offset = counter = 0; }
  PC_block(unsigned long long offset) : max_offset(offset), counter(1) {}
};
std::unordered_map<unsigned long long, PC_block> pc_cnt;
bool comp(std::pair<unsigned long long, PC_block> a,
          std::pair<unsigned long long, PC_block> b) {
  return a.second.counter > b.second.counter;
}
void parse(std::string str) {
  std::istringstream in;
  std::string str_offset;
  unsigned long long pc, pc_offset, pc_base;
  in.str(str);
  in >> std::hex >> pc >> str_offset;
  if (pc == 0 || str_offset[0] == '[') return;
  std::size_t pos = str_offset.find("+");
  if (pos == std::string::npos) return;
  in.str(str_offset.substr(pos + 1));
  in >> std::hex >> pc_offset;
  pc_base = pc - pc_offset;
  auto it = pc_cnt.find(pc_base);
  if (it == pc_cnt.end()) {
    pc_cnt[pc_base] = PC_block(pc_offset);
  } else {
    it->second.max_offset = std::max(it->second.max_offset, pc_offset);
    it->second.counter++;
  }
}
int main(int argc, char* argv[]) {
  std::ifstream input(argv[1]);
  std::ofstream output(argv[2]);
  for (std::string line; std::getline(input, line);) {
    // std::cerr << "[" << line[0] << "]" << std::endl;
    if (line[0] != '\t' && line[0] != ' ') continue;
    parse(line);
  }
  std::vector<std::pair<unsigned long long, PC_block>> elems(pc_cnt.begin(),
                                                             pc_cnt.end());
  std::sort(elems.begin(), elems.end(), comp);
  for (auto& kv : elems) {
    output << "0x" << std::hex << kv.first << " 0x" << std::hex
           << kv.first + kv.second.max_offset << " " << std::dec
           << kv.second.counter << std::endl;
  }
  return 0;
}