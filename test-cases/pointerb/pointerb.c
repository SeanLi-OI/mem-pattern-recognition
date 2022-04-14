#include <stdlib.h>
struct tree {
  int val;
  struct tree* next;
};
int main() {
  int N = 10000, i;
  struct tree* root[N];
  int now;
  for (i = 0; i < N; i++) {
    root[i] = (struct tree*)malloc(sizeof(struct tree));
  }
  for (i = 0; i < N; i++) {
    now = root[i]->val;  // pointerB
  }
  return 0;
}