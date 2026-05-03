@g = external global i32, align 4

; --- ESEMPIO 1: CICLI ANNIDATI ---
; Rappresenta:
; for(i=0; i<n; i++) {
;   for(j=0; j<m; j++) {
;     g += x;
;   }
; }
define dso_local void @nested_loops(i32 noundef %n, i32 noundef %m, i32 noundef %x) {
preheader:
  br label %outer_header

; questo potrebbe sembrare il preheader del ciclo esterno, in realtà non è
; classificabile come preheader dato che non ha un unico edge dedicato verso
; 'inner_header' ("loop-simplify" inserisce proprio un blocco di questo tipo
; tra outer_header e inner_header)
outer_header:
  %i = phi i32 [ 0, %preheader ], [ %i_next, %outer_latch ]
  %cmp_i = icmp slt i32 %i, %n
  br i1 %cmp_i, label %inner_header, label %exit

inner_header:
  %j = phi i32 [ 0, %outer_header ], [ %j_next, %inner_latch ]
  %cmp_j = icmp slt i32 %j, %m
  br i1 %cmp_j, label %body, label %outer_latch

body:
  %val = load i32, ptr @g, align 4
  %sum = add nsw i32 %val, %x
  store i32 %sum, ptr @g, align 4
  br label %inner_latch

inner_latch:
  %j_next = add nsw i32 %j, 1
  br label %inner_header

outer_latch:
  %i_next = add nsw i32 %i, 1
  br label %outer_header

exit:
  ret void
}

