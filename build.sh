#!/bin/bash

# Set the project name and version
PROJECT_NAME="MayoranaFramework"
PROJECT_VERSION="1.0"

# Set the C++ standard and compiler flags
CXX_STANDARD=17
DEBUG_FLAGS="-g -arch arm64"  # For generating debug information
PROFILE_FLAGS="-DPROFILER_PROJECT=1"  # Define the profiler project

# Set the compiler
CXX=clang++

# Set include directories
SOURCE_DIR="source"

# Define the source file
SOURCE_FILE="source/main.cpp"

# Create an output directory for the executable
OUTPUT_DIR="./bin"
mkdir -p $OUTPUT_DIR

# Compile the source file into an object file
OBJECT_FILE="${OUTPUT_DIR}/main.o"
$CXX -arch arm64 -std=c++$CXX_STANDARD $DEBUG_FLAGS $PROFILE_FLAGS -I$SOURCE_DIR -I$INCLUDE_DIR -I$PART2_INCLUDE_DIR -c $SOURCE_FILE -o $OBJECT_FILE

# Check if the compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed for $SOURCE_FILE"
    exit 1
fi

# Link the object file into the final executable
$CXX -arch arm64 $OBJECT_FILE -o $OUTPUT_DIR/$PROJECT_NAME

# Check if the linking was successful
if [ $? -eq 0 ]; then
    echo "Compilation Completed."
else
    echo "Build failed!"
    exit 1
fi