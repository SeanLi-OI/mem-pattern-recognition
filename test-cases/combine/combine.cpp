#include <stdlib.h>
#include <string.h>

#include <random>
struct tree {
  struct tree* next;
  int val;
};
struct tree2 {
  int val;
  int next;
};
const int n = 5;
const int N = 10000;
const int my_mem_size = 100000000;
std::mt19937 my_rand;
std::uniform_int_distribution<int> dist(0, my_mem_size - 1);
struct tree* my_mem;
bool* has_alloc;
void init_my_mem() {
  my_mem = new struct tree[my_mem_size];
  has_alloc = new bool[my_mem_size];
  memset(has_alloc, 0, sizeof(bool) * my_mem_size);
}
struct tree* my_new() {
  int pos = dist(my_rand);
  while (has_alloc[pos]) {
    pos = dist(my_rand);
  }
  has_alloc[pos] = true;
  return my_mem + pos;
}
int main() {
  for (int i = 0; i < n; i++) {
    init_my_mem();
    struct tree *root = my_new(), *now = root;  // pointer_chase
    struct tree* array[N];                      // pointer_array
    int* inA = new int[my_mem_size];
    int inB[N];                    // indirect
    int st[1920001];               // stride
    struct tree2 sp[N];            // struct pointer
    for (int j = 0; j < N; j++) {  // init loop
      {
        // pointer_chase init
        now->next = my_new();
        now = now->next;
      }
      {  // indirect init
        inA[j] = dist(my_rand);
        inB[j] = dist(my_rand);
      }
      {  // struct pointer init
        sp[j].val = rand() % N;
        sp[j].next = rand() % N;
      }
      {
        // pointer array
        array[j] = my_new();
      }
    }
    int tmp = 0, t, k;
    now = root;
    for (int j = 0; j < 10000; j++) {  // run loop
      {                                // pointer chase
        now = now->next;
        k = now->val;
      }
      {  // indirect
        t = inA[inB[j]];
      }
      {  // Stride
        st[j] = N - j;
        st[j * 8] = N - j;
        st[j * 64] = N - j;
        st[j * 192] = N - j;
      }
      {  // struct_pointer
        int t = rand() % N;
        struct tree2* t2 = &sp[t];
        tmp = t2->next;
      }
      {
        // pointer array
        k = array[j]->val;
      }
    }
  }
  return 0;
}