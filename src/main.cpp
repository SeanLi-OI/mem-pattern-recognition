#include <stdio.h>

#include "trace_list.h"
#include "tracereader.h"

void add_trace(TraceList &traceList, int &id, unsigned long long int ip,
               MemRecord &r, bool isWrite) {
  if (r.len == 0 || r.len > 8) return;
  unsigned long long tmp = 0;
  for (int i = r.len - 1; i >= 0; i--) tmp = tmp * 256 + r.content[i];
  traceList.add_trace(ip, r.addr, tmp, isWrite, ++id);
  fprintf(stderr, "%c %llu %llu %llu\n", isWrite ? 'W' : 'R',
          (unsigned long long)ip, (unsigned long long)r.addr, tmp);
}

int main(int argc, char *argv[]) {
  auto traces = get_tracereader(argv[1], 1, 0);
  auto traceList = TraceList();
  int id = 0;
  bool isend = false;
  while (true) {
    auto inst = traces->get(isend);
    if (isend) break;
    add_trace(traceList, id, inst.ip, inst.r0, 0);
    add_trace(traceList, id, inst.ip, inst.r1, 0);
    add_trace(traceList, id, inst.ip, inst.w0, 1);
  }
  traceList.printStats(id);
  return 0;
}