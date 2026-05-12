cmake_minimum_required(VERSION 3.20)
project(mlir-pass-example)

find_package(MLIR REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
include(AddMLIR)

add_executable(test-pass mlir_traverser.cpp)

target_link_libraries(test-pass PRIVATE
  MLIROptLib        # MlirOptMain
  MLIRPass          # PassWrapper, PassRegistration
  MLIRIR            # Operation, Region, Block
  MLIRSupport       # StringRef, raw_ostream
  MLIRRegisterAllDialects # autoesplicativo
)

if(NOT LLVM_ENABLE_RTTI)
  target_compile_options(test-pass PRIVATE -fno-rtti)
endif()
