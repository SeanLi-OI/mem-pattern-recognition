#include <stdlib.h>
struct tree {
  int val;
  struct tree* next;
};
int main() {
  int N = 10000, i;
  struct tree* root = (struct tree*)malloc(sizeof(struct tree));
  struct tree* now = root;
  for (i = 1; i <= N; i++) {
    now->next = (struct tree*)malloc(sizeof(struct tree));
    now = now->next;
  }
  now = root;
  for (i = 1; i <= N; i++) {
    now = now->next; // pointerA
  }
  return 0;
}