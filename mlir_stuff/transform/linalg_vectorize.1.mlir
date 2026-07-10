module {
  func.func @matmul(%arg0: memref<1024x512xf32>, %arg1: memref<512x2000xf32>, %arg2: memref<1024x2000xf32>) -> memref<1024x2000xf32> {
    %c0 = arith.constant 0 : index
    %c0_0 = arith.constant 0 : index
    %c0_1 = arith.constant 0 : index
    %c1024 = arith.constant 1024 : index
    %c2000 = arith.constant 2000 : index
    %c512 = arith.constant 512 : index
    %c8 = arith.constant 8 : index
    %c16 = arith.constant 16 : index
    %c1 = arith.constant 1 : index
    scf.for %arg3 = %c0 to %c1024 step %c8 {
      scf.for %arg4 = %c0_0 to %c2000 step %c16 {
        scf.for %arg5 = %c0_1 to %c512 step %c1 {
          %subview = memref.subview %arg0[%arg3, %arg5] [8, 1] [1, 1] : memref<1024x512xf32> to memref<8x1xf32, strided<[512, 1], offset: ?>>
          %subview_2 = memref.subview %arg1[%arg5, %arg4] [1, 16] [1, 1] : memref<512x2000xf32> to memref<1x16xf32, strided<[2000, 1], offset: ?>>
          %subview_3 = memref.subview %arg2[%arg3, %arg4] [8, 16] [1, 1] : memref<1024x2000xf32> to memref<8x16xf32, strided<[2000, 1], offset: ?>>
          linalg.matmul
            ins(%subview, %subview_2 : memref<8x1xf32, strided<[512, 1], offset: ?>>, memref<1x16xf32, strided<[2000, 1], offset: ?>>)
            outs(%subview_3 : memref<8x16xf32, strided<[2000, 1], offset: ?>>)
        }
      }
    }
    return %arg2 : memref<1024x2000xf32>
  }
  module attributes {transform.with_named_sequence} {
    transform.named_sequence @__transform_main(%arg0: !transform.any_op {transform.readonly}) {
      %0 = transform.structured.match ops{["linalg.matmul"]} in %arg0 : (!transform.any_op) -> !transform.any_op
      %tiled_linalg_op, %loops:3 = transform.structured.tile_using_for %0 tile_sizes [8, 16, 1] : (!transform.any_op) -> (!transform.any_op, !transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)
      transform.yield
    }
  }
}

