#map = affine_map<(d0, d1, d2) -> (d0, d2)>
#map1 = affine_map<(d0, d1, d2) -> (d2, d1)>
#map2 = affine_map<(d0, d1, d2) -> (d0, d1)>
#map3 = affine_map<(d0, d1) -> (d0, d1)>

module {
  func.func @main() -> tensor<256x1024xf32> {
    %cst = arith.constant dense<0.000000e+00> : tensor<256x1024xf32>
    %cst_0 = arith.constant 0.000000e+00 : f32
    %0 = tensor.empty() : tensor<256x512xf32>
    %1 = tensor.empty() : tensor<512x1024xf32>

    %2 = linalg.generic {
        indexing_maps = [#map, #map1, #map2],
        iterator_types = ["parallel", "parallel", "reduction"]
    } ins(%0, %1 : tensor<256x512xf32>, tensor<512x1024xf32>) outs(%cst : tensor<256x1024xf32>) {
    ^bb0(%in: f32, %in_1: f32, %out: f32):
      %5 = arith.mulf %in, %in_1 : f32
      %6 = arith.addf %out, %5 : f32
      linalg.yield %6 : f32
    } -> tensor<256x1024xf32>

    %3 = tensor.empty() : tensor<256x1024xf32>

    %4 = linalg.generic {
        indexing_maps = [#map3, #map3],
        iterator_types = ["parallel", "parallel"]
    } ins(%2 : tensor<256x1024xf32>) outs(%3 : tensor<256x1024xf32>) {
    ^bb0(%in: f32, %out: f32):
      %5 = arith.cmpf ugt, %in, %cst_0 : f32
      %6 = arith.select %5, %in, %cst_0 : f32
      linalg.yield %6 : f32
    } -> tensor<256x1024xf32>

    return %4 : tensor<256x1024xf32>
  }
}

