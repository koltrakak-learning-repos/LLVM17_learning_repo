
func.func @factorial(%arg0: i32) -> i32 {
    %c0 = arith.constant 0 : i32
    %c1 = arith.constant 1 : i32
    %eq = arith.cmpi eq, %arg0, %c0 : i32
    %result = scf.if %eq -> (i32) {
      // Caso base: n == 0
      scf.yield %c1 : i32
    } else {
      // Caso ricorsivo: n - 1
      %sub = arith.subi %arg0, %c1 : i32
      %res = func.call @factorial(%sub) : (i32) -> i32
      // n * factorial(n - 1)
      %mult = arith.muli %arg0, %res : i32 
      scf.yield %mult : i32
    }
    return %result : i32
}
