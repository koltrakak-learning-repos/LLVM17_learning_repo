# questa macro è un po' troppo opinionated:
# - parte sempre a generare le ops, poi fa dialect e types
# - ma così facendo mi scasina i nomi dei file
# add_mlir_dialect(ToyDialect toy)

# Posso fare più esplicitametne così
set(LLVM_TARGET_DEFINITIONS ToyDialect.td)
# mlir_tablegen(Ops.h.inc -gen-op-decls)
# mlir_tablegen(Ops.cpp.inc -gen-op-defs)
mlir_tablegen(ToyDialect.h.inc -gen-dialect-decls)
mlir_tablegen(ToyDialect.cpp.inc -gen-dialect-defs)
add_public_tablegen_target(MLIRToyDialectIncGen)

# Possiamo continuare a generare la documentazione come prima.
# add_mlir_doc: Prende le summary e description che hai scritto per le tue
# operazioni e genera dei file Markdown (.md).
add_mlir_doc(ToyDialect ToyDialect Toy/ -gen-dialect-doc)
# add_mlir_doc(StandaloneOps StandaloneOps Standalone/ -gen-op-doc)
