// Author: Lixiang

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <omp.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "utils/MCMF.h"
DEFINE_string(hotspot, "hotspot.csv", "hotspot file");
DEFINE_double(capture_ckp_time, 100, "Time of capture one ckp time (seconds)");
DEFINE_double(analyze_time, 100,
              "Time of capture trace and analyze time (seconds)");
DEFINE_uint32(threshold, 65535, "threshold of hotspot");

bool init(std::ifstream &fin, const std::string &filename, unsigned &N,
          std::vector<std::vector<int>> &hb_count) {
  std::string line;
  fin = std::ifstream(filename);
  if (!fin) {
    LOG(FATAL) << "Cannot open " << filename;
    return 1;
  }
  std::getline(fin, line);
  std::istringstream liness(line);
  std::string str;
  while (std::getline(liness, str, ',')) N++;
  hb_count = std::vector<std::vector<int>>(N - 1);
  N -= 2;
  return 0;
}
bool read_line(std::ifstream &fin, std::vector<std::vector<int>> &hb_count) {
  std::string line;
  if (!std::getline(fin, line)) return false;
  std::istringstream liness(line);
  std::string pc;
  std::getline(liness, pc, ',');
  std::string str;
  for (int i = 0; std::getline(liness, str, ','); i++) {
    hb_count[i].emplace_back((int)std::stod(str));
  }
  return true;
}

bool calc(const std::vector<std::vector<int>> &hb_count,
          std::vector<int> &choice, int N, const int &M, const int &max_flow) {
  MCMF::MCMF solver(N + M + 2);
  int S = 0, T = N + M + 1;
  for (int i = 0; i < N; i++) {
    int cnt = 0;
    for (int j = 0; j < M; j++) {
      if (hb_count[i][j] >= FLAGS_threshold) {
        cnt++;
        solver.add_edge(i + 1, N + j + 1, 1, 0);
      }
    }
    solver.add_edge(S, i + 1, MCMF::inf, 1.0 / cnt);
  }
  for (int i = 0; i < M; i++) {
    solver.add_edge(N + i + 1, T, 1, 0);
  }
  int flow = solver.mcmf(S, T);
  LOG(INFO) << "Flow of " << N << ": " << flow;
  if (flow < max_flow) return false;
  choice = solver.get_choice();
  return true;
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  std::ifstream fin;
  FLAGS_hotspot = std::filesystem::absolute(FLAGS_hotspot);
  unsigned N = 0, M = 0;
  std::vector<std::vector<int>> hb_count;
  if (init(fin, FLAGS_hotspot, N, hb_count)) return 0;
  LOG(INFO) << "Get total " << N << " segment of trace.";
  while (read_line(fin, hb_count)) M++;
  double min_time = N * (FLAGS_capture_ckp_time + FLAGS_analyze_time);
  std::vector<int> final_choice(N);
  for (int i = 0; i < N; i++) {
    final_choice[i] = i;
  }
  int max_flow = 0;
  for (int j = 0; j < M; j++) {
    for (int i = 0; i < N; i++) {
      if (hb_count[i][j] >= FLAGS_threshold) {
        max_flow++;
        break;
      }
    }
  }
#pragma omp parallel for
  for (int i = 7; i < N; i++) {
    std::vector<int> choice;
    if (calc(hb_count, choice, i + 1, M, max_flow)) {
      double total_time =
          choice.size() * FLAGS_analyze_time + (i + 1) * FLAGS_capture_ckp_time;
#pragma omp critical
      {
        LOG(INFO) << "Result of " << i << ": " << choice.size();
        if (total_time < min_time) {
          min_time = total_time;
          final_choice = choice;
        }
      }
    }
  }
  bool isFirst = true;
  for (auto &id : final_choice) {
    std::cout << (isFirst ? "" : ",") << id;
    isFirst = false;
  }
  std::cout << std::endl;
  return 0;
}