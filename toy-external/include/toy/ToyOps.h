#ifndef TOY_TOYOPS_H
#define TOY_TOYOPS_H

// header copiati da standalone (forse non servono tutti ma l'importante è
// riuscire a buildare)
#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"

// header definiti nel file td
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
// #include "mlir/Interfaces/InferTypeOpInterface.h"

#define GET_OP_CLASSES
#include "toy/ToyOps.h.inc"

#endif // TOY_TOYOPS_H
