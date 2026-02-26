.DEFAULT_GOAL := build

SRC_DIR := ../src
BUILD_DIR := .
TARGET := $(BUILD_DIR)/kaleidoscope.out

.PHONY: build run clean

build:
	clang++ $(SRC_DIR)/lexer.cpp -o $(TARGET)

run: build
	$(TARGET)

clean:
	rm -f $(TARGET)
