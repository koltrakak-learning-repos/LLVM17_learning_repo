@g = external global i32, align 4

; --- ESEMPIO 1: CICLI ANNIDATI (Nested Loops) ---
; Rappresenta:
; for(i=0; i<n; i++) {
;   for(j=0; j<m; j++) {
;     g += x;
;   }
; }
define dso_local void @nested_loops(i32 noundef %n, i32 noundef %m, i32 noundef %x) {
entry:
  br label %outer_cond

outer_cond:                                       ; Header del ciclo esterno
  %i = phi i32 [ 0, %entry ], [ %i_next, %outer_latch ]
  %cmp_i = icmp slt i32 %i, %n
  br i1 %cmp_i, label %inner_cond, label %exit

inner_cond:                                       ; Header del ciclo interno
  %j = phi i32 [ 0, %outer_cond ], [ %j_next, %inner_body ]
  %cmp_j = icmp slt i32 %j, %m
  br i1 %cmp_j, label %inner_body, label %outer_latch

inner_body:                                       ; Corpo del ciclo interno
  %val = load i32, ptr @g, align 4
  %sum = add nsw i32 %val, %x
  store i32 %sum, ptr @g, align 4
  %j_next = add nsw i32 %j, 1
  br label %inner_cond

outer_latch:                                      ; Latch del ciclo esterno
  %i_next = add nsw i32 %i, 1
  br label %outer_cond

exit:
  ret void
}

