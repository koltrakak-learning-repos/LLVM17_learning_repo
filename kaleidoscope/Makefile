.DEFAULT_GOAL := build

SRC_DIR := ./src
INCLUDE_DIR := ./include
BUILD_DIR := ./build
TEST_DIR := ./test
KAL_SRC_DIR := ./kaleidoscope_files

SRC_FILES := \
	$(SRC_DIR)/lexer.cpp \
	$(SRC_DIR)/ast.cpp \
	$(SRC_DIR)/parser.cpp
MAIN := $(SRC_DIR)/main.cpp

TARGET := $(BUILD_DIR)/kaleidoscope.out
INPUT_PROGRAM := $(KAL_SRC_DIR)/test_codegen.kal

LLVM_FLAGS := $(shell llvm-config --cxxflags --ldflags --system-libs --libs core)
FLAGS := -fuse-ld=mold -I$(INCLUDE_DIR)

.PHONY: build run run-file test-lexer test-parser test-codegen clean

build:
	mkdir -p $(BUILD_DIR)
	clang++ $(FLAGS) $(SRC_FILES) $(MAIN) $(LLVM_FLAGS) -o $(TARGET)

run: build
	$(TARGET)

run-file: build
	$(TARGET) $(INPUT_PROGRAM)


test-lexer: INPUT_PROGRAM := $(KAL_SRC_DIR)/fibonacci.kal
test-lexer:
	mkdir -p $(BUILD_DIR)
	clang++ $(FLAGS) $(TEST_DIR)/lexer_test.cpp $(SRC_DIR)/lexer.cpp -o $(BUILD_DIR)/lexer_test.out
	$(BUILD_DIR)/lexer_test.out $(INPUT_PROGRAM)

test-parser: MAIN := $(TEST_DIR)/parser_test.cpp
test-parser: INPUT_PROGRAM := $(KAL_SRC_DIR)/test_parser.kal
test-parser:
	mkdir -p $(BUILD_DIR)
	clang++ $(FLAGS) $(SRC_FILES) $(MAIN) $(LLVM_FLAGS) -o $(BUILD_DIR)/parser_test.out
	$(BUILD_DIR)/parser_test.out $(INPUT_PROGRAM)

test-codegen: MAIN := $(TEST_DIR)/codegen_test.cpp
test-codegen: INPUT_PROGRAM := $(KAL_SRC_DIR)/test_codegen.kal
test-codegen:
	mkdir -p $(BUILD_DIR)
	clang++ $(FLAGS) $(SRC_FILES) $(MAIN) $(LLVM_FLAGS) -o $(BUILD_DIR)/codegen_test.out
	$(BUILD_DIR)/codegen_test.out $(INPUT_PROGRAM)

clean:
	rm -rf $(BUILD_DIR)
