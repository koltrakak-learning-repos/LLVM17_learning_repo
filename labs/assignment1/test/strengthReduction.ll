define dso_local i32 @foo(i32 noundef %0, i32 noundef %1) #0 {
  %3 = mul nsw i32 %0, 9
  ; %4 = mul nsw i32 3, %1
  %4 = sdiv i32 3, %1
  %5 = add nsw i32 %3, %4
  ret i32 %5
}

