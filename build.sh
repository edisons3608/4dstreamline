#!/bin/bash
clear
# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Run CMake
cmake ..

# Build the project
make

echo "Build complete! Run with: ./main" 

./main