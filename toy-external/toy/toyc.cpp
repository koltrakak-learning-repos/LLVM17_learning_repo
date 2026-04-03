//===- toyc.cpp - The Toy Compiler ----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the entry point for the Toy compiler.
//
//===----------------------------------------------------------------------===//

#include "frontend/AST.h"
#include "frontend/Lexer.h"
#include "frontend/Parser.h"
#include "toy/ToyDialect.h"

#include "mlir/IR/AsmState.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Parser/Parser.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <string>
#include <system_error>

using namespace toy;
namespace cl = llvm::cl;

// positional cli argument for the input .toy file. Accepts an arbitrary string
static cl::opt<std::string> inputFilename(cl::Positional,
                                          cl::desc("<input toy file>"),
                                          cl::init("-"), // stdin default
                                          cl::value_desc("filename"));

namespace {
enum InputType { Toy, MLIR };
} // namespace
// flag agument '-x' che definisce il formato dell'input file
static cl::opt<enum InputType> inputType(
    "x", cl::init(Toy), cl::desc("Decided the kind of output desired"),
    cl::values(clEnumValN(Toy, "toy", "load the input file as a Toy source.")),
    cl::values(clEnumValN(MLIR, "mlir",
                          "load the input file as an MLIR file")));

namespace {
enum Action { None, DumpAST, DumpMLIR };
} // namespace
// flag argument che accetta solo valori corrispondenti all'enumerativo. Notare
// cl::values che definisce la mappa tra cli strings ed enum values; se l'utente
// inserisce una stringa invalida, viene sollevato un errore automaticamente. La
// sintassi del flag è indifferente (-/--; =/' '). Non c'è un valore di default
// esplicito (cl::init()) per cui viene usato il valore di default degli enums
// (interi) 0 che corrisponde a None.
static cl::opt<enum Action> emitAction(
    "emit", cl::desc("Select the kind of output desired"),
    cl::values(clEnumValN(DumpAST, "ast", "output the AST dump")),
    cl::values(clEnumValN(DumpMLIR, "mlir", "output the MLIR dump")));

/// Returns a Toy AST resulting from parsing the file or a nullptr on error.
static std::unique_ptr<toy::ModuleAST>
parseInputFile(llvm::StringRef filename) {
  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
      llvm::MemoryBuffer::getFileOrSTDIN(filename);
  if (std::error_code ec = fileOrErr.getError()) {
    llvm::errs() << "Could not open input file: " << ec.message() << "\n";
    return nullptr;
  }
  auto buffer = fileOrErr.get()->getBuffer();

  LexerBuffer lexer(buffer.begin(), buffer.end(), std::string(filename));
  Parser parser(lexer);
  return parser.parseModule();
}

static int dumpAST() {
  if (inputType == InputType::MLIR) {
    llvm::errs() << "Can't dump a Toy AST when the input is MLIR\n";
    return 5;
  }

  auto moduleAST = parseInputFile(inputFilename);
  if (!moduleAST)
    return 1;

  dump(*moduleAST);
  return 0;
}

static int dumpMLIR() {
  mlir::MLIRContext context;
  // Load our Dialect in this MLIR Context.
  context.getOrLoadDialect<mlir::toy::ToyDialect>();

  //   // Handle '.toy' input to the compiler.
  //   if (inputType != InputType::MLIR &&
  //       !llvm::StringRef(inputFilename).ends_with(".mlir")) {
  //     auto moduleAST = parseInputFile(inputFilename);
  //     if (!moduleAST)
  //       return 6;
  //     mlir::OwningOpRef<mlir::ModuleOp> module = mlirGen(context,
  //     *moduleAST); if (!module)
  //       return 1;

  //     module->dump();
  //     return 0;
  //   }

  //   // Otherwise, the input is '.mlir'.
  //   llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
  //       llvm::MemoryBuffer::getFileOrSTDIN(inputFilename);
  //   if (std::error_code ec = fileOrErr.getError()) {
  //     llvm::errs() << "Could not open input file: " << ec.message() << "\n";
  //     return -1;
  //   }

  //   // Parse the input mlir.
  //   llvm::SourceMgr sourceMgr;
  //   sourceMgr.AddNewSourceBuffer(std::move(*fileOrErr), llvm::SMLoc());
  //   mlir::OwningOpRef<mlir::ModuleOp> module =
  //       mlir::parseSourceFile<mlir::ModuleOp>(sourceMgr, &context);
  //   if (!module) {
  //     llvm::errs() << "Error can't load file " << inputFilename << "\n";
  //     return 3;
  //   }

  //   module->dump();
  return 0;
}

int main(int argc, char **argv) {
  // Il costruttore del cl::opt registra le opzioni in un registro. Questa
  // funzione fa il parsing automaticamente delle opzioni registrate e popola
  // le variabili corrispondenti
  mlir::registerAsmPrinterCLOptions();
  mlir::registerMLIRContextCLOptions();
  cl::ParseCommandLineOptions(argc, argv, "toy compiler\n");

  switch (emitAction) {
  case Action::DumpAST:
    dumpAST();
    return 0;
  case Action::DumpMLIR:
    return dumpMLIR();
  default:
    llvm::errs() << "No action specified (parsing only?), use -emit=<action>\n";
  }

  return 0;
}
