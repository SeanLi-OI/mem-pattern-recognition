#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "pc_meta.h"
#include "utils/macro.h"
#include "utils/miss_info.h"
#include "utils/trans_abs_path.h"

DEFINE_string(pattern, "", "pattern file");
DEFINE_string(missinfo, "", "gem5 missinfo file");
DEFINE_string(base, "", "gem5 missinfo file (base)");

void compare_missinfo(std::unique_ptr<MissInfo> missinfo,
                      std::unique_ptr<MissInfo> base) {
  std::cout << "IPC: " << missinfo->ipc
            << PERCENT_WITH_OP(missinfo->ipc - base->ipc, base->ipc)
            << std::endl;
  std::cout << "Pattern         MissCount                   HitCount           "
               "         TotalCount"
            << std::endl;
  unsigned long long misses = 0, hits = 0, total = 0;
  unsigned long long misses_base = 0, hits_base = 0, total_base = 0;
  for (int i = 0; i < PATTERN_NUM; i++) {
    std::cout << MY_ALIGN_STR(PATTERN_NAME[i]) << " "
              << MY_ALIGN(missinfo->miss_cnt[i])
              << MY_ALIGN_STR(DIFF(missinfo->miss_cnt[i], base->miss_cnt[i]))
              << " " << MY_ALIGN(missinfo->hit_cnt[i])
              << MY_ALIGN_STR(DIFF(missinfo->hit_cnt[i], base->hit_cnt[i]))
              << " " << MY_ALIGN(missinfo->total_cnt[i])
              << MY_ALIGN_STR(DIFF(missinfo->total_cnt[i], base->total_cnt[i]))
              << std::endl;
    misses += missinfo->miss_cnt[i];
    hits += missinfo->hit_cnt[i];
    total += missinfo->total_cnt[i];
    misses_base += base->miss_cnt[i];
    hits_base += base->hit_cnt[i];
    total_base += base->total_cnt[i];
  }
  std::cout << MY_ALIGN_STR("total") << " " << MY_ALIGN(misses)
            << MY_ALIGN_STR(DIFF(misses, misses_base)) << " " << MY_ALIGN(hits)
            << MY_ALIGN_STR(DIFF(hits, hits_base)) << " " << MY_ALIGN(total)
            << MY_ALIGN_STR(DIFF(total, total_base)) << std::endl;
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  LOG_IF(ERROR, FLAGS_pattern == "")
      << "Require missinfo file by -pattern=" << std::endl;
  transAbsolute(FLAGS_pattern);
  std::ifstream fin1(FLAGS_pattern);
  LOG_IF(ERROR, !fin1.good())
      << "Cannot open pattern file " << FLAGS_pattern << std::endl;
  LOG_IF(ERROR, FLAGS_missinfo == "")
      << "Require missinfo file by -missinfo=" << std::endl;
  transAbsolute(FLAGS_missinfo);
  std::shared_ptr<std::ifstream> fin2 =
      std::make_shared<std::ifstream>(FLAGS_missinfo);
  LOG_IF(ERROR, !fin2->good())
      << "Cannot open missinfo file " << FLAGS_missinfo << std::endl;
  std::string str;
  unsigned long long pc;
  unsigned long long miss, hit;
  std::shared_ptr<std::map<unsigned long long, PCmeta>> pc2meta =
      std::make_shared<std::map<unsigned long long, PCmeta>>();
  LOG(INFO) << "Read pattern info from " << FLAGS_pattern << std::endl;
  LOG(INFO) << "Read gem5 miss info from " << FLAGS_missinfo << std::endl;
  while (fin1 >> std::hex >> pc) {
    (*pc2meta)[pc] = PCmeta();
    (*pc2meta)[pc].input(fin1);
  }

  std::unique_ptr<MissInfo> missinfo =
      std::make_unique<MissInfo>(pc2meta, fin2);
  missinfo->read();

  if (FLAGS_base == "") {
    missinfo->write();
  } else {
    transAbsolute(FLAGS_base);
    std::shared_ptr<std::ifstream> fin3 =
        std::make_shared<std::ifstream>(FLAGS_base);
    LOG_IF(ERROR, !fin3->good())
        << "Cannot open missinfo file " << FLAGS_base << std::endl;
    std::unique_ptr<MissInfo> base = std::make_unique<MissInfo>(pc2meta, fin3);
    base->read();
    compare_missinfo(std::move(missinfo), std::move(base));
  }
  return 0;
}