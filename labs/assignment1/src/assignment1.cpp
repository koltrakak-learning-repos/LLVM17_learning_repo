#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <cmath>
#include <cstdint>
#include <tuple>

using namespace llvm;

namespace {

bool isPowerOfTwo(int64_t n) {
  // Le potenze di 2 sono positive
  if (n <= 0)
    return false;

  return (n & (n - 1)) == 0;
}

Value *getIdentityOperand(BinaryOperator *op) {
  Value *LHS = op->getOperand(0);
  Value *RHS = op->getOperand(1);
  auto *ConstLHS = dyn_cast<ConstantInt>(LHS);
  auto *ConstRHS = dyn_cast<ConstantInt>(RHS);

  switch (op->getOpcode()) {
  case Instruction::Add:
    if (ConstLHS && ConstLHS->isZero())
      return RHS;
    if (ConstRHS && ConstRHS->isZero())
      return LHS;
  case Instruction::Mul:
    if (ConstLHS && ConstLHS->isOne())
      return RHS;
    if (ConstRHS && ConstRHS->isOne())
      return LHS;
  default:
    break;
  }
  return nullptr;
}

struct AlgIdentityPass : PassInfoMixin<AlgIdentityPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    for (BasicBlock &B : F) {
      for (auto IIter = B.begin(); IIter != B.end();) {
        Instruction &I = *IIter;
        // spostiamo subito l'iteratore al prossimo elemento dato che il
        // l'istruzione corrente potrebbe essere eliminata (rompendo così
        // l'iteratore)
        IIter++;

        auto *op = dyn_cast<BinaryOperator>(&I);
        if (!op) {
          continue;
        }

        if (auto *operand = getIdentityOperand(op)) {
          I.replaceAllUsesWith(operand);
          // devo fare attenzione a rimuovere istruzioni dato che rompo
          // l'iteratore. Ho già incrementato e quindi sono al sicuro
          I.removeFromParent();
        }
      }
    }

    return PreservedAnalyses::all();
  }
}; // AlgIdentityPass

void substituteWithShiftAndSums(Instruction *I, Value *varOperand,
                                int shiftValue, int remainder, bool shiftLeft) {
  LLVMContext &ctx = I->getContext();
  Type *int32Ty = Type::getInt32Ty(ctx);
  Constant *c = ConstantInt::get(int32Ty, shiftValue);
  // attenzione ad usare lo shift destro aritmetico dato che possiamo star
  // dividendo un numero negativo
  Instruction::BinaryOps binopKind =
      shiftLeft ? Instruction::Shl : Instruction::AShr;
  Instruction *shiftInst = BinaryOperator::Create(binopKind, varOperand, c);

  shiftInst->insertAfter(I);

  binopKind = shiftLeft ? Instruction::Add : Instruction::Sub;
  Instruction *prevInst = shiftInst;
  // FIXME: questa logica è stupida e la dovrei correggere. Dovrei controllare
  // se mi conviene sommare o sottrarre invece di decidere a priori.
  // Ad esempio: %4 = sdiv i32 15, %1; dovrebbe tradursi in uno shift a destra
  // di 4 più una somma invece di uno shift di 3 più 7 sottrazioni
  while (remainder > 0) {
    Instruction *addInst =
        BinaryOperator::Create(binopKind, prevInst, varOperand);
    addInst->insertAfter(prevInst);

    prevInst = addInst;
    remainder--;
  }

  I->replaceAllUsesWith(prevInst);
  I->removeFromParent();
}

std::tuple<int, int, bool> getBestShiftValueAndRemainder(uint64_t constantValue,
                                                         int maxRemainder) {
  // calcoliamo shiftValue come "logaritmo intero" della costante
  int shiftValue = static_cast<int>(std::log2(constantValue));
  int remainder = constantValue - (1 << shiftValue);

  bool skip = false;
  if (remainder > maxRemainder)
    skip = true;

  return std::make_tuple(shiftValue, remainder, skip);
}

bool reduceStrength(Instruction *I, Value *varOperand, uint64_t constantValue,
                    int maxRemainder, bool shiftLeft) {
  auto [shiftValue, remainder, skip] =
      getBestShiftValueAndRemainder(constantValue, maxRemainder);
  if (skip) {
    errs() << "con un resto di " << remainder
           << " non conviene applicare strength reduction\n";
    return false;
  }

  const char *dir = shiftLeft ? "sinistra" : "destra";
  const char *remainderOp = shiftLeft ? "somme" : "sottrazioni";
  errs() << "shifto di " << shiftValue << " a " << dir << " e aggiungo "
         << remainder << " " << remainderOp << "\n";

  substituteWithShiftAndSums(I, varOperand, shiftValue, remainder, shiftLeft);
  return true;
}

struct StrengthReductionPass : PassInfoMixin<StrengthReductionPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    for (BasicBlock &B : F) {
      // NB: uso un range che incrementa l'iteratore subito, in questo modo
      // posso cancellare le istruzioni senza problemi
      for (Instruction &I : make_early_inc_range(B)) {
        if (auto *op = dyn_cast<BinaryOperator>(&I)) {
          bool shiftLeft;
          int maxRemainder;
          switch (op->getOpcode()) {
          case Instruction::Mul:
            shiftLeft = true;
            maxRemainder = 3;
            break;
          case Instruction::UDiv:
            shiftLeft = false;
            // div sono costose, conviene aggiungere anche tante somme
            maxRemainder = 10;
            break;
          case Instruction::SDiv:
            shiftLeft = false;
            maxRemainder = 10;
            break;
          default:
            // binop a cui non posso applicare strength reduction
            continue;
          }

          Value *lhs = op->getOperand(0);
          Value *rhs = op->getOperand(1);
          ConstantInt *lhsConstant = dyn_cast<ConstantInt>(lhs);
          ConstantInt *rhsConstant = dyn_cast<ConstantInt>(rhs);

          // NB: assumo che le costanti siano positive per semplificarmi la
          // vita.
          if (lhsConstant) {
            reduceStrength(&I, rhs, lhsConstant->getZExtValue(), maxRemainder,
                           shiftLeft);
          } else if (rhsConstant) {
            reduceStrength(&I, lhs, rhsConstant->getZExtValue(), maxRemainder,
                           shiftLeft);
          } else {
            // non ho un argomento costante e quindi non posso applicare
            // strenght reduction
          }
        }
      }
    }

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // StrengthReductionPass

} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------

llvm::PassPluginLibraryInfo getLocalOptsPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AlgIdentityPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "alg-identity-pass") {
                    FPM.addPass(AlgIdentityPass());
                    return true;
                  }

                  if (Name == "strength-reduction-pass") {
                    FPM.addPass(StrengthReductionPass());
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
