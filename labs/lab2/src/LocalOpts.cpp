#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/Module.h"

#include <string>

using namespace llvm;

namespace
{
    struct LocalOptsPass : PassInfoMixin<LocalOptsPass>
    {

        PreservedAnalyses run(Function &F, FunctionAnalysisManager &)
        {

            for (auto Iter = F.begin(); Iter != F.end(); ++Iter)
            {
                BasicBlock &B = *Iter;
                // Preleviamo le prime due istruzioni del BB
                Instruction &Inst1st = *B.begin();
                Instruction &Inst2nd = *(++B.begin());

                // L'indirizzo della prima istruzione deve essere uguale a quello del
                // primo operando della seconda istruzione (per costruzione dell'esempio)
                assert(&Inst1st == Inst2nd.getOperand(0));

                // Stampa la prima istruzione
                outs() << "PRIMA ISTRUZIONE: " << Inst1st << "\n";
                // Stampa la prima istruzione come operando
                outs() << "COME OPERANDO: ";
                Inst1st.printAsOperand(outs(), false);
                outs() << "\n";

                // User-->Use-->Value
                outs() << "I MIEI OPERANDI SONO:\n";
                for (auto Iter = Inst1st.op_begin(); Iter != Inst1st.op_end(); ++Iter)
                {
                    Value *Operand = *Iter;

                    if (Argument *Arg = dyn_cast<Argument>(Operand))
                    {
                        outs() << "\t" << *Arg << ": SONO L'ARGOMENTO N. " << Arg->getArgNo()
                               << " DELLA FUNZIONE " << Arg->getParent()->getName()
                               << "\n";
                    }
                    if (ConstantInt *C = dyn_cast<ConstantInt>(Operand))
                    {
                        outs() << "\t" << *C << ": SONO UNA COSTANTE INTERA DI VALORE " << C->getValue()
                               << "\n";
                    }
                }

                outs() << "LA LISTA DEI MIEI USERS:\n";
                for (auto Iter = Inst1st.user_begin(); Iter != Inst1st.user_end(); ++Iter)
                {
                    // outs() << "\t" << *(dyn_cast<Instruction>(*Iter)) << "\n";
                    User *U = *Iter;
                    outs() << "\t" << *U << "\n";
                }

                // Qual è la differenza tra gli USERS e gli USES?
                // Prova a vedere cosa succede se in Foo.ll la seconda istruzione diventa
                // %4 = mul nsw i32 %3, %3
                outs() << "E DEI MIEI USI:\n";
                for (auto Iter = Inst1st.use_begin(); Iter != Inst1st.use_end(); ++Iter)
                {
                    // outs() << "\t" << *(dyn_cast<Instruction>(Iter->getUser())) << "\n";
                    Use &U = *Iter;
                    outs() << "\t" << *U.getUser() << "@operand" << U.getOperandNo() << "\n";
                }

                // Manipolazione delle istruzioni
                Instruction *NewInst = BinaryOperator::Create(
                    Instruction::Add, Inst1st.getOperand(0), Inst1st.getOperand(0));

                NewInst->insertAfter(&Inst1st);
                // Si possono aggiornare le singole references separatamente?
                // Controlla la documentazione e prova a rispondere.
                Inst1st.replaceAllUsesWith(NewInst);
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

llvm::PassPluginLibraryInfo getLocalOptsPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION,
        "LocalOptsPass",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if (Name == "local-opts-pass")
                    {
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
llvmGetPassPluginInfo()
{
    return getLocalOptsPassPluginInfo();
}
