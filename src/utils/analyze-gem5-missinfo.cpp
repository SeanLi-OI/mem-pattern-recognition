#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "pc_meta.h"

namespace analyze_gem5_missinfo {

DEFINE_string(pattern, "", "pattern file");
DEFINE_string(missinfo, "", "gem5 missinfo file");

std::map<unsigned long long, PCmeta> pc2meta;
std::vector<unsigned long long> cnt(PATTERN_NUM, 0);

unsigned long long readhex(std::string s) {
  unsigned long long n = 0;
  for (int i = 0; i < s.length(); i++) {
    if (s[i] >= 'a' && s[i] <= 'f')
      n = n * 16 + s[i] - 'a' + 10;
    else if (s[i] >= '0' && s[i] <= '9')
      n = n * 16 + s[i] - '0';
    else
      break;
  }
  return n;
}

unsigned long long readdec(std::string s) {
  unsigned long long n = 0;
  int i;
  for (i = 0; i < s.length(); i++) {
    if (s[i] >= '0' && s[i] <= '9') {
      break;
    }
  }
  LOG_IF(ERROR, i >= s.length())
      << "Error when parse decimal number from " << s << std::endl;
  for (; i < s.length() && s[i] >= '0' && s[i] <= '9'; i++) {
    n = n * 10 + s[i] - '0';
  }
  return n;
}
}  // namespace analyze_gem5_missinfo

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  LOG_IF(ERROR, analyze_gem5_missinfo::FLAGS_pattern == "")
      << "Require missinfo file by -pattern=" << std::endl;
  std::ifstream fin1(analyze_gem5_missinfo::FLAGS_pattern);
  LOG_IF(ERROR, !fin1.good())
      << "Cannot open pattern file " << analyze_gem5_missinfo::FLAGS_pattern
      << std::endl;
  LOG_IF(ERROR, analyze_gem5_missinfo::FLAGS_missinfo == "")
      << "Require missinfo file by -missinfo=" << std::endl;
  std::ifstream fin2(analyze_gem5_missinfo::FLAGS_missinfo);
  LOG_IF(ERROR, !fin2.good())
      << "Cannot open missinfo file " << analyze_gem5_missinfo::FLAGS_missinfo
      << std::endl;
  std::string str;
  unsigned long long pc;
  unsigned long long count;
  std::cout << "Read pattern info from " << analyze_gem5_missinfo::FLAGS_pattern
            << std::endl;
  std::cout << "Read gem5 miss info from "
            << analyze_gem5_missinfo::FLAGS_missinfo << std::endl;
  while (fin1 >> std::hex >> pc) {
    analyze_gem5_missinfo::pc2meta[pc] = PCmeta();
    analyze_gem5_missinfo::pc2meta[pc].input(fin1);
  }
  while (std::getline(fin2, str)) {
    auto pos = str.find("PC: ");
    if (pos == std::string::npos) continue;
    pos += 4;
    str = str.substr(pos);
    pc = analyze_gem5_missinfo::readhex(str);
    pos = str.find("miss count: ");
    if (pos == std::string::npos) continue;
    pos += 12;
    str = str.substr(pos);
    count = analyze_gem5_missinfo::readdec(str);
    auto meta = analyze_gem5_missinfo::pc2meta.find(pc);
    if (meta == analyze_gem5_missinfo::pc2meta.end()) {
      LOG(WARNING) << "Cannot find pc 0x" << std::hex << pc
                   << " from pattern file." << std::endl;
      continue;
    }
    analyze_gem5_missinfo::cnt[to_underlying(meta->second.pattern)] += count;
  }
  for (int i = 0; i < PATTERN_NUM; i++) {
    std::cout << PATTERN_NAME[i] << " " << analyze_gem5_missinfo::cnt[i]
              << std::endl;
  }
  return 0;
}