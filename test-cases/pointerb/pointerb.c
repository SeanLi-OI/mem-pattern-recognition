#include <stdlib.h>
int main() {
  int N = 10000, i;
  int* val[N];
  int now;
  for (i = 0; i < N; i++) {
    val[i] = (int*)malloc(sizeof(int));
    (*val[i]) = rand() % N;
  }
  for (i = 0; i < N; i++) {
    now = *val[i];  // pointerB
  }
  return 0;
}