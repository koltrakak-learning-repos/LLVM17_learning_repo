set(LIBS
    MLIRSupport
    # MLIRArithDialect
    # MLIROptLib
    MLIRRegisterAllExtensions
    MLIRRegisterAllPasses
    # # includiamo la libreria creata in lib/ con le sue dipendenze PUBLIC
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
