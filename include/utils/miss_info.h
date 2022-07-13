#include <glog/logging.h>

#include <array>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "pattern.h"
#include "pc_meta.h"
#include "utils/macro.h"

const int METRICS_NUM = 5;
const int PRIMARY_KEY_INTERVAL = 3;
const std::string metric_name[] = {"PC", "misses", "hits", "pfUseful",
                                   "missLatency"};
const bool metric_is_hex[] = {true, false, false, false, false};
class MissInfo {
 private:
  std::shared_ptr<std::map<unsigned long long, PCmeta>> pc2meta_;
  std::shared_ptr<std::ifstream> fin_;

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
  double ipc_;
  std::array<std::array<unsigned long long, METRICS_NUM>, PATTERN_NUM> cnt_;
  std::array<unsigned long long, PATTERN_NUM> cannot_find_;
  std::array<unsigned long long, PATTERN_NUM> total_;
  MissInfo(std::shared_ptr<std::map<unsigned long long, PCmeta>> pc2meta,
           std::shared_ptr<std::ifstream> fin)
      : pc2meta_(pc2meta), fin_(fin), cnt_({}), cannot_find_({}), total_({}) {}
  void read() {
    std::string str;
    while (std::getline(*fin_, str)) {
      if (str.length() > 3 && str[0] == 'i' && str[1] == 'p' && str[2] == 'c') {
        std::stringstream tmp;
        tmp << str.substr(4);
        tmp >> ipc_;
      }
      bool flag = true;
      std::array<unsigned long long, METRICS_NUM> val;
      for (int j = 0; j < METRICS_NUM; j++) {
        val[j] = readstr(str, metric_name[j], metric_is_hex[j]);
        if (val[j] == INT64_MAX) {
          if (j < PRIMARY_KEY_INTERVAL) {
            flag = false;
            break;
          } else {
            LOG(WARNING) << "Cannot find value of (" << metric_name[j] << ")"
                         << " for pc 0x" << std::hex << val[0]
                         << " from missinfo file." << std::endl;
            val[j] = 0;
          }
        }
      }
      if (!flag) continue;
      total_[0]++;
      for (int j = 1; j < METRICS_NUM; j++) {
        total_[j] += val[j];
      }
      auto meta = (*pc2meta_).find(val[0]);
      if (meta == (*pc2meta_).end()) {
        LOG(WARNING) << "Cannot find pc 0x" << std::hex << val[0]
                     << " from pattern file." << std::endl;
        cannot_find_[0]++;
        for (int j = 1; j < METRICS_NUM; j++) {
          cannot_find_[j] += val[j];
        }
        continue;
      }
      auto pattern = meta->second.pattern;
      if (meta->second.maybe_pointer_chase) pattern = PATTERN::POINTER_A;
      cnt_[to_underlying(pattern)][0]++;
      for (int j = 1; j < METRICS_NUM; j++) {
        cnt_[to_underlying(pattern)][j] += val[j];
      }
    }
  }
  void write() {
    std::cout << MY_ALIGN_STR("Pattern");
    for (int j = 0; j < METRICS_NUM; j++) std::cout << MY_ALIGN(metric_name[j]);
    std::cout << std::endl;
    for (int i = 0; i < PATTERN_NUM; i++) {
      std::cout << MY_ALIGN_STR(PATTERN_NAME[i]);
      for (int j = 0; j < METRICS_NUM; j++) {
        std::cout << MY_ALIGN(cnt_[i][j]);
      }
      std::cout << std::endl;
    }

    std::cout << MY_ALIGN_STR("total");
    for (int j = 0; j < METRICS_NUM; j++) std::cout << MY_ALIGN(total_[j]);
    std::cout << std::endl;

    write_unknown();
  }
  void write_unknown() {
    for (int j = 0; j < METRICS_NUM; j++) {
      std::cout << "Unknown pattern " << metric_name[j] << ": "
                << cannot_find_[j] << PERCENT(cannot_find_[j], total_[j])
                << std::endl;
      std::cout << "Total " << metric_name[j] << ": " << total_[j] << std::endl;
    }
  }
};