; ModuleID = 'Fibonacci.c'
source_filename = "Fibonacci.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.__va_list_tag = type { i32, i32, ptr, ptr }

@stdout = external local_unnamed_addr global ptr, align 8                               ; ha tipo FILE*
@.str = private unnamed_addr constant [9 x i8] c"f(0) = 0\00", align 1
@.str.1 = private unnamed_addr constant [9 x i8] c"f(1) = 1\00", align 1
@.str.2 = private unnamed_addr constant [22 x i8] c"f(%d) = f(%d) + f(%d)\00", align 1

; Function Attrs: nofree nounwind uwtable
define dso_local noundef i32 @printf(ptr nocapture noundef readonly %0, ...) local_unnamed_addr #0 {
  %2 = alloca [1 x %struct.__va_list_tag], align 16                                     ; alloca sullo stack va_list
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %2) #4                          ; chiamata a costrutture di va_list?
  call void @llvm.va_start.p0(ptr nonnull %2)                                           ; chiamata a va_start
  %3 = load ptr, ptr @stdout, align 8, !tbaa !5                                         ; carico stdout: carica un ptr (FILE*) da stdout
  %4 = call i32 @vfprintf(ptr noundef %3, ptr noundef %0, ptr noundef nonnull %2) #4    ; chiama vprintf
  call void @llvm.va_end.p0(ptr nonnull %2)                                             ; chiama va_end
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %2) #4                            ; chiamata al distruttore di va_list
  ret i32 %4
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start.p0(ptr) #2

; Function Attrs: nofree nounwind
declare noundef i32 @vfprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ptr noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end.p0(ptr) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @Fibonacci(i32 noundef %0) local_unnamed_addr #0 {
  br label %2                                       ; istruzione spuria? no, serve per il phi-node

2:                                                ; preds = %5, %1
  %3 = phi i32 [ 0, %1 ], [ %10, %5 ]               ; %3 = accumulatore = 0 all'inizio, quanto accumulato se vengo da %5
  %4 = phi i32 [ %0, %1 ], [ %7, %5 ]               ; %4 = n all'inizio, %7 = n-2, se vengo da %5
  switch i32 %4, label %5 [                         ; switch on %4, if no match goto %5 by deafault which computes
    i32 0, label %12                                ; %4 == 0 -> caso base per n=0
    i32 1, label %11                                ; %4 == 1 -> caso base per n=1
  ]

5:                                                ; preds = %2
  %6 = add nsw i32 %4, -1                           ; n-1
  %7 = add nsw i32 %4, -2                           ; n-2
  ; printf("f(%d) = ...", n, n-1, n-2)
  %8 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.2, i32 noundef %4, i32 noundef %6, i32 noundef %7)
  %9 = tail call i32 @Fibonacci(i32 noundef %6)     ; fibonacci(n-1)
  %10 = add nsw i32 %9, %3                          ; %10 = fibonacci(n-1) + accumulatore
  br label %2

11:                                               ; preds = %2
  br label %12

12: ; uno dei due if                              ; preds = %2, %11
  %13 = phi ptr [ @.str.1, %11 ], [ @.str, %2 ]                                         ; %13 = "f(1)=1" oppure "f(0)=0" in base a da quale if provengo
  %14 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) %13)    ; printf()
  %15 = add nsw i32 %4, %3                                                              ; %15 = 0/1 in base a quale if sono
  ret i32 %15
}

attributes #0 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #3 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 19.1.7 (/home/runner/work/llvm-project/llvm-project/clang cd708029e0b2869e80abe31ddb175f7c35361f90)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"any pointer", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
