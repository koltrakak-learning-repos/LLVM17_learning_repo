define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) #0 {
  %3 = add nsw i32 %0, 1
  ; %4 = %0+1-1 = %0
  %4 = sub nsw i32 %3, 1
  ; %5 = %3 -1 -(-1) = %3
  %5 = sub nsw i32 %4, -1
  ; %6 = %0+1-1-(-1)-1 = %0
  %6 = sub nsw i32 %5, 1

  ret i32 %6
}
