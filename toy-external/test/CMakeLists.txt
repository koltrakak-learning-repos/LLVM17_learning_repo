llvm_canonicalize_cmake_booleans(
        MLIR_ENABLE_BINDINGS_PYTHON
)

# ricorda: CURRENT_SOURCE_DIR è la directory del cml corrente, SOURCE_DIR sarebbe
# quella del root cml immagino. CURRENT_BINARY_DIR è la directory corrispondente
# a quella corrente dentro a build/
#
# LIT ha bisogno di sapere dove si trovano i tuoi binari (come standalone-opt)
# con cui eseguire i test. Utilizziamo vari file di config che settiamo qua:
# - usiamo un template (.in) che contiene segnaposto come @STANDALONE_TOOLS_DIR@.
#   da cui creiamo un file reale nella cartella di build dove quei segnaposto
#   vengono sostituiti con i percorsi assoluti
# - da li invochiamo la configurazione principale lit.cfg.py
configure_lit_site_cfg(
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
        ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
        MAIN_CONFIG
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

# dipendenze che devono esserci prima di lanciare i test
set(TOY_TEST_DEPENDS
        FileCheck count not
        toy
    )

# if(NOT WIN32)
#   list(APPEND STANDALONE_TEST_DEPENDS StandalonePlugin)
# endif()
# if(MLIR_ENABLE_BINDINGS_PYTHON)
#   list(APPEND STANDALONE_TEST_DEPENDS StandalonePythonModules)
# endif()

# definiamo il target per lanciare i test
add_lit_testsuite(check-toy "Running the toy regression tests"
        ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${TOY_TEST_DEPENDS}
    )

# questo infila il target in una "target-folder" chiamata Tests (don't care)
set_target_properties(check-toy PROPERTIES FOLDER "Tests")
# questa è altra pulizia che non mi interessa
add_lit_testsuites(TOY ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${TOY_TEST_DEPENDS})

# don't care dei test di CAPI
# add_subdirectory(CAPI)
