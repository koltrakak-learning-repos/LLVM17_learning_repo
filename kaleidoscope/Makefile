.DEFAULT_GOAL := build

SRC_DIR := ./src
INCLUDE_DIR := ./include
BUILD_DIR := ./build

TARGET := $(BUILD_DIR)/kaleidoscope.out
INPUT_PROGRAM := $(SRC_DIR)/fibonacci.kal

FLAGS := -fuse-ld=mold -I$(INCLUDE_DIR)

.PHONY: build run clean

build:
	clang++ $(FLAGS) $(SRC_DIR)/lexer.cpp -o $(TARGET)

run: build
	$(TARGET)

run-file: build
	$(TARGET) $(INPUT_PROGRAM)

clean:
	rm -f $(TARGET)
