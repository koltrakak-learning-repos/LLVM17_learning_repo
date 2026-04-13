# Crea una libreria (libMLIRToy.a) con tutto cio che definisce il dialetto
add_mlir_dialect_library(MLIRToy
        # StandaloneTypes.cpp
        ToyDialect.cpp
        ToyOps.cpp
        transforms/ToyCombine.cpp
        transforms/ShapeInferencePass.cpp
        transforms/LowerToAffineLoops.cpp

        ADDITIONAL_HEADER_DIRS
        ${PROJECT_SOURCE_DIR}/include/toy
        # definiamo come dipendenze i target che generano gli header a partire
        # dai .td. Questo va a definire un ordine di compilazione, e permette
        # di rilevare modifiche ai file .td da cui si dipende.
        DEPENDS
        MLIRToyIncGen
        # MLIRStandalonePassesIncGen

        # PUBLIC: chiunque userà la libreria (include la libreria in
        # target_link_libraries()) erediterà automaticamente anche il link
        # a queste librerie.
        LINK_LIBS PUBLIC
        # questi non sono sempre facili da trovare, guarda i link errors e
        # fai come ti ha insegnato il russo
        MLIRIR
        MLIRParser
        MLIRFunctionInterfaces
        MLIRInferTypeOpInterface
        # MLIRRegisterAllDialects
        # MLIRFuncDialect
        )

