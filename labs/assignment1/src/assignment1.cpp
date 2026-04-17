#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <cmath>
#include <cstdint>
#include <tuple>
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

          // NB: ottimizzo solo costanti positive per semplificarmi la vita.
          if (lhsConstant) {
            if (lhsConstant->getSExtValue() < 0) {
              errs() << "non voglio fare strength reduction di costanti "
                        "negative\n";
              continue;
            }

            reduceStrength(&I, rhs, lhsConstant->getSExtValue(), maxRemainder,
                           shiftLeft);
          } else if (rhsConstant) {
            if (rhsConstant->getSExtValue() < 0) {
              errs() << "non voglio fare strength reduction di costanti "
                        "negative\n";
              continue;
            }

            reduceStrength(&I, lhs, rhsConstant->getSExtValue(), maxRemainder,
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

// Restituisco l'operando variabile, il valore dell'operando costante e un flag
// di errore che mi dice se la binop aveva un operando costante o meno (nella
// posizione giusta in caso di sottrazione)
std::tuple<Value *, int64_t, bool> parseBinopOperands(BinaryOperator *op) {
  Value *lhs = op->getOperand(0);
  Value *rhs = op->getOperand(1);
  auto *lhsConstant = dyn_cast<ConstantInt>(lhs);
  auto *rhsConstant = dyn_cast<ConstantInt>(rhs);

  if (lhsConstant) {
    return std::make_tuple(rhs, lhsConstant->getSExtValue(), true);

  } else if (rhsConstant) {
    return std::make_tuple(lhs, rhsConstant->getSExtValue(), true);

  } else {
    return std::make_tuple(nullptr, 0, false);
  }
}

bool computeSimplificationCondition(BinaryOperator::BinaryOps useeBinopOpcode,
                                    BinaryOperator::BinaryOps userBinopOpcode,
                                    int64_t useeConstantValue,
                                    int64_t userConstantValue) {

  bool simplificationCondition = false;

  // doppio switch tremendo che controlla le costanti di usee e user per vedere
  // se posso applicare una semplificazione.
  // NB: le sottrazioni sono problematiche in quanto non commutative (il segno
  // della costante cambia in base a se è operando destro o sinistro). Oltre al
  // valore della costante, dovrei tenere traccia anche dell'operando di
  // provenienza. Per adesso ASSUMO CHE LA COSTANTE SIA SEMPRE A DESTRA
  // (canonicalizzazione immaginaria)
  switch (userBinopOpcode) {
  case Instruction::Add:
    switch (useeBinopOpcode) {
    case Instruction::Add:
      simplificationCondition = useeConstantValue == -userConstantValue;
      break;
    case Instruction::Sub:
      simplificationCondition = useeConstantValue == userConstantValue;
      break;
    default:
      simplificationCondition = false;
    }
    break;

  case Instruction::Sub:
    switch (useeBinopOpcode) {
    case Instruction::Add:
      simplificationCondition = useeConstantValue == userConstantValue;
      break;
    case Instruction::Sub:
      simplificationCondition = useeConstantValue == -userConstantValue;
      break;
    default:
      simplificationCondition = false;
    }
    break;
  default:
    simplificationCondition = false;
  }

  return simplificationCondition;
}

struct MultiInstructionPass : PassInfoMixin<MultiInstructionPass> {

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {

    std::vector<BinaryOperator *> toDelete = {};

    for (BasicBlock &B : F) {
      for (Instruction &I : B) {
        errs() << "sono" << I << "\n";

        // posso semplificare solamente determinate categoria di binop
        if (auto *useeBinop = dyn_cast<BinaryOperator>(&I)) {

          switch (useeBinop->getOpcode()) {
          case Instruction::Add:
            break;
          case Instruction::Sub:
            break;
          default:
            // binop usee che non considero per semplificazioni
            continue;
          }

          auto [useeVarOperand, useeConstantValue, hasConstant] =
              parseBinopOperands(useeBinop);
          if (!hasConstant)
            // senza un argomento costante la binop usee non può portare a
            // semplificazioni
            continue;

          for (User *user : I.users()) {
            if (auto *userBinop = dyn_cast<BinaryOperator>(user)) {
              auto [_, userConstantValue, hasConstant] =
                  parseBinopOperands(userBinop);
              if (!hasConstant)
                // se lo user non ha una costante, non posso semplificare
                continue;

              bool simplificationCondition = computeSimplificationCondition(
                  useeBinop->getOpcode(), userBinop->getOpcode(),
                  useeConstantValue, userConstantValue);

              // controlliamo se posso eliminare l'istruzione user in quanto si
              // semplifica ad un alias dello usee
              if (simplificationCondition) {
                errs() << "semplifico" << *userBinop << "\n";
                userBinop->replaceAllUsesWith(useeVarOperand);
                // NB: non posso fare subito 'userBinop->removeFromParent()'
                // dato che romperei l'iteratore delle istruzioni; stavolta non
                // posso fare neanche incrementare subito l'iteratore dato che
                // non sto cancellando l'istruzione corrente, ma uno user che
                // viene dopo nell'ir. Mi salvo quindi le istruzioni da
                // eliminare in un vettore, e le cancello alla fine
                toDelete.push_back(userBinop);
              }
            }
          }
        }
      }
    }

    for (auto *binop : toDelete)
      binop->eraseFromParent();

    return PreservedAnalyses::all();
  }
  // Without isRequired returning true, this pass will be skipped for
  // functions decorated with the optnone LLVM attribute. Note that clang
  // -O0 decorates all functions with optnone.
  static bool isRequired() { return true; }
}; // MultiInstructionPass

} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------

llvm::PassPluginLibraryInfo getLocalOptsPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Assignment1Passes", LLVM_VERSION_STRING,
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

                  if (Name == "multi-instruction-pass") {
                    FPM.addPass(MultiInstructionPass());
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
