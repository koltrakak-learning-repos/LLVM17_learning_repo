@g = external global i32, align 4

; --- CICLO CON USCITA ANTICIPATA ---
; for(i=0; i<n; i++) {
;   if(g > limit)
;     break;
;   g += i;
; }
;
define dso_local i32 @early_exit_loop(i32 noundef %n, i32 noundef %limit, i32 noundef %x, i32 noundef %y) {
preheader:
  br label %header

header:
  %i = phi i32 [ 0, %preheader ], [ %i_next, %latch ]
  %cmp_n = icmp slt i32 %i, %n
  br i1 %cmp_n, label %exiting, label %exit

exiting:
  %val = load i32, ptr @g, align 4
  %cmp_lim = icmp sgt i32 %val, %limit          ; check per l'uscita anticipata
  br i1 %cmp_lim, label %exit, label %latch

latch:
  %new_val = add nsw i32 %val, %i
  store i32 %new_val, ptr @g, align 4
  %i_next = add nsw i32 %i, 1
  br label %header                              ; backedge verso l'header

exit:
  %res = phi i32 [ 0, %header ], [ 1, %exiting ] ; 1 se uscito col break
  br label %preheader2                          ; fallthrough esplicito

; --- CICLO CON ISTRUZIONI INVARIANTI ---
; for(i=0; i<n; i++) {
;   a = x * y;
;   g += a + i;
; }
;
; L'operazione (x * y) è invariante rispetto al ciclo.
;
preheader2:
  br label %header2

header2:
  %i2 = phi i32 [ 0, %preheader2 ], [ %i_next2, %latch2 ]
  %cmp = icmp slt i32 %i2, %n
  br i1 %cmp, label %body2, label %exit2

body2:
  %invariant_mul = mul nsw i32 %x, %y ; istruzione è Loop Invariant

  %current_g = load i32, ptr @g, align 4
  %tmp = add nsw i32 %current_g, %invariant_mul
  %new_g = add nsw i32 %tmp, %i2
  store i32 %new_g, ptr @g, align 4
  br label %latch2

latch2:
  %i_next2 = add nsw i32 %i2, 1
  br label %header2

exit2:
  ret i32 %res
}
