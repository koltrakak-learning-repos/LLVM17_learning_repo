#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

/// This pass illustrates the IR nesting through printing.
struct TestPrintNestingPass
    : public mlir::PassWrapper<TestPrintNestingPass, mlir::OperationPass<>>
{
    MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TestPrintNestingPass)

    mlir::StringRef getArgument() const final { return "test-print-nesting"; }
    mlir::StringRef getDescription() const final { return "Test various printing."; }
    // Entry point for the pass.
    void runOnOperation() override
    {
        mlir::Operation *op = getOperation();
        resetIndent();
        printOperation(op);
    }

    /// The three methods below are mutually recursive and follow the nesting of
    /// the IR: operation->region->block->operation->...

    void printOperation(mlir::Operation *op)
    {
        // Print the operation itself and some of its properties
        printIndent() << "visiting op: '" << op->getName() << "' with "
                      << op->getNumOperands() << " operands and "
                      << op->getNumResults() << " results\n";
        // Print the operation attributes
        if (!op->getAttrs().empty())
        {
            printIndent() << op->getAttrs().size() << " attributes:\n";
            for (mlir::NamedAttribute attr : op->getAttrs())
                printIndent() << " - '" << attr.getName().getValue() << "' : '"
                              << attr.getValue() << "'\n";
        }

        // Recurse into each of the regions attached to the operation.
        printIndent() << " " << op->getNumRegions() << " nested regions:\n";
        auto indent = pushIndent();
        for (mlir::Region &region : op->getRegions())
            printRegion(region);
    }

    void printRegion(mlir::Region &region)
    {
        // A region does not hold anything by itself other than a list of blocks.
        printIndent() << "Region with " << region.getBlocks().size()
                      << " blocks:\n";
        auto indent = pushIndent();
        for (mlir::Block &block : region.getBlocks())
            printBlock(block);
    }

    void printBlock(mlir::Block &block)
    {
        // Print the block intrinsics properties (basically: argument list)
        printIndent()
            << "Block with " << block.getNumArguments() << " arguments, "
            << block.getNumSuccessors()
            << " successors, and "
            // Note, this `.size()` is traversing a linked-list and is O(n).
            << block.getOperations().size() << " operations\n";

        // Block main role is to hold a list of Operations: let's recurse.
        auto indent = pushIndent();
        for (mlir::Operation &op : block.getOperations())
            printOperation(&op);
    }

    /// Manages the indentation as we traverse the IR nesting.
    int indent;
    struct IdentRAII
    {
        int &indent;
        IdentRAII(int &indent) : indent(indent) {}
        ~IdentRAII() { --indent; }
    };
    void resetIndent() { indent = 0; }
    IdentRAII pushIndent() { return IdentRAII(++indent); }

    llvm::raw_ostream &printIndent()
    {
        for (int i = 0; i < indent; ++i)
            llvm::outs() << "  ";
        return llvm::outs();
    }
};

void registerTestPrintNestingPass()
{
    // Qua sto istanziando un oggetto il cui costruttore chiama automaticamente mlir::registerPass()
    // nel registro globale dei passi. Questo registro è una mappa nome → funzione factory che vive
    // per tutta la durata del processo. Il nome usato è quello che hai definito nel tuo passo tramite
    // il metodo getArgument().
    // Inoltre, questo costrutture registra automaticamente un'opzione CLI con il nomde del passo.
    // I passi registrati vanno anche invocati, e per farlo bisogna passare un flag. Nel caso del
    // singolo passo, --pass-pipeline="test-print-nesting" e --test-print-nesting sono equivalenti
    mlir::PassRegistration<TestPrintNestingPass>();
}

/***  PASS DRIVER ***/

#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "mlir/InitAllDialects.h"

int main(int argc, char **argv)
{
    mlir::DialectRegistry registry;
    mlir::registerAllDialects(registry);

    registerTestPrintNestingPass();

    // qua chiamiamo mlir opt
    return mlir::asMainReturnCode(mlir::MlirOptMain(argc, argv, "Test MLIR pass driver\n", registry));
}
