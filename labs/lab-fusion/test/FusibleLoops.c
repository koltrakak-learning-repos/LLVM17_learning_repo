// per produrre llvm-ir:
//
// clang –O0 -Xclang -disable-O0-optnone –emit-llvm –S –c Loop.c –o Loop.O0.ll
//
// opt –passes=mem2reg test/Loop.O0.ll -S -o test/Loop.O0.m2r.ll

#define N 64

void fusion_test() {
  int buffer[N] = {0};
  int buffer2[N] = {0};

  for (int i = 0; i < N; i++) {
    buffer[i] = i;
  }

  for (int i = 0; i < N; i++) {
    buffer2[i] = buffer[i];
  }

  // fondendo i loop ottengo qualcosa di questo tipo
  //   for (int i = 0; i < N; i++) {
  //     buffer[i] = i;
  //     buffer2[i] = buffer[i];
  //   }
  //
  // questo mantiene il dato prodotto nel primo loop in cache, pronto per il
  // secondo loop.
  //
  // ottimizzazione aggiuntive si accorgono anche che il primo buffer che serve
  // solo a salvare il risultato intermedio è eliminabile
  //
  //   for (int i = 0; i < N; i++) {
  //     // buffer[i] = i;
  //     buffer2[i] = i;
  //   }
}
