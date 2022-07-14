#include <stdlib.h>

#include <array>
#include <functional>
struct tree {
  struct tree* next;
  int val;
};
struct tree2 {
  int val;
  int next;
};
void pointer_chase() {
  int N = 10000, i, k;
  struct tree* root = (struct tree*)malloc(sizeof(struct tree));
  struct tree* now = root;
  for (i = 1; i <= N; i++) {
    now->next = (struct tree*)malloc(sizeof(struct tree));
    now = now->next;
  }
  now = root;
  for (i = 1; i <= N; i++) {
    now = now->next;  // pointerA
    k = now->val;     // chain
  }
}
void indirect() {
  int N = 10000;
  int A[N], B[N];
  for (int i = 0; i < N; i++) {
    A[i] = rand() % N;
    B[i] = rand() % N;
  }
  int t;
  for (int i = 0; i < N; i++) {
    t = A[B[i]];  // indirect
  }
}
void stride() {
  int N = 10000, a[1920001];
  int stride = 1;
  for (int i = 0; i < N; i += stride) {
    a[i] = N - i;  // stride
  }
  stride = 8;
  for (int i = 0; i < N; i += stride) {
    a[i] = N - i;  // stride
  }
  stride = 64;
  for (int i = 0; i < N; i += stride) {
    a[i] = N - i;  // stride
  }
  stride = 192;
  for (int i = 0; i < N; i += stride) {
    a[i] = N - i;  // stride
  }
}
void struct_pointer() {
  int N = 10000, i;
  struct tree2 a[N];
  for (i = 0; i < N; i++) {
    a[i].val = rand() % N;
    a[i].next = rand() % N;
  }
  int tmp = 0;
  for (i = 1; i <= N; i++) {
    int t = rand() % N;
    struct tree2* t2 = &a[t];
    tmp = t2->next;  // struct_pointer
  }
}
const int num = 4;
int main() {
  std::array<int, num> N = {5, 5, 5, 5};
  std::array<std::function<void()>, num> func = {pointer_chase, indirect,
                                                 stride, struct_pointer};
  bool flag = true;
  while (flag) {
    flag = false;
    for (int i = 0; i < num; i++) {
      if (N[i] > 0) {
        func[i]();
        N[i]--;
        flag = true;
      }
    }
  }
  return 0;
}