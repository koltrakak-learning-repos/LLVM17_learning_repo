.DEFAULT_GOAL := build

SRC_DIR := ./src
INCLUDE_DIR := ./include
BUILD_DIR := ./build
TEST_DIR := ./test

SRC_FILES := \
	$(SRC_DIR)/lexer.cpp \
	$(SRC_DIR)/ast.cpp \
	$(SRC_DIR)/parser.cpp \
	$(SRC_DIR)/main.cpp

TARGET := $(BUILD_DIR)/kaleidoscope.out
TEST_TARGET := $(BUILD_DIR)/lexer_test.out

INPUT_PROGRAM := $(SRC_DIR)/test_parser.kal

FLAGS := -fuse-ld=mold -I$(INCLUDE_DIR)

.PHONY: build run run-file test clean

build:
	mkdir -p $(BUILD_DIR)
	clang++ $(FLAGS) $(SRC_FILES) -o $(TARGET)

run: build
	$(TARGET)

run-file: build
	$(TARGET) $(INPUT_PROGRAM)

test:
	mkdir -p $(BUILD_DIR)
	clang++ $(FLAGS) $(TEST_DIR)/lexer_test.cpp $(SRC_DIR)/lexer.cpp -o $(TEST_TARGET)
	$(TEST_TARGET) $(INPUT_PROGRAM)

clean:
	rm -rf $(BUILD_DIR)
