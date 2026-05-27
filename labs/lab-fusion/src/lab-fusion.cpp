#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/IntrinsicInst.h>
// #include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"

#include <vector>

using namespace llvm;

namespace {

struct MyFusionPass : PassInfoMixin<MyFusionPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);

    errs() << "\n----------" << F.getName() << "-----------\n\n";

    for (auto *L : LI) {
      errs() << "Trovato un loop, il suo header è: ";
      L->getHeader()->printAsOperand(errs(), false);
      errs() << "\n";

      if (!L->isLoopSimplifyForm()) {
        errs() << "\tNON è in forma normale! Skip di questo loop\n";
        continue;
      }
    }

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
  return {LLVM_PLUGIN_API_VERSION, "LabFusion", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "my-fusion") {
                    FPM.addPass(MyFusionPass());
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
