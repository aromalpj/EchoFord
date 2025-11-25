#!/bin/bash

echo "====================================="
echo "ECHOFORD UNIT TEST & COVERAGE UTILITY"
echo "====================================="

TEST_DIR="."
SRC_DIR="../src"

# 1. Accept an command line argument and check if it was provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <filename>"
    echo "Example: $0 main.c"
    exit 1
fi

TC_PREFIX="test_"
TC_SUFFIX="pp"
SRC_FILE="$1"
TEST_FILE="$TC_PREFIX$1$TC_SUFFIX"


# 2. Search the filename from the directory path mentioned and find the file path
SRC_FILE_PATH=$(find "$SRC_DIR" -type f -name "$SRC_FILE" -print -quit 2>/dev/null)


if [ -z "$SRC_FILE_PATH" ]; then
    echo "Error: Source File '$SRC_FILE' not found."
    exit 2
fi

TEST_FILE_PATH=$(find "$TEST_DIR" -type f -name "$TEST_FILE" -print -quit 2>/dev/null)
if [ -z "$TEST_FILE_PATH" ]; then
    echo "Error: Test File '$TEST_FILE' not found."
    exit 2
fi


# Extract the directory path of the found file
SRC_PROJECT_DIR=$(dirname "$SRC_FILE_PATH")
TEST_PROJECT_DIR=$(dirname "$TEST_FILE_PATH")

echo "Found source file at: $SRC_FILE_PATH"
echo "Project source directory identified: $SRC_PROJECT_DIR"

echo "Found Test file at: $TEST_FILE_PATH"
echo "Project test directory identified: $TEST_PROJECT_DIR"

MAKE_COV_ARGS="coverage UNIT_DIR=$(dirname "$(readlink -f "$SRC_FILE_PATH")")"

# 3. Do the make operation in the path specified
if [ -d "$TEST_PROJECT_DIR" ]; then
    echo "Attempting to run GTest in $TEST_PROJECT_DIR..."
    
    # Change directory and execute make
    if cd "$TEST_PROJECT_DIR"; then
        if make $MAKE_COV_ARGS; then
            echo "Successfully ran 'make' in $TEST_PROJECT_DIR."
            exit 0
        else
            # make command failed
            echo "Error: 'make' command failed in $TEST_PROJECT_DIR. Check the Makefile and project dependencies."
            exit 3
        fi
        echo "Successfully ran GTest in $TEST_PROJECT_DIR."
    else
        # cd command failed (should be rare if find was successful)
        echo "Error: Could not change directory to $TEST_PROJECT_DIR."
        exit 4
    fi
else
    # Directory check failed (just for robust error handling)
    echo "Error: Project directory $TEST_PROJECT_DIR does not exist or is inaccessible."
    exit 5
fi