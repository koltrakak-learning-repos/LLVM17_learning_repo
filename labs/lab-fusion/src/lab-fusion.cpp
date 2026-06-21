#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/IntrinsicInst.h>
// #include "llvm/ADT/BreadthFirstIterator.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"

#include <utility>
#include <vector>

using namespace llvm;

namespace {

bool checkLoopDependencies(Loop *L1, Loop *L2, DependenceInfo &DI) {
  // raccogliamo le memory instructions dei loop
  SmallVector<Instruction *, 16> MemInstrs;
  for (BasicBlock *BB : L1->blocks()) {
    for (Instruction &I : *BB) {
      if (I.mayReadOrWriteMemory())
        MemInstrs.push_back(&I);
    }
  }
  for (BasicBlock *BB : L2->blocks()) {
    for (Instruction &I : *BB) {
      if (I.mayReadOrWriteMemory())
        MemInstrs.push_back(&I);
    }
  }

  // confronta a coppie le istruzioni di memoria
  for (int i = 0; i < MemInstrs.size(); ++i) {
    for (int j = 0; j < MemInstrs.size(); ++j) {
      Instruction *Src = MemInstrs[i];
      Instruction *Dst = MemInstrs[j];

      // evitiamo controlli inutili
      if (Src == Dst && !isa<StoreInst>(Src))
        continue;

      // Calcola la dipendenza
      std::unique_ptr<Dependence> D = DI.depends(Src, Dst, true);
      if (!D)
        continue; // nessuna dipendenza trovata

      if (D->isDirectionNegative())
        return true;
    }
  }

  return false;
}

struct MyFusionPass : PassInfoMixin<MyFusionPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

    errs() << "\n----------" << F.getName() << "-----------\n\n";

