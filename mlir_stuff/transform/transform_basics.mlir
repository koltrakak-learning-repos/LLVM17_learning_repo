func.func @fc_relu(%lhs: tensor<512x512xf32>, %rhs: tensor<512x512xf32>,
                   %bias: tensor<512x512xf32>, %output: tensor<512x512xf32>)
                   -> tensor<512x512xf32> {
  // Matrix-matrix multiplication.
  %matmul = linalg.matmul ins(%lhs, %rhs: tensor<512x512xf32>, tensor<512x512xf32>)
                          outs(%output: tensor<512x512xf32>) -> tensor<512x512xf32>

  // Elementwise addition.
  %biased = linalg.elementwise kind=#linalg.elementwise_kind<add>
    ins(%matmul, %bias : tensor<512x512xf32>, tensor<512x512xf32>)
    outs(%output : tensor<512x512xf32>) -> tensor<512x512xf32>

  // Elementwise max with 0 (ReLU).
  %c0f = arith.constant 0.0 : f32
  %relued = linalg.elementwise kind=#linalg.elementwise_kind<max_signed>
    indexing_maps = [affine_map<(d0, d1) -> (d0, d1)>, affine_map<(d0, d1) -> ()>, affine_map<(d0, d1) -> (d0, d1)>]
    ins(%biased, %c0f : tensor<512x512xf32>, f32)
    outs(%output : tensor<512x512xf32>) -> tensor<512x512xf32>
  func.return %relued : tensor<512x512xf32>
}






// transform

// tile matmul

// module attributes {transform.with_named_sequence} {
//   transform.named_sequence @__transform_main(
//       %arg0: !transform.any_op,
//       %arg1: !transform.op<"linalg.matmul">,
//       %arg2: !transform.op<"linalg.elementwise">) {

//     // The tiling transformation takes tile sizes as attributes.
//     // - consumes the arg1 operand handle (la matmul originale non c'è più)
//     // - produces two new handles (inner matmul e surrounding tile-loop)
//     %loop, %tiled = transform.structured.tile_using_forall %arg1 tile_sizes [32, 32] : (!transform.op<"linalg.matmul">) -> (!transform.any_op, !transform.any_op)

//     // This is trying to use an invalidated handle leading to undefined behavior.
//     // that is detected and diagonosed by the expensive-checks mode
//     // - also works with aliasing handles
//     // - also works with handles that refer to some ir nested inside some other
//     //   invalidated op
//     // -> the analogy is that of pointers. If you delete an object, any pointers
//     // to the object, or to stuff inside it become invalidated.
//     // transform.debug.emit_remark_at %arg1, "remark" : !transform.op<"linalg.matmul">

//     transform.yield
//   }
// }


// tile and fuse

// module attributes {transform.with_named_sequence} {
//   transform.named_sequence @__transform_main(
//        %arg0: !transform.any_op,
//        %arg1: !transform.op<"linalg.matmul">,
//        %arg2: !transform.op<"linalg.elementwise">) {

//     // Since the %arg2 handle is associated with both elementwise operations,
//     // we need to split it into two handles so we can target only the second
//     // elementwise operation.
//     %add, %max = transform.split_handle %arg2
//         : (!transform.op<"linalg.elementwise">)
//         -> (!transform.any_op, !transform.any_op)

//     // The actual tiling transformation takes tile sizes as attributes. It
//     // produces a handle to the loop generated during tiling.
//     %tiled_max, %loop =
//         transform.structured.tile_using_forall %max tile_sizes [32, 32]
//           : (!transform.any_op) -> (!transform.any_op, !transform.any_op)

//     // We can now fuse the other operations into the loop. Here, we fuse
//     // operations one by one. This requires the operation that is being fused to
//     // define the value used within the loop, so the order of such fusions is
//     // important. We could also use "transform.merge_handles" to obtain a single
//     // handle to all operations and give it to `fuse_into_containing_op` that
//     // would take care of the ordering in this case.
//     %add_fused, %loop_0 =
//         transform.structured.fuse_into_containing_op %add into %loop
//           : (!transform.any_op, !transform.any_op)
//             -> (!transform.any_op, !transform.any_op)
//     %matmul_fused, %loop_1 =
//         transform.structured.fuse_into_containing_op %arg1 into %loop_0
//           : (!transform.op<"linalg.matmul">, !transform.any_op)
//             -> (!transform.any_op, !transform.any_op)

//     transform.yield
//   }
// }


// tile and fuse with matching ops

module attributes { transform.with_named_sequence } {

  // Entry point. This takes as the only argument the root operation (typically
  // pass root) given to the transform interpreter.
  transform.named_sequence @__transform_main(%root: !transform.any_op {transform.readonly}) {
    // Collect operations that match the criteria specified in the given named
    // sequence.
    // - If the named sequence fails with a silenceable failure, silences it
    // (the message is forwarded to the debug stream).
    // - If the named sequence succeeds, appends its results to the results of
    // this operation.
    %elemwise = transform.collect_matching @match_elemwise in %root
      : (!transform.any_op) -> !transform.any_op
    %matmul = transform.collect_matching @match_matmul in %root
      : (!transform.any_op) -> !transform.any_op

    // resto delle trasformazioni come prima dati gli handle

    transform.include @print_elemwise failures(propagate)  (%elemwise)
      : (!transform.any_op) -> ()
    transform.include @print_matmul failures(propagate)  (%matmul)
      : (!transform.any_op) -> ()

    transform.yield
  }

  // This is a matcher sequence. It is given an operation to match and the
  // match is considered successful unless any nested operation produces a
  // failure. The values yielded by this operation will be forwarded to the
  // rewriter sequence on success.
  transform.named_sequence @match_elemwise(%entry: !transform.any_op {transform.readonly}) -> !transform.any_op {
    transform.match.operation_name %entry ["linalg.elementwise"] : !transform.any_op
    transform.yield %entry : !transform.any_op
  }
  transform.named_sequence @match_matmul(%entry: !transform.any_op {transform.readonly}) -> !transform.any_op {
    transform.match.operation_name %entry ["linalg.matmul"] : !transform.any_op
    transform.yield %entry : !transform.any_op
  }

  // This is a rewriter sequence.
  transform.named_sequence @print_elemwise(%elemwise_binary: !transform.any_op {transform.readonly}) {
    transform.debug.emit_remark_at %elemwise_binary, "elementwise binary" : !transform.any_op
    transform.yield
  }
  transform.named_sequence @print_matmul(%matmul: !transform.any_op {transform.readonly}) {
    transform.debug.emit_remark_at %matmul, "matmul" : !transform.any_op
    transform.yield
  }
}
