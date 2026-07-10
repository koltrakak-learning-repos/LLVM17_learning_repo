#map = affine_map<(d0, d1) -> (d0, 0, d1)>
#map1 = affine_map<(d0, d1) -> (0, d1, d0)>
module {
  func.func @matmul(%A: memref<1024x512xf32>, %B: memref<512x2000xf32>, %C: memref<1024x2000xf32>) -> memref<1024x2000xf32> {
    %c0 = arith.constant 0 : index
    %c0_0 = arith.constant 0 : index
    %c0_1 = arith.constant 0 : index
    %c1024 = arith.constant 1024 : index
    %c2000 = arith.constant 2000 : index
    %c512 = arith.constant 512 : index
    %c8 = arith.constant 8 : index
    %c16 = arith.constant 16 : index
    %c1 = arith.constant 1 : index
    scf.for %i = %c0 to %c1024 step %c8 {
      scf.for %j = %c0_0 to %c2000 step %c16 {
        scf.for %k = %c0_1 to %c512 step %c1 {
          %A_tile = memref.subview %A[%i, %k] [8, 1] [1, 1] : memref<1024x512xf32> to memref<8x1xf32, strided<[512, 1], offset: ?>>
          %B_tile = memref.subview %B[%k, %j] [1, 16] [1, 1] : memref<512x2000xf32> to memref<1x16xf32, strided<[2000, 1], offset: ?>>
          %C_tile = memref.subview %C[%i, %j] [8, 16] [1, 1] : memref<1024x2000xf32> to memref<8x16xf32, strided<[2000, 1], offset: ?>>

          %c8_4 = arith.constant 8 : index
          %c16_5 = arith.constant 16 : index
          %c1_6 = arith.constant 1 : index
          %c0_7 = arith.constant 0 : index

          %0 = ub.poison : f32
          // NB: La permutation map è il meccanismo che specifica "quando leggi
          // questo memref e lo metti in un vettore di rank più alto, quali
          // assi corrispondono a dati reali e quali sono broadcasted".
          %1 = vector.transfer_read %A_tile[%c0_7, %c0_7], %0 {permutation_map = #map} : memref<8x1xf32, strided<[512, 1], offset: ?>>, vector<8x16x1xf32>
          %2 = ub.poison : f32
          %3 = vector.transfer_read %B_tile[%c0_7, %c0_7], %2 {permutation_map = #map1} : memref<1x16xf32, strided<[2000, 1], offset: ?>>, vector<8x16x1xf32>
          %4 = ub.poison : f32
          %5 = vector.transfer_read %C_tile[%c0_7, %c0_7], %4 : memref<8x16xf32, strided<[2000, 1], offset: ?>>, vector<8x16xf32>
          %6 = arith.mulf %1, %3 : vector<8x16x1xf32>

          %7 = vector.multi_reduction <add>, %6, %5 [2] : vector<8x16x1xf32> to vector<8x16xf32>

          %c0_8 = arith.constant 0 : index
          vector.transfer_write %7, %C_tile[%c0_8, %c0_8] : vector<8x16xf32>, memref<8x16xf32, strided<[2000, 1], offset: ?>>
        }
      }
    }
    return %C : memref<1024x2000xf32>
  }
  module attributes {transform.with_named_sequence} {
    transform.named_sequence @__transform_main(%arg0: !transform.any_op {transform.readonly}) {
      %0 = transform.structured.match ops{["linalg.matmul"]} in %arg0 : (!transform.any_op) -> !transform.any_op
      %tiled_linalg_op, %loops:3 = transform.structured.tile_using_for %0 tile_sizes [8, 16, 1] : (!transform.any_op) -> (!transform.any_op, !transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)
      transform.structured.vectorize %tiled_linalg_op vector_sizes [8, 16, 1] : !transform.any_op
      transform.yield
    }
  }
}

