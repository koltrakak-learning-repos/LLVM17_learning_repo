; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"
target datalayout = "e-m:e-n8:16:32:64-S128-p270:32:32:32:32-p271:32:32:32:32-p272:64:64:64:64-i64:64-i128:128-f80:128-p0:64:64:64:64-i1:8-i8:8-i16:16-i32:32-f16:16-f64:64-f128:128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private constant [4 x i8] c"%d \00", align 1

; Function Attrs: noinline
define dso_local ptr @_Z6matmulPKiS0_Piiiii(ptr %0, ptr %1, ptr %2, i32 %3, i32 %4, i32 %5, i32 %6) #0 {
  %8 = alloca i32, i64 1, align 4
  %9 = alloca i32, i64 1, align 4
  %10 = alloca i32, i64 1, align 4
  %11 = alloca ptr, i64 1, align 8
  %12 = alloca ptr, i64 1, align 8
  %13 = alloca ptr, i64 1, align 8
  %14 = alloca i32, i64 1, align 4
  %15 = alloca i32, i64 1, align 4
  %16 = alloca i32, i64 1, align 4
  %17 = alloca i32, i64 1, align 4
  %18 = alloca ptr, i64 1, align 8
  store ptr %0, ptr %11, align 8
  store ptr %1, ptr %12, align 8
  store ptr %2, ptr %13, align 8
  store i32 %3, ptr %14, align 4
  store i32 %4, ptr %15, align 4
  store i32 %5, ptr %16, align 4
  store i32 %6, ptr %17, align 4
  br label %19

19:                                               ; preds = %7
  %20 = load i32, ptr %15, align 4
  %21 = load i32, ptr %16, align 4
  %22 = icmp ne i32 %20, %21
  br i1 %22, label %23, label %25

23:                                               ; preds = %19
  store ptr null, ptr %18, align 8
  %24 = load ptr, ptr %18, align 8
  ret ptr %24

25:                                               ; preds = %19
  br label %26

26:                                               ; preds = %25
  br label %27

27:                                               ; preds = %26
  store i32 0, ptr %8, align 4
  br label %28

28:                                               ; preds = %94, %27
  %29 = load i32, ptr %8, align 4
  %30 = load i32, ptr %14, align 4
  %31 = icmp slt i32 %29, %30
  br i1 %31, label %32, label %97

32:                                               ; preds = %28
  br label %33

33:                                               ; preds = %32
  br label %34

34:                                               ; preds = %33
  store i32 0, ptr %9, align 4
  br label %35

35:                                               ; preds = %88, %34
  %36 = load i32, ptr %9, align 4
  %37 = load i32, ptr %17, align 4
  %38 = icmp slt i32 %36, %37
  br i1 %38, label %39, label %91

39:                                               ; preds = %35
  br label %40

40:                                               ; preds = %39
  %41 = load i32, ptr %8, align 4
  %42 = load i32, ptr %17, align 4
  %43 = mul nsw i32 %41, %42
  %44 = load i32, ptr %9, align 4
  %45 = add nsw i32 %43, %44
  %46 = load ptr, ptr %13, align 8
  %47 = sext i32 %45 to i64
  %48 = getelementptr i32, ptr %46, i64 %47
  store i32 0, ptr %48, align 4
  br label %49

49:                                               ; preds = %40
  store i32 0, ptr %10, align 4
  br label %50

50:                                               ; preds = %82, %49
  %51 = load i32, ptr %10, align 4
  %52 = load i32, ptr %15, align 4
  %53 = icmp slt i32 %51, %52
  br i1 %53, label %54, label %85

54:                                               ; preds = %50
  br label %55

55:                                               ; preds = %54
  %56 = load i32, ptr %8, align 4
  %57 = load i32, ptr %15, align 4
  %58 = mul nsw i32 %56, %57
  %59 = load i32, ptr %10, align 4
  %60 = add nsw i32 %58, %59
  %61 = load ptr, ptr %11, align 8
  %62 = sext i32 %60 to i64
  %63 = getelementptr i32, ptr %61, i64 %62
  %64 = load i32, ptr %63, align 4
  %65 = load i32, ptr %17, align 4
  %66 = mul nsw i32 %59, %65
  %67 = load i32, ptr %9, align 4
  %68 = add nsw i32 %66, %67
  %69 = load ptr, ptr %12, align 8
  %70 = sext i32 %68 to i64
  %71 = getelementptr i32, ptr %69, i64 %70
  %72 = load i32, ptr %71, align 4
  %73 = mul nsw i32 %64, %72
  %74 = mul nsw i32 %56, %65
  %75 = add nsw i32 %74, %67
  %76 = load ptr, ptr %13, align 8
  %77 = sext i32 %75 to i64
  %78 = getelementptr i32, ptr %76, i64 %77
  %79 = load i32, ptr %78, align 4
  %80 = add nsw i32 %79, %73
  store i32 %80, ptr %78, align 4
  br label %81

81:                                               ; preds = %55
  br label %82

