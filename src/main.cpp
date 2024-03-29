// Author: Lixiang

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <filesystem>

#include "pattern_list.h"
#include "trace_list.h"
#include "tracereader.h"
#include "utils/macro.h"

DEFINE_bool(analyze, false, "Do analyze");
DEFINE_bool(validate, false, "Do validate");
DEFINE_bool(debug, false, "Do debug");
DEFINE_bool(hotregion, false, "Do hotregion");
DEFINE_string(trace, "mpr.trace.gz", "mpr trace file");
DEFINE_string(stat, "mpr.stat", "stat file");
DEFINE_string(pretrain, "", "pretrain pattern file");
DEFINE_string(pattern, "mpr.pattern", "pattern file");
DEFINE_string(result, "mpr.res", "validate result");
DEFINE_string(hotregionresult, "mpr.hotregion", "hotregion result");
DEFINE_int64(len, -1, "trace len");
DEFINE_uint64(hrsize, 2048, "Size of hot region");
DEFINE_uint64(debug_print_interval, 10000000, "Size of debug print interval");

void debug_trace(TraceList &traceList, unsigned long long &id,
                 unsigned long long int ip, MemRecord &r, bool isWrite,
                 const unsigned long long int inst_id) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  // if (trace_filter(ip, isWrite, tmp)) return;
  if(ip>=0x12c82 &&ip<=0x12c96){
  // if ((ip >= 0x401846 && ip <= 0x40184e) || ip == 0x418f05) {
  // if (ip == 0x401821 || ip == 0x401822 || ip == 0x401830 || ip == 0x40183e) {
  // if (ip >= 0x50a505 && ip <= 0x50a554) {
  // if(inst_id>=1794&&inst_id<=2000){
  // if (inst_id>=6856070&&inst_id<=6856210) {
  // if (r.addr == 0x6f9cb0) {
  id++;
  fprintf(stderr, "%c %llx %llx %llx %d inst_id:%llu\n", isWrite ? 'W' : 'R',
          (unsigned long long)ip, (unsigned long long)r.addr, tmp, (int)r.len,
          inst_id);
  traceList.add_trace(ip, r.addr, tmp, isWrite, ++id, inst_id);
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  if (FLAGS_analyze == true) {
    LOG(INFO) << "====================MPR Start===================="
              << std::endl;
    FLAGS_trace = std::filesystem::absolute(FLAGS_trace);
    FLAGS_stat = std::filesystem::absolute(FLAGS_stat);
    FLAGS_pattern = std::filesystem::absolute(FLAGS_pattern);
    LOG(INFO) << "Reading trace from " << FLAGS_trace << std::endl;
    LOG(INFO) << "Writing stats to " << FLAGS_stat << std::endl;
    LOG(INFO) << "Writing pattern to " << FLAGS_pattern << std::endl;
    auto traces = get_tracereader(FLAGS_trace, 1, 0);
    auto traceList = TraceList(FLAGS_hrsize, FLAGS_hotregion);
    if (FLAGS_pretrain != "") {
      FLAGS_pretrain = std::filesystem::absolute(FLAGS_pretrain);
      LOG(INFO) << "Reading pattern to " << FLAGS_pretrain << std::endl;
      traceList =
          TraceList(FLAGS_pretrain.c_str(), FLAGS_hrsize, FLAGS_hotregion);
    }
    unsigned long long id = 0;
    unsigned long long inst_id = 0;
    bool isend = false;
    unsigned long long next_debug_print = FLAGS_debug_print_interval;
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
      if (inst_id == next_debug_print) {
        LOG(INFO) << "Finish " << std::dec << inst_id
                  << " instructions' analyzation." << std::endl;
        next_debug_print += FLAGS_debug_print_interval;
      }
    }
    traceList.printStats(id, FLAGS_stat.c_str(), FLAGS_hotregionresult.c_str());
    LOG(INFO) << "=====================MPR End====================="
              << std::endl;
  }
  if (FLAGS_debug == true) {
    LOG(INFO) << "=================Debug Start==================" << std::endl;
    FLAGS_trace = std::filesystem::absolute(FLAGS_trace);
    LOG(INFO) << "Reading trace from " << FLAGS_trace << std::endl;
    //     LOG(INFO) << "Reading pattern to " << FLAGS_pattern << std::endl;
    auto traces = get_tracereader(FLAGS_trace, 1, 0);
    auto traceList = TraceList(FLAGS_hrsize, FLAGS_hotregion);
    traceList.add_outfile(FLAGS_pattern.c_str());
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
    traceList.printStats(id, FLAGS_stat.c_str(), FLAGS_hotregionresult.c_str());
    LOG(INFO) << "==================Debug End===================" << std::endl;
  }
  if (FLAGS_validate == true) {
    LOG(INFO) << "=================Validate Start=================="
              << std::endl;
    FLAGS_trace = std::filesystem::absolute(FLAGS_trace);
    FLAGS_pattern = std::filesystem::absolute(FLAGS_pattern);
    FLAGS_result = std::filesystem::absolute(FLAGS_result);
    LOG(INFO) << "Reading trace from " << FLAGS_trace << std::endl;
    LOG(INFO) << "Reading pattern to " << FLAGS_pattern << std::endl;
    LOG(INFO) << "Writing result to " << FLAGS_result << std::endl;
    auto traces = get_tracereader(FLAGS_trace, 1, 0);
    auto patterns = PatternList(FLAGS_pattern.c_str());
    unsigned long long id = 0;
    int inst_id = 0;
    bool isend = false;
    unsigned long long next_debug_print = FLAGS_debug_print_interval;
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
      if (inst_id == next_debug_print) {
        LOG(INFO) << "Finish " << std::dec << inst_id
                  << " instructions' validation." << std::endl;
        next_debug_print += FLAGS_debug_print_interval;
      }
    }
    patterns.printStats(id, FLAGS_result.c_str());
    LOG(INFO) << "==================Validate End==================="
              << std::endl;
  }
  return 0;
}