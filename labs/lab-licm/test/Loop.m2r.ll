; ModuleID = 'Loop.ll'
source_filename = "Loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @licm_loop(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
preheader:
  br label %header

header:                                                ; preds = %latch, %preheader
  %.01 = phi i32 [ %1, %preheader], [ %.1, %latch ] ; b
  %.0 = phi i32 [ 0, %preheader ], [ %15, %latch ]  ; i
  %5 = icmp slt i32 %.0, 10                         ; i<10
  br i1 %5, label %6, label %exit

6:                                                ; preds = %header
  %7 = add nsw i32 %0, 3                            ; d=3, e=a+d -> L.I.
  %8 = icmp slt i32 %.01, 10                        ; b<10
  br i1 %8, label %9, label %11

9:                                                ; preds = %6
  %10 = add nsw i32 %.01, 4                         ; b+=4
  br label %11

11:                                               ; preds = %9, %6
  %.1 = phi i32 [ %10, %9 ], [ %.01, %6 ]           ; b
  %12 = add nsw i32 %7, %2                          ; f=e+c -> L.I.
  %13 = add nsw i32 %.0, %.1                        ; g=i+b
  br label %latch

latch:                                               ; preds = %11
  %15 = add nsw i32 %.0, 1                          ; i++
  br label %header, !llvm.loop !6

exit:                                               ; preds = %header
  ret i32 %.01                                      ; ret b
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
