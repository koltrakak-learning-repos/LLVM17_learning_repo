; ModuleID = 'Loop.c'
source_filename = "Loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @licm_loop(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
  store i32 %1, ptr %5, align 4
  store i32 %2, ptr %6, align 4
  store i32 0, ptr %7, align 4
  br label %12

12:                                               ; preds = %31, %3
  %13 = load i32, ptr %7, align 4
  %14 = icmp slt i32 %13, 10
  br i1 %14, label %15, label %34

15:                                               ; preds = %12
  store i32 3, ptr %8, align 4
  %16 = load i32, ptr %4, align 4
  %17 = load i32, ptr %8, align 4
  %18 = add nsw i32 %16, %17
  store i32 %18, ptr %9, align 4
  %19 = load i32, ptr %5, align 4
  %20 = icmp slt i32 %19, 10
  br i1 %20, label %21, label %24

21:                                               ; preds = %15
  %22 = load i32, ptr %5, align 4
  %23 = add nsw i32 %22, 4
  store i32 %23, ptr %5, align 4
  br label %24

24:                                               ; preds = %21, %15
  %25 = load i32, ptr %5, align 4
  %26 = load i32, ptr %6, align 4
  %27 = add nsw i32 %25, %26
  store i32 %27, ptr %10, align 4
  %28 = load i32, ptr %7, align 4
  %29 = load i32, ptr %5, align 4
  %30 = add nsw i32 %28, %29
  store i32 %30, ptr %11, align 4
  br label %31

31:                                               ; preds = %24
  %32 = load i32, ptr %7, align 4
  %33 = add nsw i32 %32, 1
  store i32 %33, ptr %7, align 4
  br label %12, !llvm.loop !6

34:                                               ; preds = %12
  %35 = load i32, ptr %5, align 4
  ret i32 %35
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 19.1.7 (/home/runner/work/llvm-project/llvm-project/clang cd708029e0b2869e80abe31ddb175f7c35361f90)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
