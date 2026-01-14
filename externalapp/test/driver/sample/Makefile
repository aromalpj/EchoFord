# Makefile for compiling a simple gTest project with gCoverage

# Project paths
TEST_DIR  := $(shell readlink -f ../../)
BUILD_DIR := .

# --- Configuration ---
# C, C++ compiler
CC  := gcc
CXX := g++

# Recursively find all directories under TEST_DIR
INC_TEST_DIRS := $(shell find $(TEST_DIR) -type d)

# --- Includes ---
INC := -I/usr/include \
       -I$(UNIT_DIR)/ \
	   $(addprefix -I,$(INC_TEST_DIRS))

# Compiler flags: for compiling C files
CFLAGS   := -Dstatic= -Wall \
            $(shell $(TEST_DIR)/lib/bin/cmock-config --cflags) \
            $(INC)

# Compiler flags: for compiling C++ files
CXXFLAGS := -std=c++14 -Wall \
            $(shell $(TEST_DIR)/lib/bin/cmock-config --cflags) \
            $(INC)

# Flags for code coverage (gcov)
GCOV_FLAGS := -fprofile-arcs -ftest-coverage

# Linker flags: Link with pthread, gtest, gmock
LDFLAGS := -pthread \
           -lgtest \
           -lgmock \
           $(shell $(TEST_DIR)/lib/bin/cmock-config --libs)

# Executable name
TARGET := target.test

# Source files
SRC_IMPL := $(UNIT_DIR)/helloworld.c
SRC_TEST := test_helloworld.cc

SRC_NAME := $(notdir $(SRC_IMPL))
SRC_BASE := $(basename $(SRC_NAME))

TEST_NAME := $(notdir $(SRC_TEST))
TEST_BASE := $(basename $(TEST_NAME))

# Prefixed Objects
C_OBJS   := $(BUILD_DIR)/$(TARGET)-$(SRC_BASE).o
CPP_OBJS := $(BUILD_DIR)/$(TARGET)-$(TEST_BASE).o

# Coverage Report Directory
COVERAGE_DIR := report

# --- Targets ---
.PHONY: all clean run coverage report

# Default target: builds and runs the test
all: $(TARGET) run

# C source → prefixed object
$(BUILD_DIR)/$(TARGET)-%.o: $(UNIT_DIR)/%.c
	$(CC) -c $(CFLAGS) $(GCOV_FLAGS) $< -o $@

# C++ test → prefixed object
$(BUILD_DIR)/$(TARGET)-%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $(GCOV_FLAGS) $< -o $@

# Link test executable
$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(GCOV_FLAGS) -I/home/aromal/EchoFord/externalapp/externalapp/src/driver/sample

# Rule to run the test executable
run: $(TARGET)
	./$(TARGET)

# Rule to generate the coverage report
coverage: clean $(TARGET) run report

# Rule to generate the HTML report using lcov
report:
	@echo "--- Generating Coverage Report ---"
	lcov --capture \
	     --directory $(BUILD_DIR) \
	     --directory $(UNIT_DIR) \
	     --output-file coverage.info \
	     --ignore-errors mismatch,empty
	lcov --remove coverage.info \
	     '/usr/*' '*test_*' '*mock*' '/lib/*' \
	     --output-file coverage_filtered.info \
	     --ignore-errors mismatch,empty,unused
	genhtml coverage_filtered.info --output-directory $(COVERAGE_DIR)
	@echo "--- Coverage report generated in: $(COVERAGE_DIR)/index.html ---"

# Rule to clean up generated files and coverage data
clean:
	rm -f $(TARGET)
	rm -f *.o *.gcda *.gcno *.info
	rm -rf $(COVERAGE_DIR)