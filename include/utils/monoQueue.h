#ifndef MONO_QUEUE_H
//Author: Lixiang

#ifndef MONO_QUEUE_H
#define MONO_QUEUE_H
#include <cstddef>
#include <deque>
#include <map>
template <typename T, std::size_t N>
class MonoQueue {
 private:
  std::deque<T> Q;
  std::array<T, N> V;
  std::size_t id;
  std::size_t calc_interval(std::size_t prev, std::size_t next) {
    if (next >= prev)
      return next - prev;
    else
      return N - next + prev;
  }

 public:
  MonoQueue() : id(0) {}
  void push(T val) {
    V[id] = val;
    if (!Q.empty() && calc_interval(Q.front(), id) >= N) Q.pop_front();
    while (!Q.empty() && V[Q.back()] < V[id]) Q.pop_back();
    Q.push_back(id++);
    if (id == N) id = 0;
  }
  std::pair<T, T> get() {  // Get <Min,Max> of Q
    return std::make_pair(Q.back(), Q.front());
  }
};
#endif