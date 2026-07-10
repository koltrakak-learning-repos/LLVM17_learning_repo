// version on tensors

// func.func @matmul(
//     %A: tensor<1024x512xf32>,
//     %B: tensor<512x2000xf32>,
//     %C: tensor<1024x2000xf32>
// ) -> tensor<1024x2000xf32> {

//   %res = linalg.matmul ins(%A, %B: tensor<1024x512xf32>, tensor<512x2000xf32>)
//             outs(%C: tensor<1024x2000xf32>) -> tensor<1024x2000xf32>
//   return %res : tensor<1024x2000xf32>
// }

// version on memrefs
func.func @matmul(
    %A: memref<1024x512xf32>,
    %B: memref<512x2000xf32>,
    %C: memref<1024x2000xf32>
) -> memref<1024x2000xf32> {

  linalg.matmul ins(%A, %B: memref<1024x512xf32>, memref<512x2000xf32>)
            outs(%C: memref<1024x2000xf32>)
  return %C: memref<1024x2000xf32>
}

// NB: molto interessante il fatto che quando le tile-sizes non matchano le
// input-sizes l'input, le tile hanno dimensione dinamica dato che bisogna
// gestire il caso in cui non si riesce a costruire una tile completa
// func.func @matmul(
//     %A: memref<1024x512xf32>,
//     %B: memref<512x2001xf32>,
//     %C: memref<1024x2001xf32>
// ) -> memref<1024x2001xf32> {

//   linalg.matmul ins(%A, %B: memref<1024x512xf32>, memref<512x2001xf32>)
//             outs(%C: memref<1024x2001xf32>)
//   return %C: memref<1024x2001xf32>
// }



// transform

// no peeling
module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%root: !transform.any_op {transform.readonly}) {
    // matcho la matmul
    %matmul = transform.structured.match ops{["linalg.matmul"]} in %root : (!transform.any_op) -> !transform.any_op

    // 1. Static tiling
    %tiled_matmul, %loop_1, %loop_2, %loop_3 =
      transform.structured.tile_using_for %matmul tile_sizes [8, 16, 1] : (!transform.any_op)
      -> (!transform.any_op, !transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)

    // 2. Vectorize the tiled matmul
    // NB: le vector sizes devono combaciare con l'iteration space dell'op che
    // si sta vettorizzando (anche tile)
    transform.structured.vectorize %tiled_matmul vector_sizes [8, 16, 1] : !transform.any_op

    transform.yield
  }
}

// // with peeling
// module attributes {transform.with_named_sequence} {
//   transform.named_sequence @__transform_main(%root: !transform.any_op {transform.readonly}) {
//     // matcho la matmul
//     %matmul = transform.structured.match ops{["linalg.matmul"]} in %root : (!transform.any_op) -> !transform.any_op

//     // 1. Static tiling
//     %_, %loop_1, %loop_2, %loop_3 =
//       transform.structured.tile_using_for %matmul tile_sizes [8, 16, 1] : (!transform.any_op)
//       -> (!transform.any_op, !transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)

//     // 2. Loop peeling (still on the middle dimension, if needed)
//     %main_loop, %remainder_loop = transform.loop.peel %loop_2 : (!transform.op<"scf.for">) -> (!transform.op<"scf.for">, !transform.op<"scf.for">)

//     // 3. Vectorize the main loop
//     %matmul_main = transform.structured.match ops{["linalg.matmul"]} in %main_loop : (!transform.op<"scf.for">) -> !transform.any_op
//     transform.structured.vectorize %matmul_main vector_sizes [8, 16, 1] : !transform.any_op

//     // 4. Vectorize the remainder loop
//     %matmul_remainder = transform.structured.match ops{["linalg.matmul"]} in %remainder_loop : (!transform.op<"scf.for">) -> !transform.any_op
//     transform.structured.vectorize %matmul_remainder vector_sizes [8, 16, 1] : !transform.any_op

//     transform.yield
//   }
// }
