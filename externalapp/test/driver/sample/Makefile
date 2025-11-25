# Makefile for compiling a simple gTest project with gCoverage

# --- Configuration ---
# C++ compiler
CXX := g++

# Compiler flags: Standard C++14, warnings, include path for gTest
CXXFLAGS := -std=c++14 -Wall -I/usr/include -I$(UNIT_DIR)

# Flags for code coverage (gcov)
GCOV_FLAGS := -fprofile-arcs -ftest-coverage

# Linker flags: Link with gtest, gtest_main, and pthread
LDFLAGS := -lgtest -lgtest_main -pthread

# Executable name
TARGET := target.test

# Source files
SRC_IMPL := $(UNIT_DIR)/helloworld.c
SRC_TEST := test_helloworld.cpp

# Coverage Report Directory
COVERAGE_DIR := report

# --- Targets ---

.PHONY: all clean run coverage report

# Default target: builds and runs the test
all: $(TARGET) run

# Rule to compile the test executable *with* coverage flags
$(TARGET): $(SRC_IMPL) $(SRC_TEST)
	$(CXX) $(CXXFLAGS) $(GCOV_FLAGS) $^ -o $@ $(LDFLAGS)

# Rule to run the test executable
run: $(TARGET)
	./$(TARGET)

# Rule to generate the coverage report
coverage: clean $(TARGET) run report

# Rule to generate the HTML report using lcov
report:
	@echo "--- Generating Coverage Report ---"
	# 1. Capture coverage data from the execution (.gcda files)
	lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
	# 2. Filter out system headers and test files (optional but recommended)
	lcov --remove coverage.info '/usr/*' '*/test_*.cpp' --output-file coverage_filtered.info --ignore-errors mismatch
	# 3. Generate HTML report
	genhtml coverage_filtered.info --output-directory $(COVERAGE_DIR)
	@echo "--- Coverage report generated in: $(COVERAGE_DIR)/index.html ---"

# Rule to clean up generated files and coverage data
clean:
	rm -f $(TARGET)
	rm -f *.o *.gcda *.gcno *.info
	rm -rf $(COVERAGE_DIR)