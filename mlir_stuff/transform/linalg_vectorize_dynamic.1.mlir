#map = affine_map<(d0, d1) -> (d0, 0, d1)>
#map1 = affine_map<(d0, d1) -> (0, d1, d0)>
module {
  func.func @matmul(%arg0: memref<?x?xf32>, %arg1: memref<?x?xf32>, %arg2: memref<?x?xf32>) -> memref<?x?xf32> {
    %c0 = arith.constant 0 : index
    %M = memref.dim %arg0, %c0 : memref<?x?xf32>
    %c1 = arith.constant 1 : index
    %N = memref.dim %arg1, %c1 : memref<?x?xf32>
    %c1_1 = arith.constant 1 : index
    %K = memref.dim %arg0, %c1_1 : memref<?x?xf32>

    // NB: senza tiling, qua non c'è nessun loop che mi copre tutto il
    // data-space. STO PROCESSANDO SOLAMENTE UN SINGOLO TILE!

    %c0_3 = arith.constant 0 : index
    %0 = ub.poison : f32
    // NB: creiamo una maschera grande 16x1 dove le posizioni con coordinate
    // corrispondenti < M e < K sono settate a 1
    %1 = vector.create_mask %M, %K : vector<16x1xi1>
    // NB: the mask argument holds a bit for each vector lane and determines
    // which vector lanes should execute the maskable operation and which ones
    // should not. The vector.mask operation returns the value produced by the
    // masked execution of the nested operation, if any.
    %2 = vector.mask %1 { vector.transfer_read %arg0[%c0_3, %c0_3], %0 {in_bounds = [true, true, true], permutation_map = #map} : memref<?x?xf32>, vector<16x16x1xf32> } : vector<16x1xi1> -> vector<16x16x1xf32>

    %3 = ub.poison : f32
    %4 = vector.create_mask %K, %N : vector<1x16xi1>
    %5 = vector.mask %4 { vector.transfer_read %arg1[%c0_3, %c0_3], %3 {in_bounds = [true, true, true], permutation_map = #map1} : memref<?x?xf32>, vector<16x16x1xf32> } : vector<1x16xi1> -> vector<16x16x1xf32>

    %6 = ub.poison : f32
    %7 = vector.create_mask %M, %N : vector<16x16xi1>
    %8 = vector.mask %7 { vector.transfer_read %arg2[%c0_3, %c0_3], %6 {in_bounds = [true, true]} : memref<?x?xf32>, vector<16x16xf32> } : vector<16x16xi1> -> vector<16x16xf32>

    %9 = arith.mulf %2, %5 : vector<16x16x1xf32>

    %10 = vector.create_mask %M, %N, %K : vector<16x16x1xi1>
    %11 = vector.mask %10 { vector.multi_reduction <add>, %9, %8 [2] : vector<16x16x1xf32> to vector<16x16xf32> } : vector<16x16x1xi1> -> vector<16x16xf32>
    %c0_4 = arith.constant 0 : index
    vector.mask %7 { vector.transfer_write %11, %arg2[%c0_4, %c0_4] {in_bounds = [true, true]} : vector<16x16xf32>, memref<?x?xf32> } : vector<16x16xi1>

    return %arg2 : memref<?x?xf32>
  }

  module attributes {transform.with_named_sequence} {
    transform.named_sequence @__transform_main(%arg0: !transform.any_op {transform.readonly}) {
      %0 = transform.structured.match ops{["linalg.matmul"]} in %arg0 : (!transform.any_op) -> !transform.any_op
      transform.structured.vectorize %0 vector_sizes [16, 16, 1] : !transform.any_op
      transform.yield
    }
  }
}

