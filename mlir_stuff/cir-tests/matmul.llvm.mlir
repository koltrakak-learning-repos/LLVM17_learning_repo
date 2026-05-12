module @"/home/kevin/Git_learning_repos/mlir-tutor/cir-tests/matmul.cpp" attributes {cir.lang = #cir.lang<cxx>, cir.module_asm = [], cir.triple = "x86_64-unknown-linux-gnu", dlti.dl_spec = #dlti.dl_spec<!llvm.ptr<270> = dense<32> : vector<4xi64>, !llvm.ptr<271> = dense<32> : vector<4xi64>, !llvm.ptr<272> = dense<64> : vector<4xi64>, i64 = dense<64> : vector<2xi64>, i128 = dense<128> : vector<2xi64>, f80 = dense<128> : vector<2xi64>, !llvm.ptr = dense<64> : vector<4xi64>, i1 = dense<8> : vector<2xi64>, i8 = dense<8> : vector<2xi64>, i16 = dense<16> : vector<2xi64>, i32 = dense<32> : vector<2xi64>, f16 = dense<16> : vector<2xi64>, f64 = dense<64> : vector<2xi64>, f128 = dense<128> : vector<2xi64>, "dlti.endianness" = "little", "dlti.mangling_mode" = "e", "dlti.legal_int_widths" = array<i32: 8, 16, 32, 64>, "dlti.stack_alignment" = 128 : i64>, llvm.module_asm = [], llvm.target_triple = "x86_64-unknown-linux-gnu"} {
  llvm.mlir.global private constant @".str"() {addr_space = 0 : i32, alignment = 1 : i64, dso_local} : !llvm.array<4 x i8> {
    %0 = llvm.mlir.undef : !llvm.array<4 x i8>
    %1 = llvm.mlir.constant(37 : i8) : i8
    %2 = llvm.insertvalue %1, %0[0] : !llvm.array<4 x i8> 
    %3 = llvm.mlir.constant(100 : i8) : i8
    %4 = llvm.insertvalue %3, %2[1] : !llvm.array<4 x i8> 
    %5 = llvm.mlir.constant(32 : i8) : i8
    %6 = llvm.insertvalue %5, %4[2] : !llvm.array<4 x i8> 
    %7 = llvm.mlir.constant(0 : i8) : i8
    %8 = llvm.insertvalue %7, %6[3] : !llvm.array<4 x i8> 
    llvm.return %8 : !llvm.array<4 x i8>
  }
  llvm.func @_Z6matmulPKiS0_Piiiii(%arg0: !llvm.ptr, %arg1: !llvm.ptr, %arg2: !llvm.ptr, %arg3: i32, %arg4: i32, %arg5: i32, %arg6: i32) -> !llvm.ptr attributes {dso_local, no_inline} {
    %0 = llvm.mlir.constant(1 : i64) : i64
    %1 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %2 = llvm.mlir.constant(1 : i64) : i64
    %3 = llvm.alloca %2 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %4 = llvm.mlir.constant(1 : i64) : i64
    %5 = llvm.alloca %4 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %6 = llvm.mlir.zero : !llvm.ptr
    %7 = llvm.mlir.constant(0 : i32) : i32
    %8 = llvm.mlir.constant(1 : i64) : i64
    %9 = llvm.alloca %8 x !llvm.ptr {alignment = 8 : i64} : (i64) -> !llvm.ptr
    %10 = llvm.mlir.constant(1 : i64) : i64
    %11 = llvm.alloca %10 x !llvm.ptr {alignment = 8 : i64} : (i64) -> !llvm.ptr
    %12 = llvm.mlir.constant(1 : i64) : i64
    %13 = llvm.alloca %12 x !llvm.ptr {alignment = 8 : i64} : (i64) -> !llvm.ptr
    %14 = llvm.mlir.constant(1 : i64) : i64
    %15 = llvm.alloca %14 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %16 = llvm.mlir.constant(1 : i64) : i64
    %17 = llvm.alloca %16 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %18 = llvm.mlir.constant(1 : i64) : i64
    %19 = llvm.alloca %18 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %20 = llvm.mlir.constant(1 : i64) : i64
    %21 = llvm.alloca %20 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %22 = llvm.mlir.constant(1 : i64) : i64
    %23 = llvm.alloca %22 x !llvm.ptr {alignment = 8 : i64} : (i64) -> !llvm.ptr
    llvm.store %arg0, %9 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    llvm.store %arg1, %11 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    llvm.store %arg2, %13 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    llvm.store %arg3, %15 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.store %arg4, %17 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.store %arg5, %19 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.store %arg6, %21 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb1
  ^bb1:  // pred: ^bb0
    %24 = llvm.load %17 {alignment = 4 : i64} : !llvm.ptr -> i32
    %25 = llvm.load %19 {alignment = 4 : i64} : !llvm.ptr -> i32
    %26 = llvm.icmp "ne" %24, %25 : i32
    llvm.cond_br %26, ^bb2, ^bb3
  ^bb2:  // pred: ^bb1
    llvm.store %6, %23 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    %27 = llvm.load %23 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    llvm.return %27 : !llvm.ptr
  ^bb3:  // pred: ^bb1
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    llvm.br ^bb5
  ^bb5:  // pred: ^bb4
    llvm.store %7, %1 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb6
  ^bb6:  // 2 preds: ^bb5, ^bb26
    %28 = llvm.load %1 {alignment = 4 : i64} : !llvm.ptr -> i32
    %29 = llvm.load %15 {alignment = 4 : i64} : !llvm.ptr -> i32
    %30 = llvm.icmp "slt" %28, %29 : i32
    llvm.cond_br %30, ^bb7, ^bb27
  ^bb7:  // pred: ^bb6
    llvm.br ^bb8
  ^bb8:  // pred: ^bb7
    llvm.br ^bb9
  ^bb9:  // pred: ^bb8
    llvm.store %7, %3 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb10
  ^bb10:  // 2 preds: ^bb9, ^bb22
    %31 = llvm.load %3 {alignment = 4 : i64} : !llvm.ptr -> i32
    %32 = llvm.load %21 {alignment = 4 : i64} : !llvm.ptr -> i32
    %33 = llvm.icmp "slt" %31, %32 : i32
    llvm.cond_br %33, ^bb11, ^bb23
  ^bb11:  // pred: ^bb10
    llvm.br ^bb12
  ^bb12:  // pred: ^bb11
    %34 = llvm.load %1 {alignment = 4 : i64} : !llvm.ptr -> i32
    %35 = llvm.load %21 {alignment = 4 : i64} : !llvm.ptr -> i32
    %36 = llvm.mul %34, %35 overflow<nsw> : i32
    %37 = llvm.load %3 {alignment = 4 : i64} : !llvm.ptr -> i32
    %38 = llvm.add %36, %37 overflow<nsw> : i32
    %39 = llvm.load %13 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %40 = llvm.sext %38 : i32 to i64
    %41 = llvm.getelementptr %39[%40] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    llvm.store %7, %41 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb13
  ^bb13:  // pred: ^bb12
    llvm.store %7, %5 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb14
  ^bb14:  // 2 preds: ^bb13, ^bb18
    %42 = llvm.load %5 {alignment = 4 : i64} : !llvm.ptr -> i32
    %43 = llvm.load %17 {alignment = 4 : i64} : !llvm.ptr -> i32
    %44 = llvm.icmp "slt" %42, %43 : i32
    llvm.cond_br %44, ^bb15, ^bb19
  ^bb15:  // pred: ^bb14
    llvm.br ^bb16
  ^bb16:  // pred: ^bb15
    %45 = llvm.load %1 {alignment = 4 : i64} : !llvm.ptr -> i32
    %46 = llvm.load %17 {alignment = 4 : i64} : !llvm.ptr -> i32
    %47 = llvm.mul %45, %46 overflow<nsw> : i32
    %48 = llvm.load %5 {alignment = 4 : i64} : !llvm.ptr -> i32
    %49 = llvm.add %47, %48 overflow<nsw> : i32
    %50 = llvm.load %9 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %51 = llvm.sext %49 : i32 to i64
    %52 = llvm.getelementptr %50[%51] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %53 = llvm.load %52 {alignment = 4 : i64} : !llvm.ptr -> i32
    %54 = llvm.load %21 {alignment = 4 : i64} : !llvm.ptr -> i32
    %55 = llvm.mul %48, %54 overflow<nsw> : i32
    %56 = llvm.load %3 {alignment = 4 : i64} : !llvm.ptr -> i32
    %57 = llvm.add %55, %56 overflow<nsw> : i32
    %58 = llvm.load %11 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %59 = llvm.sext %57 : i32 to i64
    %60 = llvm.getelementptr %58[%59] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %61 = llvm.load %60 {alignment = 4 : i64} : !llvm.ptr -> i32
    %62 = llvm.mul %53, %61 overflow<nsw> : i32
    %63 = llvm.mul %45, %54 overflow<nsw> : i32
    %64 = llvm.add %63, %56 overflow<nsw> : i32
    %65 = llvm.load %13 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %66 = llvm.sext %64 : i32 to i64
    %67 = llvm.getelementptr %65[%66] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %68 = llvm.load %67 {alignment = 4 : i64} : !llvm.ptr -> i32
    %69 = llvm.add %68, %62 overflow<nsw> : i32
    llvm.store %69, %67 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb17
  ^bb17:  // pred: ^bb16
    llvm.br ^bb18
  ^bb18:  // pred: ^bb17
    %70 = llvm.load %5 {alignment = 4 : i64} : !llvm.ptr -> i32
    %71 = llvm.mlir.constant(1 : i32) : i32
    %72 = llvm.add %70, %71 overflow<nsw> : i32
    llvm.store %72, %5 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb14
  ^bb19:  // pred: ^bb14
    llvm.br ^bb20
  ^bb20:  // pred: ^bb19
    llvm.br ^bb21
  ^bb21:  // pred: ^bb20
    llvm.br ^bb22
  ^bb22:  // pred: ^bb21
    %73 = llvm.load %3 {alignment = 4 : i64} : !llvm.ptr -> i32
    %74 = llvm.mlir.constant(1 : i32) : i32
    %75 = llvm.add %73, %74 overflow<nsw> : i32
    llvm.store %75, %3 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb10
  ^bb23:  // pred: ^bb10
    llvm.br ^bb24
  ^bb24:  // pred: ^bb23
    llvm.br ^bb25
  ^bb25:  // pred: ^bb24
    llvm.br ^bb26
  ^bb26:  // pred: ^bb25
    %76 = llvm.load %1 {alignment = 4 : i64} : !llvm.ptr -> i32
    %77 = llvm.mlir.constant(1 : i32) : i32
    %78 = llvm.add %76, %77 overflow<nsw> : i32
    llvm.store %78, %1 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb6
  ^bb27:  // pred: ^bb6
    llvm.br ^bb28
  ^bb28:  // pred: ^bb27
    %79 = llvm.load %13 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    llvm.store %79, %23 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    %80 = llvm.load %23 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    llvm.return %80 : !llvm.ptr
  }
  llvm.func @printf(!llvm.ptr, ...) -> i32 attributes {sym_visibility = "private"}
  llvm.func @_Z12print_matrixPiii(%arg0: !llvm.ptr, %arg1: i32, %arg2: i32) attributes {dso_local, no_inline} {
    %0 = llvm.mlir.constant(1 : i64) : i64
    %1 = llvm.alloca %0 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %2 = llvm.mlir.constant(1 : i64) : i64
    %3 = llvm.alloca %2 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %4 = llvm.mlir.constant(0 : i32) : i32
    %5 = llvm.mlir.constant(1 : i64) : i64
    %6 = llvm.alloca %5 x !llvm.ptr {alignment = 8 : i64} : (i64) -> !llvm.ptr
    %7 = llvm.mlir.constant(1 : i64) : i64
    %8 = llvm.alloca %7 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    %9 = llvm.mlir.constant(1 : i64) : i64
    %10 = llvm.alloca %9 x i32 {alignment = 4 : i64} : (i64) -> !llvm.ptr
    llvm.store %arg0, %6 {alignment = 8 : i64} : !llvm.ptr, !llvm.ptr
    llvm.store %arg1, %8 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.store %arg2, %10 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb1
  ^bb1:  // pred: ^bb0
    llvm.store %4, %1 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb2
  ^bb2:  // 2 preds: ^bb1, ^bb14
    %11 = llvm.load %1 {alignment = 4 : i64} : !llvm.ptr -> i32
    %12 = llvm.load %8 {alignment = 4 : i64} : !llvm.ptr -> i32
    %13 = llvm.icmp "slt" %11, %12 : i32
    llvm.cond_br %13, ^bb3, ^bb15
  ^bb3:  // pred: ^bb2
    llvm.br ^bb4
  ^bb4:  // pred: ^bb3
    llvm.br ^bb5
  ^bb5:  // pred: ^bb4
    llvm.store %4, %3 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb6
  ^bb6:  // 2 preds: ^bb5, ^bb10
    %14 = llvm.load %3 {alignment = 4 : i64} : !llvm.ptr -> i32
    %15 = llvm.load %10 {alignment = 4 : i64} : !llvm.ptr -> i32
    %16 = llvm.icmp "slt" %14, %15 : i32
    llvm.cond_br %16, ^bb7, ^bb11
  ^bb7:  // pred: ^bb6
    llvm.br ^bb8
  ^bb8:  // pred: ^bb7
    %17 = llvm.mlir.addressof @".str" : !llvm.ptr
    %18 = llvm.getelementptr %17[0] : (!llvm.ptr) -> !llvm.ptr, i8
    %19 = llvm.load %1 {alignment = 4 : i64} : !llvm.ptr -> i32
    %20 = llvm.load %10 {alignment = 4 : i64} : !llvm.ptr -> i32
    %21 = llvm.mul %19, %20 overflow<nsw> : i32
    %22 = llvm.load %3 {alignment = 4 : i64} : !llvm.ptr -> i32
    %23 = llvm.add %21, %22 overflow<nsw> : i32
    %24 = llvm.load %6 {alignment = 8 : i64} : !llvm.ptr -> !llvm.ptr
    %25 = llvm.sext %23 : i32 to i64
    %26 = llvm.getelementptr %24[%25] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %27 = llvm.load %26 {alignment = 4 : i64} : !llvm.ptr -> i32
    %28 = llvm.call @printf(%18, %27) vararg(!llvm.func<i32 (ptr, ...)>) : (!llvm.ptr, i32) -> i32
    llvm.br ^bb9
  ^bb9:  // pred: ^bb8
    llvm.br ^bb10
  ^bb10:  // pred: ^bb9
    %29 = llvm.load %3 {alignment = 4 : i64} : !llvm.ptr -> i32
    %30 = llvm.mlir.constant(1 : i32) : i32
    %31 = llvm.add %29, %30 overflow<nsw> : i32
    llvm.store %31, %3 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb6
  ^bb11:  // pred: ^bb6
    llvm.br ^bb12
  ^bb12:  // pred: ^bb11
    llvm.br ^bb13
  ^bb13:  // pred: ^bb12
    llvm.br ^bb14
  ^bb14:  // pred: ^bb13
    %32 = llvm.load %1 {alignment = 4 : i64} : !llvm.ptr -> i32
    %33 = llvm.mlir.constant(1 : i32) : i32
    %34 = llvm.add %32, %33 overflow<nsw> : i32
    llvm.store %34, %1 {alignment = 4 : i64} : i32, !llvm.ptr
    llvm.br ^bb2
  ^bb15:  // pred: ^bb2
    llvm.br ^bb16
  ^bb16:  // pred: ^bb15
    llvm.return
  }
  llvm.func @main() -> i32 attributes {dso_local, no_inline} {
    %0 = llvm.mlir.constant(0 : i32) : i32
    %1 = llvm.mlir.constant(3 : i32) : i32
    %2 = llvm.mlir.constant(2 : i32) : i32
    %3 = llvm.mlir.zero : !llvm.array<2 x array<2 x i32>>
    %4 = llvm.mlir.constant(dense<[[1, 2], [3, 4], [5, 6]]> : tensor<3x2xi32>) : !llvm.array<3 x array<2 x i32>>
    %5 = llvm.mlir.constant(dense<[[1, 2, 3], [4, 5, 6]]> : tensor<2x3xi32>) : !llvm.array<2 x array<3 x i32>>
    %6 = llvm.mlir.constant(1 : i64) : i64
    %7 = llvm.alloca %6 x !llvm.array<2 x array<3 x i32>> {alignment = 16 : i64} : (i64) -> !llvm.ptr
    %8 = llvm.mlir.constant(1 : i64) : i64
    %9 = llvm.alloca %8 x !llvm.array<3 x array<2 x i32>> {alignment = 16 : i64} : (i64) -> !llvm.ptr
    %10 = llvm.mlir.constant(1 : i64) : i64
    %11 = llvm.alloca %10 x !llvm.array<2 x array<2 x i32>> {alignment = 16 : i64} : (i64) -> !llvm.ptr
    llvm.store %5, %7 {alignment = 16 : i64} : !llvm.array<2 x array<3 x i32>>, !llvm.ptr
    llvm.store %4, %9 {alignment = 16 : i64} : !llvm.array<3 x array<2 x i32>>, !llvm.ptr
    llvm.store %3, %11 {alignment = 16 : i64} : !llvm.array<2 x array<2 x i32>>, !llvm.ptr
    %12 = llvm.getelementptr %7[0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<3 x i32>
    %13 = llvm.bitcast %12 : !llvm.ptr to !llvm.ptr
    %14 = llvm.getelementptr %9[0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<2 x i32>
    %15 = llvm.bitcast %14 : !llvm.ptr to !llvm.ptr
    %16 = llvm.getelementptr %11[0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<2 x i32>
    %17 = llvm.bitcast %16 : !llvm.ptr to !llvm.ptr
    %18 = llvm.call @_Z6matmulPKiS0_Piiiii(%13, %15, %17, %2, %1, %1, %2) : (!llvm.ptr, !llvm.ptr, !llvm.ptr, i32, i32, i32, i32) -> !llvm.ptr
    llvm.call @_Z12print_matrixPiii(%17, %2, %2) : (!llvm.ptr, i32, i32) -> ()
    llvm.return %0 : i32
  }
}

