#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <cmath>
#include <cstdint>
#include <vector>

using namespace llvm;

namespace {

bool isPowerOfTwo(int64_t n) {
  // Le potenze di 2 sono positive
  if (n <= 0)
    return false;

  return (n & (n - 1)) == 0;
}

struct LocalOptsPass : PassInfoMixin<LocalOptsPass> {

  // mul by power of 2 strength reduction
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {

    for (BasicBlock &B : F) {
      for (Instruction &I : B) {
        if (auto *op = dyn_cast<BinaryOperator>(&I);
            op && op->getOpcode() == Instruction::Mul) {

          outs() << "trovata una moltiplicazione: " << *op << "\n";
          Value *lhs = op->getOperand(0);
          Value *rhs = op->getOperand(1);
          ConstantInt *lhsConstant = dyn_cast<ConstantInt>(lhs);
          ConstantInt *rhsConstant = dyn_cast<ConstantInt>(rhs);

          // lhs è la potenza di due
          if (lhsConstant && !rhsConstant &&
              isPowerOfTwo(lhsConstant->getSExtValue())) {

            int shift_value =
                static_cast<int>(std::log2(lhsConstant->getSExtValue()));
            outs() << "shifto di " << shift_value << "\n";

            LLVMContext &ctx = I.getContext();
            Type *int32Ty = Type::getInt32Ty(ctx);
            Constant *c = ConstantInt::get(int32Ty, shift_value);
            Instruction *shiftInst =
                BinaryOperator::Create(Instruction::Shl, rhs, c);
            shiftInst->insertAfter(&I);
            I.replaceAllUsesWith(shiftInst);
          }
          // rhs è la potenza di due
          else if (!lhsConstant && rhsConstant &&
                   isPowerOfTwo(rhsConstant->getSExtValue())) {

            int shift_value = std::log2(rhsConstant->getSExtValue());
            outs() << "shifto di " << shift_value << "\n";

            LLVMContext &ctx = I.getContext();
            Type *int32Ty = Type::getInt32Ty(ctx);
            Constant *c = ConstantInt::get(int32Ty, shift_value);

            Instruction *shiftInst =
                BinaryOperator::Create(Instruction::Shl, lhs, c);
            shiftInst->insertAfter(&I);
            I.replaceAllUsesWith(shiftInst);
          } else {
            outs() << "... ma non aveva una potenza di due come argomento :(\n";
            continue;
          }
        }
      }
    }

    return PreservedAnalyses::all();
  }

