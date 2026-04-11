// #ifndef TOY_TOYPASSES_H
// #define TOY_TOYPASSES_H

// #include "mlir/Pass/Pass.h"
// #include <memory>

// #include "toy/ToyDialect.h"
// #include "toy/ToyOps.h"

// namespace mlir {
// namespace standalone {
// #define GEN_PASS_DECL
// #include "toy/ToyPasses.h.inc"

// #define GEN_PASS_REGISTRATION
// #include "toy/ToyPasses.h.inc"
// } // namespace standalone
// } // namespace mlir

// #endif // TOY_TOYPASSES_H

//===----------------------------------------------------------------------===//
//
// This file exposes the entry points to create compiler passes for Toy.
//
//===----------------------------------------------------------------------===//

#ifndef TOY_PASSES_H
#define TOY_PASSES_H

#include <memory>

namespace mlir {
class Pass;

namespace toy {
std::unique_ptr<Pass> createShapeInferencePass();
} // namespace toy
} // namespace mlir

#endif // TOY_PASSES_H
