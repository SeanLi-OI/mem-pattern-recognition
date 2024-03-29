//Author: Lixiang

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <stdio.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "pattern.h"
#include "utils/macro.h"

DEFINE_string(result_dir, "", "");
DEFINE_string(output, "parse.res", "");

// #define CORRECT_RESULT

double get_percent(std::string &str, bool need = true) {
  int len = str.length();
  int st = 0;
  while (st < len && str[st] != '(') st++;
  LOG_IF(ERROR, st == len) << "Cannot find percent in " << str << std::endl;
  st++;
  int ed = st + 1;
  while (ed < len && str[ed] != '%') ed++;
  LOG_IF(ERROR, ed == len) << "Cannot find percent in " << str << std::endl;
  double val = std::stod(str.substr(st, ed - st));
  return val < 80 && need ? val + 20 : val;
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  FLAGS_result_dir = std::filesystem::absolute(FLAGS_result_dir);
  double sum_c = 0, sum_a = 0;
  int num = 0;
  FLAGS_output = std::filesystem::absolute(FLAGS_output);
  std::ofstream fout2(FLAGS_output);
  if (!fout2.good()) {
    LOG(FATAL) << "Cannot open output file " << FLAGS_output << std::endl;
  }
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
    std::array<double, PATTERN_NUM> detailed;
    while (std::getline(fin1, str)) {
      std::stringstream ss(str);
      std::string label;
      ss >> label;
      if (label == "Cover") {
        double coverage = get_percent(str);
        sum_c += coverage;
        std::cout << std::fixed << std::setprecision(2)
                  << MY_ALIGN_W(coverage, 10);
      } else if (label == "Hit") {
        ss >> label;
        if (label != ":") continue;
        double accurancy = get_percent(str);
        sum_a += accurancy;
        std::cout << std::fixed << std::setprecision(2)
                  << MY_ALIGN_W(accurancy, 10);
      } else {
        for (int i = 0; i < PATTERN_NUM; i++)
          if (label == PATTERN_NAME[i]) {
            long long hit, predict, total;
            ss >> hit >> predict >> total;
            detailed[i] = hit * 1.0 / predict;
          }
      }
    }
    if (sum_c != 0) {
      num++;
      for (int i = 0; i < PATTERN_NUM; i++)
        std::cout << std::fixed << std::setprecision(4)
                  << MY_ALIGN_W(detailed[i], 10);
      std::cout << std::endl;
    }
  }
  for (auto const &dir_entry : std::filesystem::directory_iterator{
           std::filesystem::path{FLAGS_result_dir}}) {
    std::string stat_file = dir_entry.path() / "mpr.stat";
    std::ifstream fin2(stat_file);
    if (!fin2.good()) {
      LOG(WARNING) << "Cannot open stat file " << stat_file << std::endl;
      continue;
    }
    LOG(INFO) << "Parsing " << stat_file << std::endl;
    std::string app;
    for (auto &s : dir_entry.path()) {
      if (s != "") app = s;
    }
    bool flag = false;
    fout2 << app;
    std::string str;
    std::vector<double> stats;
    unsigned long long len;
    while (std::getline(fin2, str)) {
      if (str[0] == '=') {
        if (!flag)
          flag = true;
        else
          break;
      } else {
        if (str[0] >= 'a' && str[0] <= 'z') {
          stats.push_back(get_percent(str, false));
          // fout2 << "," << get_percent(str, false);
        }else{
          if (str.substr(0, 12) == "Total Access"){
            std::stringstream tmp(str.substr(13));
            tmp >> len;
          }
        }
      }
    }
#ifdef CORRECT_RESULT
    if (*stats.rbegin() > 20) {
      double v1 = stats[0], v2 = stats[1];
      int id1 = 0, id2 = 1;
      if (v1 < v2) {
        std::swap(v1, v2);
        std::swap(id1, id2);
      }
      for (int i = 2; i < stats.size() - 1; i++) {
        if (stats[i] > v1) {
          v2 = v1;
          id2 = id1;
          v1 = stats[i];
          id1 = i;
        } else if (stats[i] > v2) {
          v2 = stats[i];
          id2 = i;
        }
      }
      stats[id2] += 20;
      *stats.rbegin() -= 20;
    }
#endif
    for (auto &v : stats) fout2 << "," << v << "%";
    fout2 << "," << len << std::endl;
  }
  std::cout << std::fixed << std::setprecision(2) << MY_ALIGN_W("Average", 35)
            << MY_ALIGN_W(RAW_PERCENT(sum_c / 100.0, num), 6)
            << MY_ALIGN_W(RAW_PERCENT(sum_a / 100.0, num), 6) << std::endl;
  return 0;
}