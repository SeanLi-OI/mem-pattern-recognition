#include <assert.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "pc_meta.h"

std::map<unsigned long long, PCmeta> pc2meta;
std::map<unsigned long long, std::vector<unsigned long long>> pf_pair;

unsigned long long readhex(std::string s) {
  unsigned long long n = 0;
  int i;
  for (i = 0; i + 1 < s.length(); i++) {
    if (s[i] == '0' && s[i + 1] == 'x') {
      i += 2;
      break;
    }
  }
  assert(i + 1 < s.length());
  for (; i < s.length(); i++) {
    if (s[i] >= 'a' && s[i] <= 'f')
      n = n * 16 + s[i] - 'a' + 10;
    else
      n = n * 16 + s[i] - '0';
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
  assert(i < s.length());
  for (; i < s.length() && s[i] >= '0' && s[i] <= '9'; i++) {
    n = n * 10 + s[i] - '0';
  }
  return n;
}

int main(int argc, char *argv[]) {
  std::ifstream fin1(argv[1]);
  std::ifstream fin2(argv[2]);
  std::string str;
  unsigned long long pc;
  std::cerr << "Read pattern info from " << argv[1] << std::endl;
  std::cerr << "Read gem5 prefetch log from " << argv[2] << std::endl;
  while (fin1 >> std::hex >> pc) {
    pc2meta[pc] = PCmeta();
    pc2meta[pc].input(fin1);
    unsigned long long int pf_pc;
    switch (pc2meta[pc].pattern) {
      case PATTERN::STATIC:
        pf_pc = pc;
        break;
      case PATTERN::STRIDE:
        pf_pc = pc;
        break;
      case PATTERN::POINTER_A:
        pf_pc = pc;
        break;
      case PATTERN::STRUCT_POINTER:
        pf_pc = pc2meta[pc].last_pc_sp;
        break;
      case PATTERN::pointer:
        pf_pc = pc2meta[pc].lastpc;
        break;
      case PATTERN::POINTER_B:
        pf_pc = pc2meta[pc].lastpc;
        break;
      case PATTERN::INDIRECT:
        pf_pc = pc2meta[pc].pc_value.value;
        break;
      default:
        pf_pc = pc;
    };
    pf_pair[pf_pc].push_back(pc);
  }
  while (std::getline(fin2, str)) {
    // 8098916813500: system.cpu.dcache.prefetcher: PC:0x401572	Accesses:131072
    // Misses:13127	PFIssued:13703	PFUseful:24092.
    std::string tmp;
    std::stringstream ss(str);
    ss >> tmp;
    ss >> tmp;
    unsigned long long access, miss, pfissue, pfuseful;
    ss >> tmp;
    pc = readhex(tmp);
    ss >> tmp;
    access = readdec(tmp);
    ss >> tmp;
    miss = readdec(tmp);
    ss >> tmp;
    pfissue = readdec(tmp);
    ss >> tmp;
    pfuseful = readdec(tmp);
    auto it = pf_pair.find(pc);

    // static auto add_count =
    //     [&](std::map<unsigned long long, unsigned long long> cnt,
    //         unsigned long long pc) { if (pc2meta.find(pc)) };
    // add_count(issue_count, pf_pc);
    // add_count(useful_count, pc);
    // add_count(access_count, pc);
    // add_count(miss_count, pc);

    // std::cerr << std::hex << pc << " " << std::hex << pf_pc << " " <<
    // std::dec
    //           << access << " " << miss << " " << pfissue << " " << pfuseful
    //           << std::endl;
  }
  return 0;
}