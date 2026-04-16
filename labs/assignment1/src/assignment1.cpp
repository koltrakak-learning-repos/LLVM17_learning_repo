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
                                int shiftValue, int remainder) {
  LLVMContext &ctx = I->getContext();
  Type *int32Ty = Type::getInt32Ty(ctx);
  Constant *c = ConstantInt::get(int32Ty, shiftValue);
  Instruction *shiftInst =
      BinaryOperator::Create(Instruction::Shl, varOperand, c);
  shiftInst->insertAfter(I);

  Instruction *prevInst = shiftInst;
  while (remainder > 0) {
    Instruction *addInst =
        BinaryOperator::Create(Instruction::Add, varOperand, prevInst);
    addInst->insertAfter(prevInst);

    prevInst = addInst;
    remainder--;
  }

  I->replaceAllUsesWith(prevInst);
  I->removeFromParent();
}

std::tuple<int, int, bool> getBestShiftValueAndRemainder(int constantValue,
                                                         int maxRemainder = 1) {
  // calcoliamo shiftValue come "logaritmo intero" della costante
  int shiftValue = static_cast<int>(std::log2(constantValue));
  int remainder = constantValue - (1 << shiftValue);

  bool skip = false;
  if (remainder > maxRemainder)
    skip = true;

  return std::make_tuple(shiftValue, remainder, skip);
}

bool reduceStrength(Instruction *I, Value *varOperand, int constantValue,
                    int maxRemainder = 1) {
  auto [shiftValue, remainder, skip] =
      getBestShiftValueAndRemainder(constantValue, maxRemainder);
  if (skip) {
    outs() << "con un resto di " << remainder
           << " non conviene sostituire la moltiplicazione con uno "
              "shift\n";
    return false;
  }

  outs() << "shifto di " << shiftValue << " e aggiungo " << remainder
         << " somme\n";

  // NB: qua faccio un cast statico dato che questa funzione deve essere
  // chiamata solamente quando a monte si è è già confermato che l'istruzione
  // sia un binop
  switch (auto opCode = cast<BinaryOperator>(I)->getOpcode()) {
  case Instruction::Mul:
    substituteWithShiftAndSums(I, varOperand, shiftValue, remainder);
    break;
  case Instruction::UDiv:
    substituteWithShiftAndSums(I, varOperand, shiftValue, remainder);
    break;
  case Instruction::SDiv:
    substituteWithShiftAndSums(I, varOperand, shiftValue, remainder);
    break;
  default:
    outs() << "questo non sarebbe dovuto succedere..., come ci è arrivato qua: "
           << opCode;
    return false;
  }
  return true;
}

struct StrengthReductionPass : PassInfoMixin<StrengthReductionPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    for (BasicBlock &B : F) {
      // NB: uso un range che incrementa l'iteratore subito, in questo modo
      // posso cancellare le istruzioni senza problemi
      for (Instruction &I : make_early_inc_range(B)) {
        if (auto *op = dyn_cast<BinaryOperator>(&I);
            op && op->getOpcode() == Instruction::Mul) {
          outs() << "trovata una moltiplicazione: " << *op << "\n";

          Value *lhs = op->getOperand(0);
          Value *rhs = op->getOperand(1);
          ConstantInt *lhsConstant = dyn_cast<ConstantInt>(lhs);
          ConstantInt *rhsConstant = dyn_cast<ConstantInt>(rhs);

          if (lhsConstant) {
            reduceStrength(&I, rhs, lhsConstant->getSExtValue(), 10);
          } else if (rhsConstant) {
            reduceStrength(&I, lhs, rhsConstant->getSExtValue(), 10);
          } else {
            outs() << "... ma non aveva una potenza di due come argomento "
                      ":(\n";
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
