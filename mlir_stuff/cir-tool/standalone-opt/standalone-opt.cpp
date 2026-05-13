//===- standalone-opt.cpp ---------------------------------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/DLTI/DLTI.h" // necessario per CIR
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h" // necessario per CIR
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "mlir/Transforms/Passes.h"

// ClangIR: dialetto e passi
#include "clang/CIR/Dialect/IR/CIRDialect.h"
#include "clang/CIR/Dialect/Passes.h"
// se vuoi lowering verso MLIR/LLVM:
// #include "clang/CIR/Passes.h"

#include "Standalone/StandaloneDialect.h"
#include "Standalone/StandalonePasses.h"

int main(int argc, char **argv) {
  mlir::registerAllPasses();
  mlir::standalone::registerPasses();
  // TODO: Register standalone passes here.

  mlir::registerCIRPasses();

  mlir::PassPipelineRegistration<>(
      "cir-pipeline", "Pipeline CIR: canonicalize + hoist-allocas",
      [](mlir::OpPassManager &pm) {
        pm.addPass(mlir::createCanonicalizerPass());
        pm.addPass(mlir::createHoistAllocasPass());
      });

  mlir::DialectRegistry registry;

  // i dialetti LLVM e DLTI sono necessari per CIR
  registry
      .insert<cir::CIRDialect, mlir::DLTIDialect, mlir::LLVM::LLVMDialect>();

  registry.insert<mlir::standalone::StandaloneDialect,
                  mlir::arith::ArithDialect, mlir::func::FuncDialect>();
  // Add the following to include *all* MLIR Core dialects, or selectively
  // include what you need like above. You only need to register dialects that
  // will be *parsed* by the tool, not the one generated
  // registerAllDialects(registry);

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Standalone optimizer driver\n", registry));
}
