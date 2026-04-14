# Questa variabile è il modo idiomatico per gestire le librerie LLVM (distinte
# da quelle MLIR). 'LLVM_LINK_COMPONENTS' viene letta da 'add_llvm_executable()'
# - usa 'set(LLVM_LINK_COMPONENTS ...)' per tutto ciò che inizia con llvm::
# - usa 'target_link_libraries(...)' per tutto ciò che inizia con mlir::
set(LLVM_LINK_COMPONENTS
  Core
  Support
  nativecodegen
  OrcJIT
#   ExecutionEngine
)

set(LIBS
    MLIRSupport
    # MLIRArithDialect
    # MLIROptLib
    MLIRRegisterAllExtensions
    MLIRRegisterAllPasses

    # roba per lowering a llvm
    MLIRLLVMDialect
    MLIRTargetLLVMIRExport
    MLIRBuiltinToLLVMIRTranslation
    MLIRLLVMToLLVMIRTranslation
    MLIRExecutionEngine

    # includiamo la libreria creata in lib/ con le sue dipendenze PUBLIC
    MLIRToy
  )

# version llvm di add_executable per aggiungere un executable target
add_llvm_executable(toy
    toyc.cpp
    frontend/AST.cpp
    frontend/MLIRGen.cpp
)
# applichiamo i vari flag visti nel root CML qua
llvm_update_compile_flags(toy)
# linkiamo con le dipendenze
target_link_libraries(toy PRIVATE ${LIBS})
# check di mlir per l'eseguibile appena creato (don't care).
mlir_check_all_link_libraries(toy)
