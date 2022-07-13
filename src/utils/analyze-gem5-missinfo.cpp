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
  std::cout << "IPC: " << missinfo->ipc_
            << PERCENT_WITH_OP(missinfo->ipc_ - base->ipc_, base->ipc_)
            << std::endl;
  std::cout << MY_ALIGN_STR("Pattern");
  for (int j = 0; j < METRICS_NUM; j++)
    std::cout << MY_ALIGN(metric_name[j]) << MY_ALIGN_STR("");
  std::cout << std::endl;
  for (int i = 0; i < PATTERN_NUM; i++) {
    std::cout << MY_ALIGN_STR(PATTERN_NAME[i]);
    for (int j = 0; j < METRICS_NUM; j++) {
      std::cout << MY_ALIGN(missinfo->cnt_[i][j])
                << MY_ALIGN_STR(DIFF(missinfo->cnt_[i][j], base->cnt_[i][j]));
    }
    std::cout << std::endl;
  }
  std::cout << MY_ALIGN_STR("total");
  for (int j = 0; j < METRICS_NUM; j++) {
    std::cout << MY_ALIGN(missinfo->total_[j])
              << MY_ALIGN_STR(DIFF(missinfo->total_[j], base->total_[j]));
  }
  std::cout << std::endl;
  missinfo->write_unknown();
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