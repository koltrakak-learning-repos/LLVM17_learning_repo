#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

// #include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"

#include <vector>

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
          B.printAsOperand(errs(), false);
          errs() << "\n";
        }

        // - Come recuperare l’handle al loop che contiene un dato basic block
        Loop *loop = LI.getLoopFor(&B);
        if (loop) {
          B.printAsOperand(errs(), false);
          errs() << " appartiene al loop con header: ";
          loop->getHeader()->printAsOperand(errs(), false);
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
      L->getHeader()->printAsOperand(errs(), false);
      errs() << "\n";

      if (!L->isLoopSimplifyForm())
        errs() << "\tNON è in forma normale\n";

      errs() << "\tI suoi blocchi significativi sono:\n";
      errs() << "\t\tPreheader: ";
      L->getLoopPreheader()->printAsOperand(errs(), false);
      errs() << "\n";
      errs() << "\t\tHeader: ";
      L->getHeader()->printAsOperand(errs(), false);
      errs() << "\n";

      errs() << "\tTutti i suoi blocchi :\n";
      for (auto *B : L->getBlocks()) {
        errs() << "\t\t";
        B->printAsOperand(errs(), false);
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

    errs() << "\n----------" << F.getName() << "-----------\n\n";

    if (LI.begin() == LI.end()) {
      errs() << F.getName() << " non contiene loop\n";
    } else {
      errs() << "In seguito tutti i loop-header di: " << F.getName() << "\n";
      for (BasicBlock &B : F) {
        if (LI.isLoopHeader(&B)) {
          errs() << "\t";
          B.printAsOperand(errs(), false);
          errs() << "\n";
        }
      }
      errs() << "\n";

      errs() << "In seguito tutti i loop di: " << F.getName() << "\n";
      // se voglio iterare anche i loop innestati devo utilizzare dei
      // metodi specifici di LoopInfo
      for (auto *L : LI.getLoopsInPreorder()) {
        // errs() << "\nQuesto è il CFG della sua funzione padre\n";
        // errs() << *L->getHeader()->getParent();
        errs() << "\theader: ";
        L->getHeader()->printAsOperand(errs(), false);
        errs() << "\n";

        if (!L->isLoopSimplifyForm())
          errs() << "\t\tNON è in forma normale\n";

        errs() << "\ttutti i blocchi:\n";
        for (auto *B : L->getBlocks()) {
          auto *innerMostLoop = LI.getLoopFor(B);
          if (L != innerMostLoop)
            // blocco appartenente ad un loop più interno; verrà stampato dopo
            continue;

          errs() << "\t\t";
          B->printAsOperand(errs(), false);
          errs() << "\n";
        }

        auto subLoops = L->getSubLoops();
        for (auto *subLoop : subLoops) {
          errs() << "\t\tSubloop con header: ";
          subLoop->getHeader()->printAsOperand(errs(), false);
          errs() << "\n";
          // FIXME: qua sto facendo una porcata dato che getLoopPredecessor()
          // potrebbe restituirmi null se ci sono predecessori multipli, e non
          // sto controllando prima di dereferenziare.
          //
          // non proprio preheader dato che i preheader devono avere solamente
          // un edge verso l'header; un predecessore singolo può avere altri
          // edges
          errs() << "\t\t\tIl suo unico predecessore (non preheader) è: ";
          subLoop->getLoopPredecessor()->printAsOperand(errs(), false);
          errs() << "\n";

          SmallVector<BasicBlock *, 4> ExitingBlocks;
          subLoop->getExitingBlocks(ExitingBlocks);
          errs() << "\t\te con i seguenti exiting blocks: \n";
          for (auto *exitingBlock : ExitingBlocks) {
            errs() << "\t\t\t";
            exitingBlock->printAsOperand(errs(), false);
            errs() << " -> exits to -> ";
            // NB: purtroppo, non esiste una funzione B->getSuccessors(); devo
            // passare dalla terminator instruction e iterare anche fin troppo
            // manualmente
            const Instruction *TI = exitingBlock->getTerminator();
            for (unsigned i = 0, e = TI->getNumSuccessors(); i != e; ++i) {
              BasicBlock *Succ = TI->getSuccessor(i);
              // voglio solamente i successori (exit) che rimangono dentro il
              // loop esterno, ma non dentro quello interno
              if (L->contains(Succ) && !subLoop->contains(Succ)) {
                Succ->printAsOperand(errs(), false);
                errs() << ", ";
              }
            }
            errs() << "\n";
          }
        }
        // const std::vector< Loop * > & 	getSubLoops () const
        // Return the loops contained entirely within this loop.

        // void getExitBlocks (SmallVectorImpl<BasicBlock * >
        // &ExitingBlocks) const
        //  	Return all blocks inside the loop that have successors outside
        //  of the loop.

        errs() << "\n";
      }
    }

    errs() << "\n--------------------\n\n";

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // LoopPass

struct DominanceTreePass : PassInfoMixin<DominanceTreePass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    errs() << "printing the dom-tree of: " << F.getName() << "\n";
    DT.print(errs());

    // Come scorrere i blocchi di un dominator tree (nodo radice, discendenti)
    for (auto *domNode : depth_first(DT.getRootNode())) {
      domNode->getBlock()->printAsOperand(errs(), false);
      errs() << "\n";
    }

    // Come stabilire le relazioni di dominanza tra basic block, istruzioni
    // e/o usi
    // - bool 	dominates (const BasicBlock *A, const BasicBlock *B) const
    // - metodi simili per il resto

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // DominanceTreePass

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

                  if (Name == "dom-tree-pass") {
                    FPM.addPass(DominanceTreePass());
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
