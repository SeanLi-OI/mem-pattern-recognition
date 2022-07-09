#include <glog/logging.h>

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "pattern.h"
#include "pc_meta.h"
#include "utils/macro.h"

class MissInfo {
 private:
  std::shared_ptr<std::map<unsigned long long, PCmeta>> pc2meta;
  std::shared_ptr<std::ifstream> fin2;

  unsigned long long readhex(std::string s) {
    unsigned long long n = 0;
    int i;
    for (i = 0; i < s.length(); i++) {
      if ((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'f')) {
        break;
      }
    }
    LOG_IF(ERROR, i >= s.length())
        << "Error when parse hex number from " << s << std::endl;
    for (; i < s.length(); i++) {
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

  unsigned long long readstr(std::string str, std::string pattern, bool ishex) {
    auto pos = str.find(pattern);
    if (pos == std::string::npos) return INT64_MAX;
    pos += pattern.length();
    return ishex ? readhex(str.substr(pos)) : readdec(str.substr(pos));
  }

 public:
  double ipc;
  std::vector<unsigned long long> miss_cnt;
  std::vector<unsigned long long> hit_cnt;
  std::vector<unsigned long long> total_cnt;
  MissInfo(std::shared_ptr<std::map<unsigned long long, PCmeta>> pc2meta_,
           std::shared_ptr<std::ifstream> fin2_)
      : pc2meta(pc2meta_), fin2(fin2_) {
    miss_cnt = std::vector<unsigned long long>(PATTERN_NUM, 0);
    hit_cnt = std::vector<unsigned long long>(PATTERN_NUM, 0);
    total_cnt = std::vector<unsigned long long>(PATTERN_NUM, 0);
  }
  void read() {
    std::string str;
    while (std::getline(*fin2, str)) {
      if (str.length() > 3 && str[0] == 'i' && str[1] == 'p' && str[2] == 'c') {
        std::stringstream tmp;
        tmp << str.substr(4);
        tmp >> ipc;
      }
      auto pc = readstr(str, "PC:", true);
      if (pc == INT64_MAX) continue;
      auto miss = readstr(str, "misses:", false);
      if (miss == INT64_MAX) continue;
      auto hit = readstr(str, "hits:", false);
      if (hit == INT64_MAX) continue;
      auto meta = (*pc2meta).find(pc);
      if (meta == (*pc2meta).end()) {
        LOG(WARNING) << "Cannot find pc 0x" << std::hex << pc
                     << " from pattern file." << std::endl;
        continue;
      }
      auto pattern = meta->second.pattern;
      if (meta->second.maybe_pointer_chase) pattern = PATTERN::POINTER_A;
      miss_cnt[to_underlying(pattern)] += miss;
      hit_cnt[to_underlying(pattern)] += hit;
      total_cnt[to_underlying(pattern)] += hit + miss;
    }
  }
  void write() {
    std::cout << "Pattern         MissCount    HitCount    TotalCount"
              << std::endl;
    unsigned long long misses = 0, hits = 0, total = 0;
    for (int i = 0; i < PATTERN_NUM; i++) {
      std::cout << MY_ALIGN_STR(PATTERN_NAME[i]) << " " << MY_ALIGN(miss_cnt[i])
                << " " << MY_ALIGN(hit_cnt[i]) << MY_ALIGN(total_cnt[i])
                << std::endl;
      misses += miss_cnt[i];
      hits += hit_cnt[i];
      total += total_cnt[i];
    }

    std::cout << MY_ALIGN_STR("total") << " " << MY_ALIGN(misses) << " "
              << MY_ALIGN(hits) << MY_ALIGN(total) << std::endl;
  }
};