cmake_minimum_required(VERSION 3.20)
project(assignment1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # per clangd; configura con -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

#===============================================================================
# 1. LOAD LLVM CONFIGURATION
#===============================================================================
# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "/home/kevin/LLVM-19.1.7-Linux-X64" CACHE PATH "LLVM installation directory")

# Add the location of LLVMConfig.cmake to CMake search paths (so that
# find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM CONFIG)
if("${LLVM_VERSION_MAJOR}" VERSION_LESS 19)
  message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 19 or above")
endif()
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# HelloWorld includes headers from LLVM - update the include paths accordingly
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

#===============================================================================
# 2. BUILD CONFIGURATION
#===============================================================================
# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

# Usa mold come linker se disponibile
find_program(MOLD_LINKER mold)
if(MOLD_LINKER)
  add_link_options(-fuse-ld=mold)
  message(STATUS "Using mold linker: ${MOLD_LINKER}")
else()
  message(STATUS "mold linker not found, using default")
endif()

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

#===============================================================================
# 3. ADD THE TARGET
#===============================================================================
add_library(assignment1 SHARED assignment1.cpp)

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(assignment1
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