82:                                               ; preds = %81
  %83 = load i32, ptr %10, align 4
  %84 = add nsw i32 %83, 1
  store i32 %84, ptr %10, align 4
  br label %50

85:                                               ; preds = %50
  br label %86

86:                                               ; preds = %85
  br label %87

87:                                               ; preds = %86
  br label %88

88:                                               ; preds = %87
  %89 = load i32, ptr %9, align 4
  %90 = add nsw i32 %89, 1
  store i32 %90, ptr %9, align 4
  br label %35

91:                                               ; preds = %35
  br label %92

92:                                               ; preds = %91
  br label %93

93:                                               ; preds = %92
  br label %94

94:                                               ; preds = %93
  %95 = load i32, ptr %8, align 4
  %96 = add nsw i32 %95, 1
  store i32 %96, ptr %8, align 4
  br label %28

97:                                               ; preds = %28
  br label %98

98:                                               ; preds = %97
  %99 = load ptr, ptr %13, align 8
  store ptr %99, ptr %18, align 8
  %100 = load ptr, ptr %18, align 8
  ret ptr %100
}

declare i32 @printf(ptr, ...)

; Function Attrs: noinline
define dso_local void @_Z12print_matrixPiii(ptr %0, i32 %1, i32 %2) #0 {
  %4 = alloca i32, i64 1, align 4
  %5 = alloca i32, i64 1, align 4
  %6 = alloca ptr, i64 1, align 8
  %7 = alloca i32, i64 1, align 4
  %8 = alloca i32, i64 1, align 4
  store ptr %0, ptr %6, align 8
  store i32 %1, ptr %7, align 4
  store i32 %2, ptr %8, align 4
  br label %9

9:                                                ; preds = %3
  store i32 0, ptr %4, align 4
  br label %10

10:                                               ; preds = %40, %9
  %11 = load i32, ptr %4, align 4
  %12 = load i32, ptr %7, align 4
  %13 = icmp slt i32 %11, %12
  br i1 %13, label %14, label %43

14:                                               ; preds = %10
  br label %15

15:                                               ; preds = %14
  br label %16

16:                                               ; preds = %15
  store i32 0, ptr %5, align 4
  br label %17

17:                                               ; preds = %34, %16
  %18 = load i32, ptr %5, align 4
  %19 = load i32, ptr %8, align 4
  %20 = icmp slt i32 %18, %19
  br i1 %20, label %21, label %37

21:                                               ; preds = %17
  br label %22

22:                                               ; preds = %21
  %23 = load i32, ptr %4, align 4
  %24 = load i32, ptr %8, align 4
  %25 = mul nsw i32 %23, %24
  %26 = load i32, ptr %5, align 4
  %27 = add nsw i32 %25, %26
  %28 = load ptr, ptr %6, align 8
  %29 = sext i32 %27 to i64
  %30 = getelementptr i32, ptr %28, i64 %29
  %31 = load i32, ptr %30, align 4
  %32 = call i32 (ptr, ...) @printf(ptr @.str, i32 %31)
  br label %33

33:                                               ; preds = %22
  br label %34

34:                                               ; preds = %33
  %35 = load i32, ptr %5, align 4
  %36 = add nsw i32 %35, 1
  store i32 %36, ptr %5, align 4
  br label %17

37:                                               ; preds = %17
  br label %38

38:                                               ; preds = %37
  br label %39

39:                                               ; preds = %38
  br label %40

40:                                               ; preds = %39
  %41 = load i32, ptr %4, align 4
  %42 = add nsw i32 %41, 1
  store i32 %42, ptr %4, align 4
  br label %10

43:                                               ; preds = %10
  br label %44

44:                                               ; preds = %43
  ret void
}

; Function Attrs: noinline
define dso_local i32 @main() #0 {
  %1 = alloca [2 x [3 x i32]], i64 1, align 16
  %2 = alloca [3 x [2 x i32]], i64 1, align 16
  %3 = alloca [2 x [2 x i32]], i64 1, align 16
  store [2 x [3 x i32]] [[3 x i32] [i32 1, i32 2, i32 3], [3 x i32] [i32 4, i32 5, i32 6]], ptr %1, align 16
  store [3 x [2 x i32]] [[2 x i32] [i32 1, i32 2], [2 x i32] [i32 3, i32 4], [2 x i32] [i32 5, i32 6]], ptr %2, align 16
  store [2 x [2 x i32]] zeroinitializer, ptr %3, align 16
  %4 = getelementptr [3 x i32], ptr %1, i32 0
  %5 = getelementptr [2 x i32], ptr %2, i32 0
  %6 = getelementptr [2 x i32], ptr %3, i32 0
  %7 = call ptr @_Z6matmulPKiS0_Piiiii(ptr %4, ptr %5, ptr %6, i32 2, i32 3, i32 3, i32 2)
  call void @_Z12print_matrixPiii(ptr %6, i32 2, i32 2)
  ret i32 0
}

attributes #0 = { noinline }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
