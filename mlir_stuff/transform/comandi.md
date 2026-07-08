
`mlir-opt test.mlir --pass-pipeline="builtin.module(transform-interpreter{debug-bind-trailing-args=linalg.matmul,linalg.elementwise})"`

- con binding espliciti

`mlir-opt test.mlir --transform-interpreter`

- se usi matchars per ottenere le handles iniziali
