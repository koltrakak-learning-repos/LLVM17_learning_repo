#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/IntrinsicInst.h>
// #include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Dominators.h"

#include <utility>
#include <vector>

using namespace llvm;

namespace {

struct MyFusionPass : PassInfoMixin<MyFusionPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);

    errs() << "\n----------" << F.getName() << "-----------\n\n";

    // NB: Passo 1 trovare i loop adiacenti
    // FIXME: faccio un doppio ciclo naive e quindi molto lavoro inutile
    std::vector<std::pair<Loop *, Loop *>> LoopAdiacenti;
    for (auto *L1 : LI.getLoopsInPreorder()) {
      errs() << "Analizzo il loop con header: ";
      L1->getHeader()->printAsOperand(errs(), false);
      errs() << "\n";

      if (!L1->isLoopSimplifyForm()) {
        errs() << "\tNON è in forma normale! Skip di questo loop\n";
        continue;
      }

      // se ho più exit-block vuol dire che ho un goto ... skip
      if (!L1->getExitBlock())
        continue;

      for (auto *L2 : LI.getLoopsInPreorder()) {
        if (!L2->isLoopSimplifyForm())
          continue;

        // - Se i loop non sono guarded L’EXIT BLOCK DI L0 DEVE ESSERE IL
        // PREHEADER DI L1
        if (L2->getLoopPreheader() == L1->getExitBlock()) {
          LoopAdiacenti.push_back(std::make_pair(L1, L2));
        }
        // - Se i loop sono guarded il successore non loop del guard
        // branch di L0 deve essere il guard block di L1.
        // TODO: questo non ho voglia di implementarlo
      }
    }

    for (auto [L1, L2] : LoopAdiacenti) {
      errs() << "Loop adiacenti:\n";
      errs() << "\t(";
      L1->getHeader()->printAsOperand(errs(), false);
      errs() << " ,";
      L2->getHeader()->printAsOperand(errs(), false);
      errs() << ")\n";
      errs() << "\n";
    }

    // ...

    // NB: Passo 3 i loop di una coppia devono essere control-flow equivalent
    // se esegue uno esegue anche l'altro. Questo si traduce in:
    // - L1 domina L2? (se arrivo ad L2 sono passato per L1?)
    // - L2 postdomina L1? (se eseguo L1 dopo dovrò anche eseguire L2?)
    llvm::erase_if(LoopAdiacenti, [&](const auto &pair) {
      auto [L1, L2] = pair;
      return !DT.dominates(L1->getHeader(), L2->getHeader()) ||
             !PDT.dominates(L2->getHeader(), L1->getHeader());
    });

    for (auto [L1, L2] : LoopAdiacenti) {
      errs() << "Loop control-flow equivalent:\n";
      errs() << "\t(";
      L1->getHeader()->printAsOperand(errs(), false);
      errs() << " ,";
      L2->getHeader()->printAsOperand(errs(), false);
      errs() << ")\n";
      errs() << "\n";
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
