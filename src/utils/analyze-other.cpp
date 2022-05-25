#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "pattern.h"
struct Info {
  unsigned long long int ip;
  unsigned long long count;
  Info() {}
  Info(unsigned long long _i, long long _c) : ip(_i), count(_c) {}
  bool operator<(const struct Info &b) { return count > b.count; }
};
int main(int argc, char *argv[]) {
  std::ifstream fin(argv[1]);
  std::string str;
  std::vector<Info> other_list;
  unsigned long long sum = 0;
  std::cerr << "Read from " << argv[1] << std::endl;
  while (std::getline(fin, str)) {
    std::stringstream ss(str);
    unsigned long long int ip;
    std::string pattern;
    unsigned long long count;

    ss >> std::hex >> ip >> pattern >> std::dec >> count;
    if (pattern != "other") continue;

    other_list.emplace_back(Info(ip, count));
    sum += count;
  }
  std::sort(other_list.begin(), other_list.end());

  std::ofstream fout(argv[2]);
  std::cerr << "Write to " << argv[2] << std::endl;
  for (auto &other : other_list) {
    fout << std::hex << other.ip << " " << std::dec << other.count << " "
         << other.count * (double)100.0 / sum << "%" << std::endl;
  }
  return 0;
}