  // esperimenti
  PreservedAnalyses run2(Function &F, FunctionAnalysisManager &) {

    for (BasicBlock &B : F) {
      // Preleviamo le prime due istruzioni del BB
      Instruction &Inst1st = *B.begin();
      Instruction &Inst2nd = *(++B.begin());

      // L'indirizzo della prima istruzione deve essere uguale a quello del
      // primo operando della seconda istruzione (per costruzione dell'esempio)
      // NB: getOperand() restituisce un Value*, questo è in linea con la
      // filosofia SSA in cui le istruzioni sono valori aliased dal loro
      // registro virtuale di destinazione
      assert(&Inst1st == Inst2nd.getOperand(0));

      // Stampa la prima istruzione
      outs() << "PRIMA ISTRUZIONE: " << Inst1st << "\n";
      // Stampa la prima istruzione come operando
      // NB: stampare un'istruzione come operando, significa stampare il nome
      // del suo registro virtuale di destinazione, dato che questo è il nome
      // con cui le altre istruzioni posso riferircisi
      outs() << "COME OPERANDO: ";
      Inst1st.printAsOperand(outs(), true);
      outs() << "\n";

      // User-->Use-->Value
      outs() << "I MIEI OPERANDI SONO:\n";
      for (Value *Operand : Inst1st.operands()) {
        if (Argument *Arg = dyn_cast<Argument>(Operand)) {
          outs() << "\t" << *Arg << ": SONO L'ARGOMENTO N. " << Arg->getArgNo()
                 << " DELLA FUNZIONE " << Arg->getParent()->getName() << "\n";
        }
        if (ConstantInt *C = dyn_cast<ConstantInt>(Operand)) {
          outs() << "\t" << *C << ": SONO UNA COSTANTE INTERA DI VALORE "
                 << C->getValue() << "\n";
        }
      }

      // NB: nota che se un'istruzione è due volte un User (ha la stessa
      // istruzione come argomento due volte), verrà considerata due volte
      // durante l'iterazione
      outs() << "LA LISTA DEI MIEI USERS:\n";
      for (User *user : Inst1st.users()) {
        // Stampare come user o come instruction non cambia l'output stampato.
        // Siccome User è una superclasse però avrà meno metodi di Instruction
        // outs() << "\t" << *(dyn_cast<Instruction>(user)) << "\n";
        outs() << "\t" << *user << "\n";
      }

      // Qual è la differenza tra gli USERS e gli USES?
      // Prova a vedere cosa succede se in Foo.ll la seconda istruzione diventa
      // %4 = mul nsw i32 %3, %3
      //
      // Uno Use è un arco nel dataflow graph del programma che connette due
      // istruzioni: uno User e uno Usee (Value*). Dato un oggetto Use posso
      // recuperare entrambi:
      // - use.getUser()
      // - use.get() mi da lo usee come Value*
      //    - non strettamente necessario dato che Use viene trattato come
      //    Value* da chi lo manipola
      //    - fa override dell'operatore -> restituendo il Value* del usee
      //
      // Nota come indirizzi di User e Usee non cambiano quando usi multipli
      // appartengono alla stessa istruzione (due archi diversi tra gli stessi
      // nodi)
      outs() << "E DEI MIEI USI:\n";
      for (Use &use : Inst1st.uses()) {
        outs() << "\t" << "usee: " << use << "\n";
        outs() << "\t" << *(dyn_cast<Instruction>(use.get())) << "\n";
        outs() << "\t" << "user:" << use.getUser() << "@operand"
               << use.getOperandNo() << "\n";
        outs() << "\t" << *(dyn_cast<Instruction>(use.getUser())) << "\n";
      }

      // Manipolazione delle istruzioni
      Instruction *NewInst = BinaryOperator::Create(
          Instruction::Add, Inst1st.getOperand(0), &Inst1st);

      NewInst->insertAfter(&Inst1st);
      // Si possono aggiornare le singole references separatamente?
      // Controlla la documentazione e prova a rispondere.
      //   Inst1st.replaceAllUsesWith(NewInst);

      //// Soluzione 1:
      // Posso iterare a mano, però bisogna fare attenzione a non rompere la
      // linked list che c'è tra gli uses di uno stesso usee quando si fa un
      // set:
      //   outs() << "FACCIAMO A MANO" << "\n";
      //   std::vector<Use *> uses{};
      //   for (Use &use : Inst1st.uses()) {
      //     outs() << *(dyn_cast<Instruction>(use.getUser())) << "\n";
      //     uses.push_back(&use);
      //   }

      //   bool basta = false;
      //   for (Use *use : uses) {
      //     if (use->getUser() == NewInst) {
      //       outs() << "non voglio modificare anche NewInst nel caso abbia
      //       usi"
      //              << "\n";
      //       continue;
      //     }
      //     if (basta) {
      //       outs() << "correggo solamente il primo uso" << "\n";
      //       continue;
      //     }

      //     use->set(NewInst);
      //     basta = true;
      //   }

      //// Soluzione 1.5:
      // posso usare user->setOperand() come in "LLVM for gradstudents" e non
      // preoccuparmi di copiare

      //// Soluzione 2:
      // oppure posso usare questo algoritmo apposito in modo da non dovermi
      // preoccupare di copiare gli usi prima di modificarli
      bool basta = false;
      Inst1st.replaceUsesWithIf(NewInst, [&](Use &use) {
        if (use.getUser() == NewInst)
          return false;
        if (basta)
          return false;

        basta = true;
        return true;
      });

      // Proviamo anche ad usare il builder!

      // Insert at the point where 'Inst1st' appears.
      // La nuova istruzione si ritrova una posizione indietro rispetto alla
      // vecchia
      IRBuilder<> builder(&Inst1st);
      Value *lhs = Inst1st.getOperand(0);
      Value *rhs = Inst1st.getOperand(1);
      // crea e inserisce la nuova istruzione
      Value *mul = builder.CreateSDiv(lhs, rhs);
    }

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};
} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------

llvm::PassPluginLibraryInfo getLocalOptsPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LocalOptsPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "local-opts-pass") {
                    FPM.addPass(LocalOptsPass());
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
