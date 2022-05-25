// Author: Lixiang

#include <gflags/gflags.h>
#include <stdio.h>

#include "macro.h"
#include "pattern_list.h"
#include "trace_list.h"
#include "tracereader.h"

DEFINE_bool(analyze, false, "Do analyze");
DEFINE_bool(validate, false, "Do validate");
DEFINE_bool(debug, false, "Do validate");
DEFINE_string(trace, "mpr.trace.gz", "mpr trace file");
DEFINE_string(stat, "mpr.stat", "stat file");
DEFINE_string(pattern, "mpr.pattern", "pattern file");
DEFINE_string(result, "mpr.res", "validate result");
DEFINE_uint64(len, 0, "trace len");

void add_trace(TraceList &traceList, unsigned long long &id,
               unsigned long long int ip, MemRecord &r, bool isWrite,
               const int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  traceList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
  //   if (ip == 0x40484f)
  // fprintf(stderr, "%c %llx %llx %llx\n", isWrite ? 'W' : 'R',
  //         (unsigned long long)ip, (unsigned long long)r.addr, tmp);
}
void valid_trace(PatternList &patternList, unsigned long long &id,
                 unsigned long long int ip, MemRecord &r, bool isWrite,
                 const int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  patternList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
  // if (ip == 0x101a908)
  // if ((ip >> 2) == 0x406a1e)
  //   fprintf(stderr, "%c %llx %llx %llx\n", isWrite ? 'W' : 'R',
  //           (unsigned long long)ip, (unsigned long long)r.addr, tmp);
}
void debug_trace(TraceList &traceList, unsigned long long &id,
                 unsigned long long int ip, MemRecord &r, bool isWrite,
                 const int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  if (ip == 0x401661) {
    id++;
    fprintf(stderr, "%c %llx %llx %llx\n", isWrite ? 'W' : 'R',
            (unsigned long long)ip, (unsigned long long)r.addr, tmp);
    //     traceList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
  }
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
    unsigned long long id = 0;
    unsigned long long inst_id = 0;
    bool isend = false;
    traceList.add_outfile(FLAGS_pattern.c_str());
    while (true) {
      auto inst = traces->get(isend);
      inst_id++;
      if (isend) break;
// std::cerr << std::hex << inst.ip << " " << inst.func_name << " "
//           << inst.image << std::endl;
#ifdef ENABLE_PC_DIFF
      add_trace(traceList, id, inst.ip << 2, inst.r0, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      add_trace(traceList, id, (inst.ip << 2) | 1, inst.r1, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      add_trace(traceList, id, (inst.ip << 2) | 2, inst.w0, 1, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
#else
      add_trace(traceList, id, inst.ip, inst.r0, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      add_trace(traceList, id, inst.ip, inst.r1, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      add_trace(traceList, id, inst.ip, inst.w0, 1, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
#endif
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
    unsigned long long id = 0;
    int inst_id = 0;
    bool isend = false;
    while (true) {
      auto inst = traces->get(isend);
      if (isend) break;
      inst_id++;
      // std::cerr << std::hex << inst.ip << " " << inst.func_name << " "
      //           << inst.image << std::endl;
#ifdef ENABLE_PC_DIFF
      valid_trace(patterns, id, inst.ip << 2, inst.r0, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      valid_trace(patterns, id, (inst.ip << 2) | 1, inst.r1, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      valid_trace(patterns, id, (inst.ip << 2) | 2, inst.w0, 1, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
#else
      valid_trace(patterns, id, inst.ip, inst.r0, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      valid_trace(patterns, id, inst.ip, inst.r1, 0, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
      valid_trace(patterns, id, inst.ip, inst.w0, 1, inst_id);
      if (id == FLAGS_len && FLAGS_len != 0) break;
#endif
    }
    patterns.printStats(id, FLAGS_result.c_str());
    std::cout << "==================Validate End==================="
              << std::endl;
  }
  if (FLAGS_debug == true) {
    std::cout << "=================Debug Start==================" << std::endl;
    std::cout << "Reading trace from " << FLAGS_trace << std::endl;
    std::cout << "Reading pattern to " << FLAGS_pattern << std::endl;
    auto traces = get_tracereader(FLAGS_trace, 1, 0);
    auto traceList = TraceList();
    unsigned long long id = 0;
    int inst_id = 0;
    bool isend = false;
    while (true) {
      auto inst = traces->get(isend);
      if (isend) break;
      inst_id++;
#ifdef ENABLE_PC_DIFF
      debug_trace(id, inst.ip << 2, inst.r0, 0, inst_id);
      if (id == FLAGS_len) break;
      debug_trace(id, (inst.ip << 2) | 1, inst.r1, 0, inst_id);
      if (id == FLAGS_len) break;
      debug_trace(id, (inst.ip << 2) | 2, inst.w0, 1, inst_id);
      if (id == FLAGS_len) break;
#else
      debug_trace(traceList, id, inst.ip, inst.r0, 0, inst_id);
      if (id == FLAGS_len) break;
      debug_trace(traceList, id, inst.ip, inst.r1, 0, inst_id);
      if (id == FLAGS_len) break;
      debug_trace(traceList, id, inst.ip, inst.w0, 1, inst_id);
      if (id == FLAGS_len) break;
#endif
      // if (inst.ip == 0x40318f) std::cerr << std::endl;
    }
    traceList.printStats(id, FLAGS_stat.c_str());
    std::cout << "==================Debug End===================" << std::endl;
  }
  return 0;
}