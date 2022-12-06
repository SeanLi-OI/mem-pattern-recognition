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
DEFINE_uint32(max_ckp, 0, "maximum num of checkpoints");
DEFINE_uint32(max_ckp_id, 0, "maximum id of checkpoints");
DEFINE_double(capture_ckp_time, 210, "Time of capture one ckp time (seconds)");
DEFINE_double(analyze_time, 45000,
              "Time of capture trace and analyze time (seconds)");
DEFINE_double(fast_forward_time, 750,
              "Time of execute in fast-forward mode for 1 Billion "
              "instructions. (seconds)");
DEFINE_uint32(concurrency, 20, "Concurrency of capturing and analyzing trace");
DEFINE_bool(force, false, "Force to choose ckps");
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
    unsigned cnt = 0;
    for (int j = 0; j < M; j++) {
      if (hb_count[i][j] >= FLAGS_threshold) {
        cnt++;
        solver.add_edge(i + 1, N + j + 1, 1, 0);
        // LOG(INFO) << i + 1 << " " << N + j + 1 << " 1 0";
      }
    }
    if (cnt != 0) {
      solver.add_edge(S, i + 1, MCMF::inf, 100000000ll / cnt);
      // LOG(INFO) << S << " " << i + 1 << " inf " << 100000000 / cnt;
    }
  }
  for (int i = 0; i < M; i++) {
    solver.add_edge(N + i + 1, T, 1, 0);
    // LOG(INFO) << N + i + 1 << " " << T << " 1 0";
  }
  int flow = solver.mcmf(S, T);
  LOG(INFO) << "Flow of " << N << ": " << flow;
  if (flow < max_flow) return false;
  choice = solver.get_choice();
  if (FLAGS_max_ckp != 0 && choice.size() > FLAGS_max_ckp) return false;
  return true;
}

long double solve(const std::vector<std::vector<int>> &hb_count,
                  std::vector<int> &choice, int N, const int &M) {
  std::vector<std::vector<long double>> f(N);
  std::vector<std::vector<int>> ans(N);
  for (int i = 0; i < N; i++) {
    f[i] = std::vector<long double>(FLAGS_max_ckp + 1, -1);
    ans[i] = std::vector<int>(FLAGS_max_ckp + 1, -1);
  }
  std::vector<unsigned long long> cnt(M, 0);
  std::vector<long double> hb_percent(M);
  std::vector<bool> is_hotspot(M, false);
  unsigned long long sum;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      cnt[j] += hb_count[i][j];
      if (hb_count[i][j] >= FLAGS_threshold) is_hotspot[j] = true;
    }
  }
  for (int i = 0; i < M; i++)
    if (is_hotspot[i]) sum += cnt[i];
  for (int i = 0; i < M; i++)
    if (is_hotspot[i]) hb_percent[i] = cnt[i] / (long double)sum;
  std::vector<bool> vis_ckp(N, false), vis(M, false);
  for (int k = 0; k < FLAGS_max_ckp; k++) {
    long double res = -1, res_id = -1;
    for (int i = 0; i < N; i++) {
      if (vis_ckp[i]) continue;
      long double ts_percent = 0;
      for (int j = 0; j < M; j++)
        if (!vis[j] && hb_count[i][j] >= FLAGS_threshold) {
          ts_percent += hb_percent[j];
        }
      if (ts_percent > res) {
        res = ts_percent;
        res_id = i;
      }
    }
    LOG_IF(FATAL, res < 0) << "FATAL in finding alternative solution.";
    choice.push_back(res_id);
    vis_ckp[res_id] = true;
    for (int j = 0; j < M; j++)
      if (hb_count[res_id][j] >= FLAGS_threshold) vis[j] = true;
  }
  std::sort(choice.begin(), choice.end());
  long double res = 0;
  for (int j = 0; j < M; j++)
    if (vis[j]) res += hb_percent[j];
  return res;
}

void print(std::vector<int> &choice) {
  bool isFirst = true;
  for (auto &id : choice) {
    std::cout << (isFirst ? "" : ",") << id;
    isFirst = false;
  }
  std::cout << std::endl;
}

int main(int argc, char *argv[]) {
  gflags::SetVersionString("1:0.0.0");
  gflags::SetUsageMessage("Usage: ./choose_ckp -hotspot_file=${hotspot_file}");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  FLAGS_logtostderr = 1;
  std::ifstream fin;
  FLAGS_hotspot = std::filesystem::absolute(FLAGS_hotspot);
  unsigned N = 0, M = 0;
  std::vector<std::vector<int>> hb_count;
  if (init(fin, FLAGS_hotspot, N, hb_count)) return 0;
  LOG(INFO) << "Get total " << N << " segment of trace.";
  if (FLAGS_max_ckp_id != 0 && FLAGS_max_ckp_id < N) {
    N = FLAGS_max_ckp_id;
    LOG(INFO) << "Flag limits N to " << N << ".";
  }
  while (read_line(fin, hb_count)) M++;
  double min_time = N * (FLAGS_analyze_time + FLAGS_capture_ckp_time +
                         FLAGS_fast_forward_time) +
                    5;
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
  for (int i = 4; i < N; i++) {
    std::vector<int> choice;
    if (calc(hb_count, choice, i + 1, M, max_flow)) {
      double total_time =
          choice.size() * (FLAGS_analyze_time +
                           FLAGS_capture_ckp_time /
                               std::max(i + 1, (int)FLAGS_concurrency)) +
          (i + 1) * (FLAGS_fast_forward_time);
#pragma omp critical
      {
        LOG(INFO) << "Result of " << i + 1 << ": " << choice.size();
        if (total_time < min_time) {
          min_time = total_time;
          final_choice = choice;
        }
      }
    }
  }
  if (min_time > N * (FLAGS_analyze_time + FLAGS_capture_ckp_time +
                      FLAGS_fast_forward_time)) {
    if (FLAGS_force) {
      std::vector<int> choice;
      long double total_percent = solve(hb_count, choice, N, M);
      print(choice);
      std::cout << total_percent << std::endl;
    } else {
      std::cout << -1 << std::endl;
    }
  } else {
    print(final_choice);
  }
  return 0;
}