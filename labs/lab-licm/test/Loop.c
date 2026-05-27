// Per produrre la IR che sarà l'input del nostro ottimizzatore:
//      clang -O2 -S -emit-llvm -c FILENAME.c -o FILENAME.ll

int licm_loop(int a, int b, int c) {
  for (int i = 0; i < 10; i++) {
    int d = 3;     // l.i. in quanto costante
    int e = a + d; // l.i. in quanto ha usi l.i.
    if (b < 10)    // non l.i. in quanto b ha una definizione fuori e una dentro
      b = b + 4;   // non l.i. in quanto b ha una definizione fuori e una dentro
    int f = e + c; // l.i. in quanto ha usi l.i.
    int g = i + b; // non l.i. in quanto non ha usi l.i.
  }

  return b;
}
