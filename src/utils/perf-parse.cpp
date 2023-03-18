//Author: Lixiang

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <fstream>

#include "utils/macro.h"
#include "perf-parse/perf-parse-common.h"
#include "perf-parse/perf-parse-cycle.h"
#include "perf-parse/perf-parse-miss.h"

DEFINE_string(cycle, "", "cycle perf data");
DEFINE_string(miss, "", "miss perf data");
DEFINE_string(output, "", "output file");
DEFINE_string(roi, "", "roi file");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  /***************************** Cycle Mode *****************************/
  if (FLAGS_cycle != "") {
    LOG(INFO) << "=============== Perf Parse Start [Cycle Mode] ==============="
              << std::endl;
    LOG(INFO) << "Input: " << FLAGS_cycle << std::endl;
    std::ifstream input(FLAGS_cycle);
    LOG_IF(FATAL, input.fail())
        << "Cannot open cycle file " << FLAGS_cycle << std::endl;
    for (std::string line; std::getline(input, line);) {
      if (line[0] != '\t' && line[0] != ' ') continue;
      parse_cycle(line);
    }
  }

  /***************************** Miss Mode *****************************/
  if (FLAGS_miss != "") {
    LOG(INFO) << "============== Perf Parse Start [Miss Mode] ==============="
              << std::endl;
    LOG(INFO) << "Input: " << FLAGS_miss << std::endl;
    std::ifstream input(FLAGS_miss);
    LOG_IF(FATAL, input.fail())
        << "Cannot open miss file " << FLAGS_miss << std::endl;
    for (std::string line; std::getline(input, line);) {
      if (line[0] != '\t' && line[0] != ' ') continue;
      parse_miss(line);
    }
  }

  /***************************** Output *****************************/
  if (FLAGS_cycle != "" || FLAGS_miss != "") {
    std::ofstream output(FLAGS_output);
    LOG_IF(FATAL, output.fail())
        << "Cannot open output file " << FLAGS_output << std::endl;
    std::vector<std::pair<unsigned long long, PC_block>> elems(pc_cnt.begin(),
                                                               pc_cnt.end());
    std::sort(elems.begin(), elems.end(),
              [](std::pair<unsigned long long, PC_block> a,
                 std::pair<unsigned long long, PC_block> b) {
                return a.second.counter > b.second.counter;
              });

    for (auto& [pc, block] : elems) {
      output << "0x" << std::hex << pc << " 0x" << std::hex
             << pc + block.max_offset << " " << PERCENT(block.counter, totalCnt)
             << " " << std::dec << MY_ALIGN(block.counter) << block.func_name
             << std::endl;
    }
    LOG(INFO) << "Output: " << FLAGS_output << std::endl;
    if (FLAGS_roi != "") {
      std::ofstream roi(FLAGS_roi);
      LOG_IF(FATAL, roi.fail())
          << "Cannot open roi file " << FLAGS_roi << std::endl;
      for (auto& [pc, block] : elems) {
        if (block.counter * 10 < totalCnt) break;
        roi << "0x" << std::hex << pc << " 0x" << std::hex
            << pc + block.max_offset << std::endl;
      }
      LOG(INFO) << "ROI: " << FLAGS_output << std::endl;
    }
    LOG(INFO) << "=============== Perf Parse End ["
              << (FLAGS_cycle != "" ? "Cycle" : "Miss")
              << " Mode] ===============" << std::endl;
  }
  return 0;
}