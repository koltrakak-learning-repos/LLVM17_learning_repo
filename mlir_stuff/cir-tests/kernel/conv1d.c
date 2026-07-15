// Assumo:
// - K dispari
// - no padding -> output è più piccolo
void conv1d(int *output, int *input, int N, int *window, int W) {
  int dim_bordo = (W - 1) / 2;
  int N_output = N - 2 * dim_bordo;

  for (int i = 0; i < N_output; i++) {
    for (int w_i = 0; w_i < W; w_i++) {
      output[i] += input[i + w_i] * window[w_i];
    }
  }
}