    // NB: Passo 1 trovare i loop adiacenti
    // FIXME: faccio un doppio ciclo naive e quindi molto lavoro inutile
    std::vector<std::pair<Loop *, Loop *>> CandidateFusibleLoops;
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
          CandidateFusibleLoops.push_back(std::make_pair(L1, L2));
        }
        // - Se i loop sono guarded il successore non loop del guard
        // branch di L0 deve essere il guard block di L1.
        // TODO: questo non ho voglia di implementarlo
      }
    }

    for (auto [L1, L2] : CandidateFusibleLoops) {
      errs() << "Loop adiacenti:\n";
      errs() << "\t(";
      L1->getHeader()->printAsOperand(errs(), false);
      errs() << " ,";
      L2->getHeader()->printAsOperand(errs(), false);
      errs() << ")\n";
      errs() << "\n";
    }

    // NB: Passo 2, i loop di una coppia devono avere lo stesso trip count
    errs() << "Trip count\n";
    llvm::erase_if(CandidateFusibleLoops, [&](const auto &pair) {
      auto [L1, L2] = pair;
      // un oggetto SCEV rappresenta una scalar evolution come una funzione.
      // Ad esempio, un'espressione dentro a un loop di questo tipo: c = c +
      // 2, viene rappresentata come f(n) = base + n*step = 0 + n*2; dove n
      // rappresenta l'n-esima iterazione
      const SCEV *BackedgeCount1 = SE.getBackedgeTakenCount(L1);
      const SCEV *BackedgeCount2 = SE.getBackedgeTakenCount(L2);

      if (isa<SCEVCouldNotCompute>(BackedgeCount1) ||
          isa<SCEVCouldNotCompute>(BackedgeCount2))
        // se il trip count non è calcolabile elimino
        return true;
      else if (BackedgeCount1 != BackedgeCount2)
        return true;
      else
        return false;
    });

    errs() << "Loop con trip count uguale:\n";
    for (auto [L1, L2] : CandidateFusibleLoops) {
      errs() << "\t(";
      L1->getHeader()->printAsOperand(errs(), false);
      errs() << " ,";
      L2->getHeader()->printAsOperand(errs(), false);
      errs() << ")\n";
      errs() << "\n";
    }

    // NB: Passo 3, i loop di una coppia devono essere control-flow equivalent
    // se esegue uno esegue anche l'altro. Questo si traduce in:
    // - L1 domina L2? (se arrivo ad L2 sono passato per L1?)
    // - L2 postdomina L1? (se eseguo L1 dopo dovrò anche eseguire L2?)
    llvm::erase_if(CandidateFusibleLoops, [&](const auto &pair) {
      auto [L1, L2] = pair;
      return !DT.dominates(L1->getHeader(), L2->getHeader()) ||
             !PDT.dominates(L2->getHeader(), L1->getHeader());
    });

    errs() << "Loop control-flow equivalent:\n";
    for (auto [L1, L2] : CandidateFusibleLoops) {
      errs() << "\t(";
      L1->getHeader()->printAsOperand(errs(), false);
      errs() << " ,";
      L2->getHeader()->printAsOperand(errs(), false);
      errs() << ")\n";
      errs() << "\n";
    }

    // NB: Passo 4, there cannot be any negative distance dependencies
    // between Lj and Lk
    // - A negative distance dependence occurs between Lj and Lk, Lj before Lk,
    // when at iteration m from Lk uses a value that is computed by Lj at a
    // future iteration m+n (where n > 0).
    // - es: Lj scrive a[i] e Lk legge a[i+1]. Se fondessi, il secondo statement
    // che legge a[i+1] leggerebbe un valore non ancora scritto
    llvm::erase_if(CandidateFusibleLoops, [&](const auto &pair) {
      auto [L1, L2] = pair;
      return checkLoopDependencies(L1, L2, DI);
    });

    errs() << "Loop senza dipendenze negative:\n";
    for (auto [L1, L2] : CandidateFusibleLoops) {
      errs() << "\t(";
      L1->getHeader()->printAsOperand(errs(), false);
      errs() << " ,";
      L2->getHeader()->printAsOperand(errs(), false);
      errs() << ")\n";
      errs() << "\n";
    }

    // per i loop fondibili rimanenti devo fare due cose:
    // - Modificare gli usi della induction variable del loop 2 con quelli della
    // induction variable del loop 1
    // - Modificare il CFG perché il body del loop 2 sia agganciato a seguito
    // del body del loop 1 nel loop 1
    for (auto [L1, L2] : CandidateFusibleLoops) {
      auto L1_IV = L1->getInductionVariable(SE);
      auto L2_IV = L2->getInductionVariable(SE);
      if (!L1_IV || !L2_IV) {
        errs() << "Induction variables non trovate.\n";
        continue;
      }
      L2_IV->replaceAllUsesWith(L1_IV);

      // per fondere i loop dobbiamo:
      // - far puntare il branch del corpo (predecessore del latch) di L1, al
      // corpo di L2
      // - far puntare il body di L2 al latch di L1
      // - far puntare l'exiting block di L1 (header) verso l'exit block di L2
      //
      // Inoltre, facciamo puntare l'header di L2 direttamente al suo latch
      // (saltando il suo corpo) in maniera tale che un successivo passo di
      // 'SimplifyCFG' possa eliminare i basic block inutili
      BasicBlock *L1_Header = L1->getHeader();
      BasicBlock *L1_Latch = L1->getLoopLatch();
      BasicBlock *L1_Exit = L1->getExitBlock();
      BasicBlock *L2_Header = L2->getHeader();
      BasicBlock *L2_Latch = L2->getLoopLatch();
      BasicBlock *L2_Exit = L2->getExitBlock();
      // il blocco del body del loop che mi interessa è il predecessore del
      // latch in quanto contiene la branch che voglio modificare
      BasicBlock *L1_Body = L1_Latch->getSinglePredecessor();
      BasicBlock *L2_Body = L2_Latch->getSinglePredecessor();

      if (!L1_Body || !L2_Body || !L1_Latch || !L2_Latch || !L1_Exit ||
          !L2_Exit) {
        errs() << "qualcosa è andato storto, probabilmente devo normalizzare "
                  "il CFG dei loop\n";
        continue;
      }

      // punta il branch del corpo di L1 al corpo di L2
      Instruction *L1_BodyTerm = L1_Body->getTerminator();
      L1_BodyTerm->replaceSuccessorWith(L1_Latch, L2_Body);
      // punta il branch del corpo di L2 al latch di L1
      Instruction *L2_BodyTerm = L2_Body->getTerminator();
      L2_BodyTerm->replaceSuccessorWith(L2_Latch, L1_Latch);
      // punta l'exiting block di L1 (header) verso l'exit block di L2
      Instruction *L1_Header_Term = L1_Header->getTerminator();
      L1_Header_Term->replaceSuccessorWith(L1_Exit, L2_Exit);
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
