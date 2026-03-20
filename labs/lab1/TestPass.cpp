//=============================================================================
// FILE:
//    TestPass.cpp
//
// DESCRIPTION:
//    Visits all functions in a module and prints their names. Strictly speaking,
//    this is an analysis pass (i.e. //    the functions are not modified). However,
//    in order to keep things simple there's no 'print' method here (every analysis
//    pass should implement it).
//
// USAGE:
//    New PM
//      opt -load-pass-plugin=<path-to>libTestPass.so -passes="test-pass" `\`
//        -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

using namespace llvm;

//-----------------------------------------------------------------------------
// TestPass implementation
//-----------------------------------------------------------------------------
// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace
{
    // New PM implementation
    struct TestPass : PassInfoMixin<TestPass>
    {
        // Main entry point, takes IR unit to run the pass on (&F) and the
        // corresponding pass manager (to be queried if need be)
        //
        // ------------------
        //
        // KKoltraka: questo metodo del passo viene invocato
        // ad ogni funzione
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &)
        {
            errs() << F.getName() << "\n\t";
            errs() << "number of arguments: " << F.arg_size();
            if (F.isVarArg())
                errs() << "+*";
            errs() << "\n\t";
            errs() << "number of basic blocks: " << F.size() << "\n\t";
            errs() << "number of instructions: " << F.getInstructionCount() << "\n\t";

            int callCount = 0;
            for (BasicBlock &BB : F)
            {
                for (Instruction &I : BB)
                {
                    // Verifica se l'istruzione è una chiamata a funzione
                    if (auto *Call = dyn_cast<CallInst>(&I))
                    {
                        // controllo se la chiamata è verso una funzione che è definita
                        // nel modulo corrente
                        Function *CalledF = Call->getCalledFunction();
                        if (!CalledF->isDeclaration())
                        {
                            callCount++;
                        }
                    }
                }
            }
            errs() << "number of inter-module calls: " << callCount << "\n";

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
llvm::PassPluginLibraryInfo getTestPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION,
        "TestPass",
        LLVM_VERSION_STRING,
        // KKoltraka: questo mi sembra serva a registrare il passo
        // quando invochiamo: 'opt -passes=test-pass'
        [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if (Name == "test-pass")
                    {
                        FPM.addPass(TestPass());
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
    return getTestPassPluginInfo();
}
