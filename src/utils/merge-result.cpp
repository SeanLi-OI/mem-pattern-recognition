// Author: Lixiang

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <omp.h>

#include <filesystem>
#include <memory>

#include "pattern_list.h"
#include "pc_meta.h"
#include "trace_list.h"
#include "tracereader.h"

DEFINE_bool(analyze_result, false, "Do analyze");
DEFINE_bool(validate_result, false, "Do validate");
DEFINE_string(input_dir, "", "input dir");
DEFINE_string(trace_dir, "", "trace dir");
DEFINE_string(stat, "mpr.stat", "stat file");
DEFINE_string(pattern, "mpr.pattern", "pattern file");
DEFINE_string(result, "mpr.res", "validate result");
DEFINE_uint64(len, 0, "len");
DEFINE_uint64(debug_print_interval, 10000000, "Size of debug print interval");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  if (FLAGS_analyze_result) {
    LOG(INFO) << "===========Merge Analyze Result Start===========";
    FLAGS_input_dir = std::filesystem::absolute(FLAGS_input_dir);
    FLAGS_pattern = std::filesystem::absolute(FLAGS_pattern);
    FLAGS_stat = std::filesystem::absolute(FLAGS_stat);
    LOG(INFO) << "Reading patterns from " << FLAGS_input_dir << std::endl;
    LOG(INFO) << "Writing pattern to " << FLAGS_pattern << std::endl;
    LOG(INFO) << "Writing stats to " << FLAGS_stat << std::endl;
    auto traceList = TraceList(0, false);
    traceList.add_outfile(FLAGS_pattern.c_str());
    unsigned long long inst_id = 0;
    for (int i = 1; i <= FLAGS_len; i++) {
      traceList.merge(FLAGS_input_dir, i, inst_id);
    }
    traceList.printStats(inst_id, FLAGS_stat.c_str());
    LOG(INFO) << "============Merge Analyze Result End============";
  }
  if (FLAGS_validate_result) {
    LOG(INFO) << "==========Validate Merged Result Start==========";
    FLAGS_trace_dir = std::filesystem::absolute(FLAGS_trace_dir);
    FLAGS_pattern = std::filesystem::absolute(FLAGS_pattern);
    FLAGS_result = std::filesystem::absolute(FLAGS_result);
    LOG(INFO) << "Reading trace in " << FLAGS_trace_dir << std::endl;
    LOG(INFO) << "Reading pattern to " << FLAGS_pattern << std::endl;
    LOG(INFO) << "Writing result to " << FLAGS_result << std::endl;
    auto patternlist = PatternList(FLAGS_pattern.c_str());
    unsigned long long id = 0;
#pragma omp parallel for
    for (int i = 1; i <= FLAGS_len; i++) {
      auto filename = std::filesystem::path(FLAGS_trace_dir)
                          .append(std::to_string(i))
                          .append("roi_trace.gz")
                          .string();
      LOG(INFO) << "Reading trace in " << filename << std::endl;
      unsigned long long id_ = 0;
      int inst_id = 0;
      auto traces = get_tracereader(filename, 1, 0);
      bool isend = false;
      auto patterns = PatternList(patternlist.pc2meta);
      while (true) {
        auto inst = traces->get(isend);
        if (isend) break;
        inst_id++;
        valid_trace(patterns, id_, inst.ip, inst.r0, 0, inst_id);
        valid_trace(patterns, id_, inst.ip, inst.r1, 0, inst_id);
        valid_trace(patterns, id_, inst.ip, inst.w0, 1, inst_id);
      }
#pragma omp critical
      {
        id += id_;
        for (int j = 0; j < PATTERN_NUM; j++) {
          patternlist.hit_count[j] += patterns.hit_count[j];
          patternlist.all_count[j] += patterns.all_count[j];
          patternlist.total_count[j] += patterns.total_count[j];
        }
      }
    }
    patternlist.printStats(id, FLAGS_result.c_str());
    LOG(INFO) << "===========Validate Merged Result End===========";
  }
  return 0;
}