; ModuleID = 'Loop.c'
source_filename = "Loop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @g_incr(i32 noundef %0) local_unnamed_addr #0 {
  %2 = load i32, ptr @g, align 4, !tbaa !5
  %3 = add nsw i32 %2, %0
  store i32 %3, ptr @g, align 4, !tbaa !5
  ret i32 %3
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @loop(i32 noundef %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #0 {
  %4 = load i32, ptr @g, align 4, !tbaa !5          ; %4 = g
  %5 = icmp slt i32 %0, %1                          ; %5 = a < b
  br i1 %5, label %6, label %10                     ; se si salta al loop body, se no al post-loop

6:                                                ; preds = %3
  %7 = sub i32 %1, %0                               ; %7 = b - a = numero di iterazioni
  %8 = mul i32 %7, %2                               ; %8 = (b-a) * c = incremento totale di g
  %9 = add i32 %4, %8                               ; %9 = g + incremento totale
  store i32 %9, ptr @g, align 4, !tbaa !5           ; aggiorno g
  br label %10

10:                                               ; preds = %6, %3
  %11 = phi i32 [ %9, %6 ], [ %4, %3 ]              ; %11 = g con incremento se sono entrato nel loop body, altrimenti valore di g
  ret i32 %11
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, argmem: none, inaccessiblemem: none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 19.1.7 (/home/runner/work/llvm-project/llvm-project/clang cd708029e0b2869e80abe31ddb175f7c35361f90)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
