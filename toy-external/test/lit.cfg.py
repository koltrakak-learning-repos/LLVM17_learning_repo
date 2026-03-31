# -*- Python -*-

import os
import platform
import re
import subprocess
import tempfile

import lit.formats
import lit.util

from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst
from lit.llvm.subst import FindTool

# Configuration file for the 'lit' test runner.

# name: The name of this test suite.
config.name = "TOY-EXTERNAL"
# test_format: dice a lit come interpretare i file di test (direttiva RUN)
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)
# suffixes: A list of file extensions to treat as test files (others are ignored).
config.suffixes = [".mlir", ".toy"]

# test_source_root: The root path where to look for the test files (dir of this file)
config.test_source_root = os.path.dirname(__file__)
# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.toy_obj_root, "test") # MODIFICARE

# directory utili
config.toy_tools_dir = os.path.join(config.toy_obj_root, "bin")
# config.toy_libs_dir = os.path.join(config.toy_obj_root, "lib")

# sostituzioni standard per le direttive RUN (%s)
llvm_config.use_default_substitutions()
# sostituzioni custom (pattern -> valore)
# config.substitutions.append(("%PATH%", config.environment["PATH"]))
# config.substitutions.append(("%shlibext", config.llvm_shlib_ext))
# config.substitutions.append(("%standalone_libs", config.standalone_libs_dir))

# excludes: A list of files and directories to exclude from the testsuite.
# The 'Inputs' subdirectories contain auxiliary inputs for various tests
# in their parent directories.
config.excludes = ["Inputs", "Examples", "CMakeLists.txt", "README.txt", "LICENSE.txt"]

llvm_config.with_system_environment(["HOME", "INCLUDE", "LIB", "TMP", "TEMP"])
# Tweak the PATH to include the tools dir (serve a lit per trovare FileCheck, etc.).
llvm_config.with_environment("PATH", config.llvm_tools_dir, append_path=True)

# diciamo a lit quali tool usare (e dove trovarli) quando processa le righe RUN
tool_dirs = [config.toy_tools_dir, config.llvm_tools_dir]
tools = [
    "toy", # nome del mio binario
    "mlir-opt",
]
llvm_config.add_tool_substitutions(tools, tool_dirs)


# per python bindings
# python_path = [os.path.join(config.mlir_obj_dir, "python_packages", "standalone")]
# if "PYTHONPATH" in os.environ:
#     python_path += [os.environ["PYTHONPATH"]]
# llvm_config.with_environment("PYTHONPATH", python_path, append_path=True)
