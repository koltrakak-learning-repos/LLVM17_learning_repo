// Matmul tra matrici con dimensioni:
// - A -> MxK
// - B -> KxN
// - C -> MxN
//
// K è la dimensione comune
// void matmul(int *A, int *B, int *C, int M, int N, int K) {
//   for (int i = 0; i < M; i++) {
//     for (int j = 0; j < N; j++) {
//       for (int k = 0; k < K; k++) {
//         C[i * N + j] += A[i * K + k] * B[k * N + j];
//       }
//     }
//   }
// }

// nota che mi compare l'attributo iter_args se uso un accumulatore
// void matmul(int *A, int *B, int *C, int M, int N, int K) {
//   for (int i = 0; i < M; i++) {
//     for (int j = 0; j < N; j++) {
//       int acc = 0;
//       for (int k = 0; k < K; k++) {
//         acc += A[i * K + k] * B[k * N + j];
//       }
//       C[i * N + j] = acc;
//     }
//   }
// }

// Posso utilizzare questa sintassi con VLA per non dover delinearizzare gli
// accessi alle memref (godo)
void matmul(int M, int N, int K, int A[M][K], int B[K][N], int C[M][N]) {
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
      for (int k = 0; k < K; k++) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}
