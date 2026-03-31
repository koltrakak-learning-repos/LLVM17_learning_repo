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
enum Action { None, DumpAST };
} // namespace

// flag argument che accetta solo valori corrispondenti all'enumerativo. Notare
// cl::values che definisce la mappa tra cli strings ed enum values; se l'utente
// inserisce una stringa invalida, viene sollevato un errore automaticamente. La
// sintassi del flag è indifferente (-/--; =/' '). Non c'è un valore di default
// esplicito (cl::init()) per cui viene usato il valore di default degli enums
// (interi) 0 che corrisponde a None.
static cl::opt<enum Action>
    emitAction("emit", cl::desc("Select the kind of output desired"),
               cl::values(clEnumValN(DumpAST, "ast", "output the AST dump")));

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

int main(int argc, char **argv) {
  // Il costruttore del cl::opt registra le opzioni in un registro. Questa
  // funzione fa il parsing automaticamente delle opzioni registrate e popola
  // le variabili corrispondenti
  cl::ParseCommandLineOptions(argc, argv, "toy compiler\n");

  auto moduleAST = parseInputFile(inputFilename);
  if (!moduleAST)
    return 1;

  switch (emitAction) {
  case Action::DumpAST:
    dump(*moduleAST);
    return 0;
  default:
    llvm::errs() << "No action specified (parsing only?), use -emit=<action>\n";
  }

  return 0;
}
