#include <gflags/gflags.h>
#include <glog/logging.h>
#include <stdio.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "utils/macro.h"
DEFINE_string(result_dir, "", "");
DEFINE_string(output, "parse.res", "");
double get_percent(std::string &str) {
  int len = str.length();
  int st = 0;
  while (st < len && str[st] != '(') st++;
  LOG_IF(ERROR, st == len) << "Cannot find percent in " << str << std::endl;
  st++;
  int ed = st + 1;
  while (ed < len && str[ed] != '%') ed++;
  LOG_IF(ERROR, ed == len) << "Cannot find percent in " << str << std::endl;
  return std::stod(str.substr(st, ed - st));
}
int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  FLAGS_result_dir = std::filesystem::absolute(FLAGS_result_dir);
  double sum_c = 0, sum_a = 0;
  int num = 0;
  for (auto const &dir_entry : std::filesystem::directory_iterator{
           std::filesystem::path{FLAGS_result_dir}}) {
    std::string res_file = dir_entry.path() / "mpr.res";
    std::ifstream fin1(res_file);
    if (!fin1.good()) {
      LOG(WARNING) << "Cannot open res file " << res_file << std::endl;
      continue;
    }
    LOG(INFO) << "Parsing " << res_file << std::endl;
    std::string app;
    for (auto &s : dir_entry.path()) {
      if (s != "") app = s;
    }
    std::cout << MY_ALIGN_W(app, 35);
    std::string str;
    while (std::getline(fin1, str)) {
      if (str.substr(0, 5) == "Cover") {
        double coverage = get_percent(str);
        sum_c += coverage;
        std::cout << MY_ALIGN_W(coverage, 6);
      }
      if (str.substr(0, 5) == "Hit  ") {
        double accurancy = get_percent(str);
        sum_a += accurancy;
        std::cout << MY_ALIGN_W(accurancy, 6) << std::endl;
      }
    }
    num++;
    std::string stat_file = dir_entry.path() / "mpr.stat";
    std::ifstream fin2(stat_file);
    if (!fin2.good()) {
      LOG(WARNING) << "Cannot open stat file " << res_file << std::endl;
      continue;
    }
    LOG(INFO) << "Parsing " << res_file << std::endl;
    FLAGS_output = std::filesystem::absolute(FLAGS_output);
    std::ofstream fout2(FLAGS_output);
    if (!fout2.good()) {
      LOG(WARNING) << "Cannot open output file " << FLAGS_output << std::endl;
      continue;
    }
    bool flag = false;
    fout2 << app;
    while (std::getline(fin2, str)) {
      if (str[0] == '=') {
        if (!flag)
          flag = true;
        else
          break;
      } else {
        if (str[0] >= 'a' && str[0] <= 'z') {
          fout2 << "," << get_percent(str);
        }
      }
    }
  }
  std::cout << MY_ALIGN_W("Average", 35)
            << MY_ALIGN_W(RAW_PERCENT(sum_c / 100.0, num), 6)
            << MY_ALIGN_W(RAW_PERCENT(sum_a / 100.0, num), 6) << std::endl;
  return 0;
}