// Author: Lixiang
#ifndef MCMF_H
#define MCMF_H
#include <glog/logging.h>

#include <algorithm>
#include <deque>
#include <vector>

namespace MCMF {
const int inf = 1e9;
class MCMF {
 private:
  std::vector<int> to_, nxt_, c_, fir_;
  std::vector<double> w_, dis_;
  std::vector<bool> vis_;
  int N_, S_, T_;
  double Ans_;

  bool spfa(int s, int t) {
    vis_ = std::vector<bool>(N_, false);
    for (int i = 0; i < N_; i++) dis_[i] = inf;
    dis_[t] = 0;
    vis_[t] = true;
    std::deque<int> Q;
    Q.push_back(t);
    while (!Q.empty()) {
      int u = Q.front();
      Q.pop_front();
      for (int i = fir_[u]; i; i = nxt_[i])
        if (c_[i ^ 1] && dis_[to_[i]] > dis_[u] - w_[i]) {
          dis_[to_[i]] = dis_[u] - w_[i];
          if (!vis_[to_[i]]) {
            vis_[to_[i]] = true;
            if (!Q.empty() && dis_[to_[i]] < dis_[Q.front()])
              Q.push_front(to_[i]);
            else
              Q.push_back(to_[i]);
          }
        }
      vis_[u] = 0;
    }
    return dis_[S_] < inf;
  }

  int Aug(int u, int flow) {
    if (u == T_) {
      vis_[T_] = true;
      return flow;
    }
    int used = 0, delta = 0;
    vis_[u] = true;
    for (int i = fir_[u]; i; i = nxt_[i])
      if (!vis_[to_[i]] && c_[i] && dis_[u] == dis_[to_[i]] + w_[i]) {
        delta = Aug(to_[i], std::min(c_[i], flow - used));
        Ans_ += delta * w_[i], c_[i] -= delta, c_[i ^ 1] += delta,
            used += delta;
        if (used == flow) break;
      }
    return used;
  }
  void add(int u, int v, int cc, double wt) {
    to_.emplace_back(v);
    nxt_.emplace_back(fir_[u]);
    fir_[u] = nxt_.size() - 1;
    c_.emplace_back(cc);
    w_.emplace_back(wt);
  }

 public:
  MCMF(int N) {
    N_ = N;
    fir_ = std::vector<int>(N);
    dis_ = std::vector<double>(N);
    Ans_ = 0;
  }
  void add_edge(int u, int v, int cc, double wt) {
    add(u, v, cc, wt);
    add(v, u, 0, -wt);
  }
  int mcmf(int S, int T) {
    S_ = S;
    T_ = T;
    int flow = 0;
    while (spfa(S, T)) {
      vis_[T] = 1;
      while (vis_[T]) {
        vis_ = std::vector<bool>(N_, false);
        flow += Aug(S, inf);
      }
    }
    return flow;
  }
  std::vector<int> get_choice() {
    std::vector<int> choice;
    for (int i = 1; i < c_.size(); i += 2) {
      if (to_[i] != S_) continue;
      if (c_[i] != 0) choice.emplace_back(to_[i - 1]);
    }
    std::sort(choice.begin(), choice.end());
    return choice;
  }
};
}  // namespace MCMF

#endif