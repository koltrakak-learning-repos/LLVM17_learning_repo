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
          B.printAsOperand(errs(), true);
          errs() << "\n";
        }

        // - Come recuperare l’handle al loop che contiene un dato basic block
        Loop *loop = LI.getLoopFor(&B);
        if (loop) {
          B.printAsOperand(errs(), true);
          errs() << " appartiene al loop con header: ";
          loop->getHeader()->printAsOperand(errs(), true);
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
}; // LoopInfoPass

struct LoopIterPass : PassInfoMixin<LoopIterPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    // Cerchiamo di capire come:
    // - Verificare che il loop sia in forma normale
    // - Recuperare blocchi significativi del loop
    //     - Preheader
    //     - Header
    // - Scorrere i basic blocks che compongono un loop

    for (auto *L : LI) {
      errs() << "Trovato un loop, il suo header è: ";
      L->getHeader()->printAsOperand(errs(), true);
      errs() << "\n";

      if (!L->isLoopSimplifyForm())
        errs() << "\tNON è in forma normale\n";

      errs() << "\tI suoi blocchi significativi sono:\n";
      errs() << "\t\tPreheader: ";
      L->getLoopPreheader()->printAsOperand(errs(), true);
      errs() << "\n";
      errs() << "\t\tHeader: ";
      L->getHeader()->printAsOperand(errs(), true);
      errs() << "\n";

      errs() << "\tTutti i suoi blocchi :\n";
      for (auto *B : L->getBlocks()) {
        errs() << "\t\t";
        B->printAsOperand(errs(), true);
        errs() << "\n";
      }
    }

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // LoopIterPass

struct LoopPass : PassInfoMixin<LoopPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    // 1. Verifichi se il CFG corrente contiene loop. Se no, ritorni subito
    // 2. Scorra tutti i basic block (BB) del CFG, e per ciascuno di essi
    // verifichi se è l’header di un loop. In tal caso stampi il BB.
    // 3. Scorra tutti i loop del CFG e per ciascuno di essi:
    //   a) Verifichi se è in forma normale
    //   b) Recuperi l’header del loop, e da lì recuperi l’handle alla funzione
    //   che lo contiene. Usando l’handle alla funzione così ottenuto (e NON
    //   sfruttando l’handle alla funzione passato dal Pass Manager) stampi il
    //   CFG
    //   c) Stampi tutti i blocchi che compongono il loop

    if (LI.begin() == LI.end())
      errs() << F.getName() << " non contiene loop\n\n";

    errs() << "In seguito tutti gli header dei loop di " << F.getName();
    for (BasicBlock &B : F) {
      if (LI.isLoopHeader(&B)) {
        errs() << B;
      }
    }

    errs() << "In seguito tutti i loop di " << F.getName();
    for (auto *L : LI) {
      errs() << "Loop: ";
      L->getHeader()->printAsOperand(errs(), true);
      errs() << "\n";

      if (!L->isLoopSimplifyForm())
        errs() << "NON è in forma normale\n";

      errs() << "Questo è il CFG della sua funzione padre\n";
      errs() << *L->getHeader()->getParent();

      errs() << "Tutti i suoi blocchi :\n";
      for (auto *B : L->getBlocks()) {
        errs() << "\t";
        B->printAsOperand(errs(), true);
        errs() << "\n";
      }

      errs() << "\n--------------------\n\n";
    }

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // LoopPass

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

                  if (Name == "loop-iter-pass") {
                    FPM.addPass(LoopIterPass());
                    return true;
                  }

                  if (Name == "loop-pass") {
                    FPM.addPass(LoopPass());
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
