#include "llvm/ADT/STLExtras.h"
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

      // NB: siccome un istruzione è l.i. anche quando i suoi operandi sono
      // l.i., mi salvo in un set le istruzioni l.i. che ho trovato fino ad
      // ora, e continuo cercare iterando fin quando il mio set non converge.
      // Questa strategia gestisce il caso in cui processi un'istruzione PRIMA
      // di uno dei suoi argomenti l.i., in questo caso la singola iterazione
      // non classificherebbe l'istruzione come l.i.
      //
      // In realtà, dovrei procedere con un'altra iterazione solo se trovo una
      // nuova istruzione l.i. che ha degli user dentro al loop che non sono
      // stati marcati come l.i.
      std::vector<Instruction *> LIInstructions;
      bool changed = true;
      while (changed) {
        changed = false;

        for (auto *B : L->getBlocks()) {
          for (auto &I : *B) {
            // branch di fallthrough sono l.i., sicuramente però non posso
            // hoistarle. Di conseguenza le ignoro.
            if (I.isTerminator())
              continue;

            // istruzione già marcata come loop-invariant, non ho bisogno di
            // riprocessarla
            if (llvm::is_contained(LIInstructions, &I))
              continue;

            bool loopInvariantArguments = true;

            // Un'istruzione è loopInvariant se:
            // - i suoi operandi sono definiti fuori dal loop
            // - i suoi operandi sono costanti
            // - i suoi operandi sono definiti dentro al loop ma sono istruzioni
            //   loop-invariant
            for (Value *Operand : I.operands()) {
              if (auto *OperandInst = dyn_cast<Instruction>(Operand)) {
                Loop *LoopOfOperand = LI.getLoopFor(OperandInst->getParent());

                if (LoopOfOperand && L == LoopOfOperand) {
                  if (llvm::is_contained(LIInstructions, OperandInst)) {
                    // operando definito dentro al loop ma la definizione è L.I.
                  } else {
                    loopInvariantArguments = false;
                  }
                }

                // operando definito fuori da loop, OPPURE, fuori da questo loop
              }

              // altrimenti, operando costante e quindi loop-invariant
            }

            if (loopInvariantArguments) {
              LIInstructions.push_back(&I);
              changed = true;
            }
          }
        }
      }

      errs() << "Istruzioni LoopInvariant del loop: \n";
      for (auto *I : LIInstructions) {
        errs() << "\t" << *I << "\n";
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
