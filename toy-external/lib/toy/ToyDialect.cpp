#include "toy/ToyDialect.h"
#include "toy/ToyOps.h"
// #include "toy/ToyTypes.h"

#include <iostream>

using namespace mlir;
using namespace mlir::toy;

#include "toy/ToyDialect.cpp.inc"

//===----------------------------------------------------------------------===//
// ToyDialect
//===----------------------------------------------------------------------===//

/// Dialect initialization, the instance will be owned by the context. This is
/// the point of registration of types and operations for the dialect.
void ToyDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "toy/ToyOps.cpp.inc"
      >();
  //   registerTypes();

  std::cout << "ToyDialect inizializzato\n";
}
