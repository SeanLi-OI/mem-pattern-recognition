int main() {
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
  return 0;
}