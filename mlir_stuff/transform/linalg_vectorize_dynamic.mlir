func.func @matmul(
    %A: memref<?x?xf32>,
    %B: memref<?x?xf32>,
    %C: memref<?x?xf32>
) -> memref<?x?xf32> {

  linalg.matmul ins(%A, %B: memref<?x?xf32>, memref<?x?xf32>)
            outs(%C: memref<?x?xf32>)
  return %C: memref<?x?xf32>
}

// transform

// no peeling
// module attributes {transform.with_named_sequence} {
//   transform.named_sequence @__transform_main(%root: !transform.any_op {transform.readonly}) {
//     // matcho la matmul
//     %matmul = transform.structured.match ops{["linalg.matmul"]} in %root : (!transform.any_op) -> !transform.any_op

//     // 1. Static tiling
//     %tiled_matmul, %loop_1, %loop_2, %loop_3 =
//       transform.structured.tile_using_for %matmul tile_sizes [16, 16, 1] : (!transform.any_op)
//       -> (!transform.any_op, !transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)

//     // 2. Vectorize the tiled matmul
//     // NB: le vector sizes devono combaciare con l'iteration space dell'op che
//     // si sta vettorizzando (anche tile)
//     transform.structured.vectorize %tiled_matmul vector_sizes [16, 16, 1] : !transform.any_op

//     transform.yield
//   }
// }

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%root: !transform.any_op {transform.readonly}) {

    %matmul = transform.structured.match ops{["linalg.matmul"]} in %root : (!transform.any_op) -> !transform.any_op

    %tiled, %li, %lj, %lk =
      transform.structured.tile_using_for %matmul tile_sizes [16, 16, 1]
      : (!transform.any_op) -> (!transform.any_op, !transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)

    // 1. peel il loop esterno (M)
    %main_i, %rem_i = transform.loop.peel %li : (!transform.op<"scf.for">) -> (!transform.op<"scf.for">, !transform.op<"scf.for">)

    // 2a. isola il loop N dentro il ramo main_i (M pieno) e peela anche quello
    %match_main = transform.structured.match ops{["scf.for"]} in %main_i : (!transform.op<"scf.for">) -> !transform.op<"scf.for">
    %self_a, %lj_main, %lk_main = transform.split_handle %match_main
      : (!transform.op<"scf.for">) -> (!transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)
    %main_ii, %rem_ij = transform.loop.peel %lj_main : (!transform.op<"scf.for">) -> (!transform.op<"scf.for">, !transform.op<"scf.for">)

    // 2b. isola il loop N dentro il ramo rem_i (M resto) e peela anche quello
    %match_rem = transform.structured.match ops{["scf.for"]} in %rem_i : (!transform.op<"scf.for">) -> !transform.op<"scf.for">
    %self_b, %lj_rem, %lk_rem = transform.split_handle %match_rem
      : (!transform.op<"scf.for">) -> (!transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)
    %rem_ji, %rem_jj = transform.loop.peel %lj_rem : (!transform.op<"scf.for">) -> (!transform.op<"scf.for">, !transform.op<"scf.for">)

    // 3. vettorizza ognuno dei 4 rami risultanti
    %mm_A = transform.structured.match ops{["linalg.matmul"]} in %main_ii : (!transform.op<"scf.for">) -> !transform.any_op
    transform.structured.vectorize %mm_A vector_sizes [16, 16, 1] : !transform.any_op   // M pieno, N pieno   -> NIENTE masking

    %mm_B = transform.structured.match ops{["linalg.matmul"]} in %rem_ij : (!transform.op<"scf.for">) -> !transform.any_op
    transform.structured.vectorize %mm_B vector_sizes [16, 16, 1] : !transform.any_op   // M pieno, N resto   -> masking solo su N

    %mm_C = transform.structured.match ops{["linalg.matmul"]} in %rem_ji : (!transform.op<"scf.for">) -> !transform.any_op
    transform.structured.vectorize %mm_C vector_sizes [16, 16, 1] : !transform.any_op   // M resto, N pieno   -> masking solo su M

    %mm_D = transform.structured.match ops{["linalg.matmul"]} in %rem_jj : (!transform.op<"scf.for">) -> !transform.any_op
    transform.structured.vectorize %mm_D vector_sizes [16, 16, 1] : !transform.any_op   // M resto, N resto   -> masking su entrambi

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
