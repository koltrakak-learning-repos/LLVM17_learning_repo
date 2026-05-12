#include <cstdio>

int *matmul(const int *a, const int *b, int *c, int rows_a, int cols_a,
            int rows_b, int cols_b) {

  if (cols_a != rows_b)
    return nullptr;

  for (int i = 0; i < rows_a; i++) {
    for (int j = 0; j < cols_b; j++) {
      c[i * cols_b + j] = 0;
      for (int k = 0; k < cols_a; k++) {
        c[i * cols_b + j] += a[i * cols_a + k] * b[k * cols_b + j];
      }
    }
  }

  return c;
}

void print_matrix(int *m, int rows, int cols);

int main() {
  int a[2][3] = {
      {1, 2, 3},
      {4, 5, 6},
  };
  int b[3][2] = {
      {1, 2},
      {3, 4},
      {5, 6},
  };
  int c[2][2] = {0};

  matmul((int *)a, (int *)b, (int *)c, 2, 3, 3, 2);

  // NB: le print non sono ancora supportate da CIR
  print_matrix((int *)c, 2, 2);

  return 0;
}

void print_matrix(int *m, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      printf("%d ", m[i * cols + j]);
    }
  }
}
