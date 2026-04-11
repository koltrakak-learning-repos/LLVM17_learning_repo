# questa macro è un po' troppo opinionated:
# - parte sempre a generare le ops, poi fa dialect e types
# - ma così facendo mi scasina i nomi dei file
# add_mlir_dialect(ToyDialect toy)

# Posso fare più esplicitamente così (ricorda che ops include tutto)
set(LLVM_TARGET_DEFINITIONS ToyOps.td)
mlir_tablegen(ToyDialect.h.inc -gen-dialect-decls)
mlir_tablegen(ToyDialect.cpp.inc -gen-dialect-defs)
mlir_tablegen(ToyOps.h.inc -gen-op-decls)
mlir_tablegen(ToyOps.cpp.inc -gen-op-defs)

set(LLVM_TARGET_DEFINITIONS ToyCombine.td)
mlir_tablegen(ToyCombine.cpp.inc --gen-rewriters)

set(LLVM_TARGET_DEFINITIONS ShapeInferenceInterface.td)
mlir_tablegen(ShapeInferenceOpInterfaces.h.inc -gen-op-interface-decls)
mlir_tablegen(ShapeInferenceOpInterfaces.cpp.inc -gen-op-interface-defs)

# set(LLVM_TARGET_DEFINITIONS Passes.td)
# mlir_tablegen(Passes.h.inc --gen-pass-decls)

add_public_tablegen_target(MLIRToyIncGen)

# Possiamo continuare a generare la documentazione come prima.
# add_mlir_doc: Prende le summary e description che hai scritto per le tue
# operazioni e genera dei file Markdown (.md).
add_mlir_doc(ToyDialect ToyDialect Toy/ -gen-dialect-doc)
add_mlir_doc(ToyOps ToyOps Toy/ -gen-op-doc)
