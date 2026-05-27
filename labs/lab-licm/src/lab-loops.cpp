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

#define DEBUG

using namespace llvm;

namespace {

std::vector<Instruction *> findLIInstr(const LoopInfo &LI, const Loop *L) {
  std::vector<Instruction *> LIInstructions;
  bool changed = true;

  // NB: siccome un istruzione è l.i. anche quando i suoi operandi sono l.i., mi
  // salvo in un set le istruzioni l.i. che ho trovato fino ad ora, e continuo a
  // cercare iterando fin quando il mio set non converge. Questa strategia
  // gestisce il caso in cui processi un'istruzione PRIMA di uno dei suoi
  // argomenti l.i.
  //
  // In realtà, dovrei procedere con un'altra iterazione solo se trovo una
  // nuova istruzione l.i. che ha degli user dentro al loop che non sono
  // stati marcati come l.i.
  //
  // TODO: il prof suggerisce un approccio ricorsivo che sembra più semplice:
  // un'istruzione è l.i. (oltre ad altre condizioni) quando i suoi argomenti
  // sono l.i.
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

  return LIInstructions;
}

std::vector<Instruction *>
findHoistableInstr(const DominatorTree &DT, const Loop *L,
                   std::vector<Instruction *> LIInstructions) {
  std::vector<Instruction *> HoistableInstructions;
  bool DominatesAllExits = true;
  bool NoUsersOutOfLoop = true;

  for (auto *LIInstr : LIInstructions) {
    // l'istruzione l.i. si trova in un blocco che domina tutte le uscite
    // del loop?
    BasicBlock *BBOfCandidate = LIInstr->getParent();
    SmallVector<llvm::BasicBlock *> ExitBlocks;
    L->getExitBlocks(ExitBlocks);
    DominatesAllExits = llvm::all_of(ExitBlocks, [&](BasicBlock *ExitBlock) {
      return DT.dominates(BBOfCandidate, ExitBlock);
    });

    // Oppure la variabile definita dall’istruzione è dead all’uscita del
    // loop
    NoUsersOutOfLoop = true;
    for (auto *user : LIInstr->users()) {
      auto BBOfUser = dyn_cast<Instruction>(user)->getParent();

      bool FoundUserOutOfLoop =
          llvm::any_of(ExitBlocks, [&](BasicBlock *ExitBlock) {
            return DT.dominates(ExitBlock, BBOfUser);
          });

      if (FoundUserOutOfLoop) {
        NoUsersOutOfLoop = false;
        break;
      }
    }

    // l'istruzione l.i. assegna un valore a una variabile assegnata altrove
    // nel loop? No, siamo in SSA e quindi ogni variabile ha una sola
    // definizione

    // l'istruzione si trova in blocco che domina tutti i blocchi nel loop
    // che usano la variabile a cui si sta assegnando un valore? Si, siamo
    // in SSA e quindi una definizione domina tutti gli usi, anche fuori dal
    // loop

    if (DominatesAllExits || NoUsersOutOfLoop)
      HoistableInstructions.push_back(LIInstr);
  }

  return HoistableInstructions;
}

struct MyLICMPass : PassInfoMixin<MyLICMPass> {

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

#ifdef DEBUG
      errs() << "\n";
      DT.print(errs());
      errs() << "\n";
#endif

      std::vector<Instruction *> LIInstructions = findLIInstr(LI, L);
      errs() << "Istruzioni LoopInvariant del loop: \n";
      for (auto *I : LIInstructions) {
        errs() << "\t" << *I << "\n";
      }

      std::vector<Instruction *> HoistableInstructions =
          findHoistableInstr(DT, L, LIInstructions);
      errs() << "Hoistable Instructions: \n";
      for (auto *I : HoistableInstructions) {
        errs() << "\t" << *I << "\n";
      }

      // Spostare l’istruzione candidata nel preheader se tutte le istruzioni
      // invarianti da cui questa dipende sono state spostate
      //
      // NB: devo fare attenzione a ordinare nel preheader le istruzioni
      // hoistable nell'ordine di dipendenza per non rompere il dataflow. Per
      // fare cio, possiamo fare una visita DFS del DT, questo mi darà un
      // ordinamento topologico delle istruzioni
      BasicBlock *PreHeader = L->getLoopPreheader();
      for (auto *domNode : depth_first(DT.getRootNode())) {
        for (auto *I : HoistableInstructions) {
          if (domNode->getBlock() == I->getParent()) {
            I->moveBefore(PreHeader->getTerminator());
          }
        }
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
