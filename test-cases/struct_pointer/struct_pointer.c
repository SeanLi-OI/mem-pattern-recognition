#include <stdlib.h>
struct tree {
  int val;
  int next;
};
int main() {
  int N = 10000, i;
  struct tree a[N];
  for (i = 0; i < N; i++) {
    a[i].val = rand() % N;
    a[i].next = rand() % N;
  }
  int tmp = 0;
  for (i = 1; i <= N; i++) {
    int t = rand() % N;
    struct tree* t2 = &a[t];
    tmp = t2->next;  // struct_pointer
  }
  return 0;
}