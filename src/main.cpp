// Author: Lixiang

#include <gflags/gflags.h>
#include <stdio.h>

#include "pattern_list.h"
#include "trace_list.h"
#include "tracereader.h"

DEFINE_bool(analyze, true, "Do analyze");
DEFINE_bool(validate, false, "Do validate");
DEFINE_string(trace, "mpr.trace.gz", "mpr trace file");
DEFINE_string(stat, "mpr.stat", "stat file");
DEFINE_string(pattern, "mpr.pattern", "pattern file");
DEFINE_string(result, "mpr.res", "validate result");

void add_trace(TraceList &traceList, int &id, unsigned long long int ip,
               MemRecord &r, bool isWrite, const int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  traceList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
  // if (ip == 0x404b09)
  //   fprintf(stderr, "%c %llx %llx %llx\n", isWrite ? 'W' : 'R',
  //           (unsigned long long)ip, (unsigned long long)r.addr, tmp);
}
void valid_trace(PatternList &patternList, int &id, unsigned long long int ip,
                 MemRecord &r, bool isWrite, const int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  patternList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
  // if (ip == 0x404b09)
  //   fprintf(stderr, "%c %llx %llx %llx\n", isWrite ? 'W' : 'R',
  //           (unsigned long long)ip, (unsigned long long)r.addr, tmp);
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_analyze == true) {
    std::cout << "====================MPR Start===================="
              << std::endl;
    std::cout << "Reading trace from " << FLAGS_trace << std::endl;
    std::cout << "Writing stats to " << FLAGS_stat << std::endl;
    std::cout << "Writing pattern to " << FLAGS_pattern << std::endl;
    auto traces = get_tracereader(FLAGS_trace, 1, 0);
    auto traceList = TraceList();
    int id = 0;
    int inst_id = 0;
    bool isend = false;
    traceList.add_outfile(FLAGS_pattern.c_str());
    while (true) {
      auto inst = traces->get(isend);
      if (isend) break;
      inst_id++;
      // std::cerr << std::hex << inst.ip << " " << inst.func_name << " "
      //           << inst.image << std::endl;
      add_trace(traceList, id, inst.ip, inst.r0, 0, inst_id);
      add_trace(traceList, id, inst.ip, inst.r1, 0, inst_id);
      add_trace(traceList, id, inst.ip, inst.w0, 1, inst_id);
    }
    traceList.printStats(id, FLAGS_stat.c_str());
    std::cout << "=====================MPR End====================="
              << std::endl;
  }
  if (FLAGS_validate == true) {
    std::cout << "=================Validate Start=================="
              << std::endl;
    std::cout << "Reading trace from " << FLAGS_trace << std::endl;
    std::cout << "Reading pattern to " << FLAGS_pattern << std::endl;
    std::cout << "Writing result to " << FLAGS_result << std::endl;
    auto traces = get_tracereader(FLAGS_trace, 1, 0);
    auto patterns = PatternList(FLAGS_pattern.c_str());
    int id = 0;
    int inst_id = 0;
    bool isend = false;
    while (true) {
      auto inst = traces->get(isend);
      if (isend) break;
      inst_id++;
      // std::cerr << std::hex << inst.ip << " " << inst.func_name << " "
      //           << inst.image << std::endl;
      valid_trace(patterns, id, inst.ip, inst.r0, 0, inst_id);
      valid_trace(patterns, id, inst.ip, inst.r1, 0, inst_id);
      valid_trace(patterns, id, inst.ip, inst.w0, 1, inst_id);
    }
    patterns.printStats(id, FLAGS_result.c_str());
    std::cout << "==================Validate End==================="
              << std::endl;
  }
  return 0;
}