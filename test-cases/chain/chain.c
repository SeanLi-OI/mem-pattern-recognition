#include <stdlib.h>
struct tree {
  struct tree* next;
  int val;
};
int main() {
  int N = 10000, i, k;
  struct tree* root = (struct tree*)malloc(sizeof(struct tree));
  struct tree* now = root;
  for (i = 1; i <= N; i++) {
    now->next = (struct tree*)malloc(sizeof(struct tree));
    now = now->next;
  }
  now = root;
  for (i = 1; i <= N; i++) {
    now = now->next; // pointerA
    k = now->val; // chain
  }
  return 0;
}