cmake_minimum_required(VERSION 3.20.0)
project(toy-external LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # per intellisense

set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON) # roba per macOS a quanto pare

# c++17 a default ma è anche parametrizzabile con -D
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to conform to")

set(LIT_PATH /home/kevin/llvm-project/build/bin/llvm-lit)
set(LLVM_EXTERNAL_LIT ${LIT_PATH} CACHE STRING "path per llvm-lit")
message(STATUS "Using llvm-lit in: ${LIT_PATH}")

### INCLUDIAMO LE INFORMAZIONI DI BUILD ESPORTATE DA LLVM E MLIR ###


# controlliamo se stiamo facendo una build out-of-tree o meno
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(EXTERNAL_PROJECT_BUILD FALSE)
  # carichiamo le informazioni di mlir e (transitivamente) di llvm.
  # in particolare:
  # - popoliamo le variabili MLIR_CMAKE_DIR e LLVM_CMAKE_DIR dove
  #   sono state esportate le informazioni di build (/usr/local/lib/cmake/...)
  # - popoliamo le variabili LLVM_INCLUDE_DIRS e MLIR_INCLUDE_DIRS dove sono
  #   presenti gli header
  find_package(MLIR REQUIRED CONFIG)

  # Define the default argument to use by `lit` when testing.
  set(LLVM_LIT_ARGS "-sv" CACHE STRING "Default options for lit")

  message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  # Definizione delle directory di output, non so chi utilizza queste variabili,
  # ma qualcuno lo farà. (CMAKE_BINARY_DIR è una variabile builtin, settata da
  # project(), che punta alla root della cartella di build)
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
  set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})

  # CMAKE_MODULE_PATH è una lista di percorsi dove CMake va a cercare i file con
  # estensione cmake. Aggiungendo questi percorsi al CMAKE_MODULE_PATH, permetti
  # al comando include() di trovare script specifici (come quelli per TableGen).
  list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

  # include() cerca un file chiamato NomeFile.cmake nel CMAKE_MODULE_PATH e lo
  # esegue. I file inclusi contengono definizioni di funzioni e configurazioni.
  # Ad esempio, include(AddMLIR) definisce la funzione add_mlir_dialect(...).
  # Le configurazioni includono variabili e quant'altro (non mi interessa)
  include(TableGen)             # Permette di usare add_public_tablegen_target
  include(AddLLVM)              # Fornisce macro come add_llvm_library
  include(AddMLIR)              # Fornisce macro come add_mlir_dialect
  include(HandleLLVMOptions)    # Configura i flag del compilatore (warning, ottimizzazioni) in linea con LLVM
else()
  set(EXTERNAL_PROJECT_BUILD TRUE)
  # Build via external projects mechanism
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/tools/Standalone/lib)

  set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})
  set(MLIR_MAIN_SRC_DIR ${LLVM_MAIN_SRC_DIR}/../mlir)
  set(MLIR_INCLUDE_DIR ${MLIR_MAIN_SRC_DIR}/include)
  set(MLIR_GENERATED_INCLUDE_DIR ${LLVM_BINARY_DIR}/tools/mlir/include)
  set(MLIR_INCLUDE_DIRS "${MLIR_INCLUDE_DIR};${MLIR_GENERATED_INCLUDE_DIR}")

  list(APPEND CMAKE_MODULE_PATH "${MLIR_MAIN_SRC_DIR}/cmake/modules")
  list(APPEND CMAKE_MODULE_PATH "${LLVM_MAIN_SRC_DIR}/cmake/modules")
endif()

if(MLIR_ENABLE_BINDINGS_PYTHON)
  include(MLIRDetectPythonEnv)
  mlir_configure_python_dev_packages()
endif()


### SETTIAMO DOVE CERCARE HEADER E LIBRERIE ###


# Qui stai creando degli "alias" per i percorsi del tuo progetto:
# - PROJECT_SOURCE_DIR: cartella dove risiede il codice (dove risiede il CML)
# - PROJECT_BINARY_DIR: cartella dove stai compilando (./build/)
# (le variabili PROJECT sono popolate automaticamente come CMAKE_BINARY_DIR)
set(TOY_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(TOY_BINARY_DIR ${PROJECT_BINARY_DIR})

# include_directories aggiunge i percorsi al flag -I del compilatore
include_directories(${LLVM_INCLUDE_DIRS})               # header llvm
include_directories(${MLIR_INCLUDE_DIRS})               # header mlir
include_directories(${TOY_SOURCE_DIR}/include)   # miei header file
include_directories(${TOY_BINARY_DIR}/include)   # generati da tablegen

# link_directories aggiunge i percorsi al flag -L del compilatore
# qua puntiamo a ./llvm-project/build/lib/
link_directories(${LLVM_BUILD_LIBRARY_DIR})

# questo aggiunge i flag -D del preprocessore con cui LLVM e MLIR sono stati
# compilati. Servono per far si che gli header file che il mio progetto esterno
# definiscano un'ABI compatibile con quella delle librerie con cui linko
add_definitions(${LLVM_DEFINITIONS})



### IMPORTIAMO LE CARTELLE CHE DEFINISCONO COME BUILDARE I PEZZI DEL PROGETTO ###


# importante importare prima include di lib dato che include contiene anche
# i file .td che genereranno i file .cpp.inc utilizzati dentro lib/
# add_subdirectory(include)
# add_subdirectory(lib)

if(MLIR_ENABLE_BINDINGS_PYTHON)
  message(STATUS "Enabling Python API")
  include(MLIRDetectPythonEnv)
  mlir_configure_python_dev_packages()
  # Note: for EXTERNAL_PROJECT_BUILD this must be set from the command line.
  if(NOT MLIR_PYTHON_PACKAGE_PREFIX)
    set(MLIR_PYTHON_PACKAGE_PREFIX "mlir_standalone" CACHE STRING "" FORCE)
  endif()
  if(NOT MLIR_BINDINGS_PYTHON_NB_DOMAIN)
    set(MLIR_BINDINGS_PYTHON_NB_DOMAIN "mlir_standalone" CACHE STRING "" FORCE)
  endif()
  if(NOT MLIR_BINDINGS_PYTHON_INSTALL_PREFIX)
    set(MLIR_BINDINGS_PYTHON_INSTALL_PREFIX "python_packages/standalone/${MLIR_PYTHON_PACKAGE_PREFIX}" CACHE STRING "" FORCE)
  endif()
  add_subdirectory(python)
endif()

# variabile configurabile per includere i test o meno
option(INCLUDE_TESTS "Enable tests" ON)
if(INCLUDE_TESTS)
  add_subdirectory(test)
endif()

add_subdirectory(toy) # main

# vedi se ti servono
#
# # crea un plugin che può essere caricato con un altro tool mlir-opt esistente
# # (su windows non funzione e quindi non è abilitato)
# if(NOT WIN32)
#   add_subdirectory(standalone-plugin)
# endif()
# # altro main. Serve per tradurre il tuo dialetto in altri formati (es. esportare
# # verso LLVM IR o altri linguaggi).
# add_subdirectory(standalone-translate)
