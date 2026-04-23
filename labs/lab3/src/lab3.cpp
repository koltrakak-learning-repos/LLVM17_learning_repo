#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;

namespace {

struct LoopInfoPass : PassInfoMixin<LoopInfoPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    // - Come determinare se il CFG non contiene loop
    if (!LI.empty()) {
      errs() << "la funzione: " << F.getName() << " ha dei loop!\n";

      for (BasicBlock &B : F) {

        // - Come capire se un basic block del CFG è l’header di un loop
        if (LI.isLoopHeader(&B)) {
          errs() << "Header trovato: ";
          B.printAsOperand(errs(), true); // Stamperà %4
          errs() << "\n";
        }

        // - Come recuperare l’handle al loop che contiene un dato basic block
        Loop *loop = LI.getLoopFor(&B);
        if (loop) {
          B.printAsOperand(errs(), true); // Stamperà %4, %6 o %8
          errs() << " appartiene al loop con header: ";
          loop->getHeader()->printAsOperand(errs(), true); // Stamperà %4
          errs() << "\n";
        }
      }
    }

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // MultiInstructionPass

} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------

llvm::PassPluginLibraryInfo getLocalOptsPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Lab3Passes", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "loop-info-pass") {
                    FPM.addPass(LoopInfoPass());
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
