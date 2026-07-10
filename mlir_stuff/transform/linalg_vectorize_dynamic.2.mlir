#map = affine_map<(d0)[s0] -> (-d0 + s0, 16)>
#map1 = affine_map<(d0) -> (d0 - 1)>
#map2 = affine_map<(d0, d1) -> (d0, 0, d1)>
#map3 = affine_map<(d0, d1) -> (0, d1, d0)>
module {
  // manca un po' di canonicallizzazione però ormai ho già rinominato i valori
  func.func @matmul(%A: memref<?x?xf32>, %B: memref<?x?xf32>, %C: memref<?x?xf32>) -> memref<?x?xf32> {
    %c0 = arith.constant 0 : index
    %M = memref.dim %A, %c0 : memref<?x?xf32>
    %c1 = arith.constant 1 : index
    %K = memref.dim %A, %c1 : memref<?x?xf32>
    %c0_1 = arith.constant 0 : index
    %dim_2 = memref.dim %B, %c0_1 : memref<?x?xf32>
    %c1_3 = arith.constant 1 : index
    %N = memref.dim %B, %c1_3 : memref<?x?xf32>
    %c0_5 = arith.constant 0 : index
    %dim_6 = memref.dim %C, %c0_5 : memref<?x?xf32>
    %c1_7 = arith.constant 1 : index
    %dim_8 = memref.dim %C, %c1_7 : memref<?x?xf32>

    %c0_9 = arith.constant 0 : index
    %c0_10 = arith.constant 0 : index
    %c0_11 = arith.constant 0 : index
    %c16 = arith.constant 16 : index
    %c16_12 = arith.constant 16 : index
    %c1_13 = arith.constant 1 : index
    scf.for %i = %c0_9 to %M step %c16 {
      scf.for %j = %c0_10 to %N step %c16_12 {
        scf.for %k = %c0_11 to %K step %c1_13 {
          %0 = affine.min #map(%i)[%M] // minimo tra iterazioni rimanenti e 16
          %1 = affine.min #map(%j)[%N] // minimo tra iterazioni rimanenti e 16
        //   %2 = affine.apply #map1(%0)
        //   %3 = affine.apply #map1(%1)
        //   %4 = affine.apply #map1(%0)
        //   %5 = affine.apply #map1(%1)
        //   %6 = affine.apply #map1(%0)
        //   %7 = affine.apply #map1(%1)

          %tile_A = memref.subview %A[%i, %k] [%0, 1] [1, 1] : memref<?x?xf32> to memref<?x1xf32, strided<[?, 1], offset: ?>>
          %tile_B = memref.subview %B[%k, %j] [1, %1] [1, 1] : memref<?x?xf32> to memref<1x?xf32, strided<[?, 1], offset: ?>>
          %tile_C = memref.subview %C[%i, %j] [%0, %1] [1, 1] : memref<?x?xf32> to memref<?x?xf32, strided<[?, 1], offset: ?>>

          %c0_16 = arith.constant 0 : index
          %M_tile_A = memref.dim %tile_A, %c0_16 : memref<?x1xf32, strided<[?, 1], offset: ?>>
          %c1_18 = arith.constant 1 : index
          %N_tile_B = memref.dim %tile_B, %c1_18 : memref<1x?xf32, strided<[?, 1], offset: ?>>

          %c1_20 = arith.constant 1 : index
          %c0_21 = arith.constant 0 : index

          %8 = ub.poison : f32
          %9 = vector.create_mask %M_tile_A, %c1_20 : vector<16x1xi1>
          %10 = vector.mask %9 { vector.transfer_read %tile_A[%c0_21, %c0_21], %8 {in_bounds = [true, true, true], permutation_map = #map2} : memref<?x1xf32, strided<[?, 1], offset: ?>>, vector<16x16x1xf32> } : vector<16x1xi1> -> vector<16x16x1xf32>

          %11 = ub.poison : f32
          %12 = vector.create_mask %c1_20, %N_tile_B : vector<1x16xi1>
          %13 = vector.mask %12 { vector.transfer_read %tile_B[%c0_21, %c0_21], %11 {in_bounds = [true, true, true], permutation_map = #map3} : memref<1x?xf32, strided<[?, 1], offset: ?>>, vector<16x16x1xf32> } : vector<1x16xi1> -> vector<16x16x1xf32>

          %14 = ub.poison : f32
          %15 = vector.create_mask %M_tile_A, %N_tile_B : vector<16x16xi1>
          %16 = vector.mask %15 { vector.transfer_read %tile_C[%c0_21, %c0_21], %14 {in_bounds = [true, true]} : memref<?x?xf32, strided<[?, 1], offset: ?>>, vector<16x16xf32> } : vector<16x16xi1> -> vector<16x16xf32>

          %17 = arith.mulf %10, %13 : vector<16x16x1xf32>

          %18 = vector.create_mask %M_tile_A, %N_tile_B, %c1_20 : vector<16x16x1xi1>
          %19 = vector.mask %18 { vector.multi_reduction <add>, %17, %16 [2] : vector<16x16x1xf32> to vector<16x16xf32> } : vector<16x16x1xi1> -> vector<16x16xf32>

          %c0_22 = arith.constant 0 : index
          vector.mask %15 { vector.transfer_write %19, %tile_C[%c0_22, %c0_22] {in_bounds = [true, true]} : vector<16x16xf32>, memref<?x?xf32, strided<[?, 1], offset: ?>> } : vector<16x16xi1>
        }
      }
    }
    return %C : memref<?x?xf32>
  }
  module attributes {transform.with_named_sequence} {
    transform.named_sequence @__transform_main(%arg0: !transform.any_op {transform.readonly}) {
      %0 = transform.structured.match ops{["linalg.matmul"]} in %arg0 : (!transform.any_op) -> !transform.any_op
      %tiled_linalg_op, %loops:3 = transform.structured.tile_using_for %0 tile_sizes [16, 16, 1] : (!transform.any_op) -> (!transform.any_op, !transform.op<"scf.for">, !transform.op<"scf.for">, !transform.op<"scf.for">)
      transform.structured.vectorize %tiled_linalg_op vector_sizes [16, 16, 1] : !transform.any_op
      transform.yield
    }
  }
}

