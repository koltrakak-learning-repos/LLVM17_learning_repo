@g = external global i32, align 4

; --- ESEMPIO 2: CICLO CON USCITA ANTICIPATA (Early Exit) ---
; Rappresenta: for(i=0; i<n; i++) { if(g > limit) break; g += i; }
define dso_local i32 @early_exit_loop(i32 noundef %n, i32 noundef %limit) {
entry:
  br label %loop_cond

loop_cond:
  %i = phi i32 [ 0, %entry ], [ %i_next, %loop_body ]
  %cmp_n = icmp slt i32 %i, %n
  br i1 %cmp_n, label %check_exit, label %end

check_exit:
  %val = load i32, ptr @g, align 4
  %cmp_lim = icmp sgt i32 %val, %limit
  br i1 %cmp_lim, label %end, label %loop_body

loop_body:
  %new_val = add nsw i32 %val, %i
  store i32 %new_val, ptr @g, align 4
  %i_next = add nsw i32 %i, 1
  br label %loop_cond

end:
  %res = phi i32 [ 0, %loop_cond ], [ 1, %check_exit ] ; 1 se uscito col break
  ret i32 %res
}

; --- ESEMPIO 3: CICLO CON ISTRUZIONI INVARIANTI (Per testare LICM) ---
; Rappresenta: for(i=0; i<n; i++) { a = x * y; g += a + i; }
; L'operazione (x * y) è invariante rispetto al ciclo.
define dso_local void @licm_test(i32 noundef %n, i32 noundef %x, i32 noundef %y) {
entry:
  br label %header

header:
  %i = phi i32 [ 0, %entry ], [ %i_next, %latch ]
  %cmp = icmp slt i32 %i, %n
  br i1 %cmp, label %body, label %exit

body:
  ; Questa istruzione è Loop Invariant (non cambia mai nel ciclo)
  %invariant_mul = mul nsw i32 %x, %y

  %current_g = load i32, ptr @g, align 4
  %tmp = add nsw i32 %current_g, %invariant_mul
  %new_g = add nsw i32 %tmp, %i
  store i32 %new_g, ptr @g, align 4
  br label %latch

latch:
  %i_next = add nsw i32 %i, 1
  br label %header

exit:
  ret void
}
