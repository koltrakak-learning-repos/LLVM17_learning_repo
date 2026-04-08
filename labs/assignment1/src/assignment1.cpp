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
    break;
  case Instruction::Mul:
    if (ConstLHS && ConstLHS->isOne())
      return RHS;
    if (ConstRHS && ConstRHS->isOne())
      return LHS;
    break;
  default:
    break;
  }
  return nullptr;
}

struct AlgIdentityPass : PassInfoMixin<AlgIdentityPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    for (BasicBlock &B : F) {
      bool incrementIterator = true;
      for (auto IIter = B.begin(); IIter != B.end();) {
        Instruction &I = *IIter;
        // spostiamo subito l'iteratore al prossimo elemento dato che il
        // prossimo blocco potrebbe eliminare delle istruzioni
        IIter++;

        auto *op = dyn_cast<BinaryOperator>(&I);
        if (!op) {
          continue;
        }

        if (auto *operand = getIdentityOperand(op)) {
          I.replaceAllUsesWith(operand);
          // devo fare attenzione a rimuover istruzioni dato che altrimenti
          // rompo l'iteratore. Ho già incrementato e quindi sono al sicuro
          I.removeFromParent();
        }
      }
    }

    return PreservedAnalyses::all();
  }

  // mul by power of 2 strength reductio}n
  PreservedAnalyses run2(Function &F, FunctionAnalysisManager &) {

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
                static_cast<int>(std::sqrt(lhsConstant->getSExtValue()));
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

            int shift_value = std::sqrt(rhsConstant->getSExtValue());
            outs() << "shifto di " << shift_value << "\n";

            LLVMContext &ctx = I.getContext();
            Type *int32Ty = Type::getInt32Ty(ctx);
            Constant *c = ConstantInt::get(int32Ty, shift_value);

            Instruction *shiftInst =
                BinaryOperator::Create(Instruction::Shl, lhs, c);
            shiftInst->insertAfter(&I);
            I.replaceAllUsesWith(shiftInst);
          } else {
            outs() << "... ma non aveva una potenza di due come argomento "
                      ":(\n";
            continue;
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
};
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
