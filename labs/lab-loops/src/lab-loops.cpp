#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <llvm/IR/IntrinsicInst.h>
#include <vector>

using namespace llvm;

namespace {

struct MyLICMPass : PassInfoMixin<MyLICMPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    errs() << "\n----------" << F.getName() << "-----------\n\n";

    for (auto *L : LI) {
      errs() << "Trovato un loop, il suo header è: ";
      L->getHeader()->printAsOperand(errs(), false);
      errs() << "\n";

      if (!L->isLoopSimplifyForm())
        errs() << "\tNON è in forma normale\n";

      std::vector<Instruction *> LIInstructions;
      for (auto *B : L->getBlocks()) {
        for (auto &I : *B) {
          bool loopInvariantArguments = true;

          for (Value *Operand : I.operands()) {
            if (auto *OperandInst = dyn_cast<Instruction>(Operand)) {
              Loop *LoopOfOperand = LI.getLoopFor(OperandInst->getParent());
              if (LoopOfOperand && L == LoopOfOperand) {
                // TODO: fai anche gli altri casi
                loopInvariantArguments = false;
              }
              // operando definito fuori da loop, OPPURE, fuori da questo loop
            }
          }

          if (loopInvariantArguments)
            LIInstructions.push_back(&I);
        }
      }

      errs() << "Istruzioni LoopInvariant del loop: \n";
      for (auto *I : LIInstructions) {
        errs() << "\t" << *I << "\n";
      }
    }

    // prova a confrontare con i metodi di Loop
    // bool 	isLoopInvariant (const Value *V) const
    //  	Return true if the specified value is loop invariant.
    // bool 	hasLoopInvariantOperands (const Instruction *I) const
    //  	Return true if all the operands of the specified instruction are
    //  loop invariant.

    errs() << "\n----------------------------\n\n";

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // MyLICMPass

} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------

llvm::PassPluginLibraryInfo getLocalOptsPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LabLoopsPasses", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "my-licm") {
                    FPM.addPass(MyLICMPass());
                    return true;
                  }

                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize TestPass when added to the pass pipeline on the
// command line, i.e. via '-passes=test-pass'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLocalOptsPassPluginInfo();
}
