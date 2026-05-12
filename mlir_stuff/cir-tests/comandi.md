
lowering a llvm:

- nota come lanci vari passi di preparazione

```
cir-opt --dump-pass-pipeline --cir-to-llvm matmul.opt.cir

Pass Manager with 4 passes:
builtin.module(
  cir-hoist-allocas,
  cir-flatten-cfg,
  cir-goto-solver,
  cir-flat-to-llvm
)
```


lowering a llvm-ir:

- cir, lascia degli attributi del tipo cir.xyz anche dopo il lowering a llvm
- siccome mlir-translate non ha registrato il dialetto cir, dobbiamo ignorare esplicitamente quegli attributi

```
mlir-translate --allow-unregistered-dialect --mlir-to-llvmir matmul.llvm.mlir -o matmul.ll
```


dato il file .ll basta invocare clang per produrre l'eseguibile
