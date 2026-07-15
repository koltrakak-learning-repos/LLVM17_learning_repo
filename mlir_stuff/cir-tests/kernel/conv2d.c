// Assumo:
// - K dispari
// - no padding -> output è più piccolo
// void conv2d(int *output, int *input, int rows, int cols, int *kernel, int K)
// {
//   int dim_bordo = (K - 1) / 2;
//   int rows_output = rows - 2 * dim_bordo;
//   int cols_output = cols - 2 * dim_bordo;

//   for (int i = 0; i < rows_output; i++) {
//     for (int j = 0; j < cols_output; j++) {
//       for (int k_i = 0; k_i < K; k_i++) {
//         for (int k_j = 0; k_j < K; k_j++) {
//           int i_input = i + k_i;
//           int j_input = j + k_j;
//           output[i * cols_output + j] +=
//               input[i_input * cols + j_input] * kernel[k_i * K + k_j];
//         }
//       }
//     }
//   }
// }

int conv2d(int rows_out, int cols_out, int rows_in, int cols_in, int K,
           int output[rows_out][cols_out], int input[rows_in][cols_in],
           int kernel[K][K]) {

  //   int dim_bordo = (K - 1) / 2;
  //   if (rows_out != (rows_in - 2 * dim_bordo) ||
  //       cols_out != cols_in - 2 * dim_bordo)
  //     return -1;

  for (int i = 0; i < rows_out; i++) {
    for (int j = 0; j < cols_out; j++) {
      for (int k_i = 0; k_i < K; k_i++) {
        for (int k_j = 0; k_j < K; k_j++) {
          int i_input = i + k_i;
          int j_input = j + k_j;
          output[i][j] += input[i_input][j_input] * kernel[k_i][k_j];
        }
      }
    }
  }

  return 0;
}
