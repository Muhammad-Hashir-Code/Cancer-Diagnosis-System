#!/bin/bash
# Quick setup script to build and run the Cancer Diagnosis System server
# Run this from the project root directory

echo ""
echo "========================================"
echo "Cancer Diagnosis System - Server Setup"
echo "========================================"
echo ""

# Check if we're in the right directory
if [ ! -f "headers/CancerDiagnosisSystem.h" ] && [ ! -f "CancerDiagnosisSystem.h" ]; then
    echo "ERROR: Please run this script from the project root directory"
    echo "Expected location: e:\dsaproject - Copy\CancerDiagnosisSystem\"
    exit 1
fi

# Step 1: Check if CMake is installed
echo "[1/4] Checking CMake installation..."
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Please install CMake first."
    echo "Install with: brew install cmake (macOS) or apt-get install cmake (Linux)"
    exit 1
fi
echo "✓ CMake found"

# Step 2: Create and configure build directory
echo ""
echo "[2/4] Setting up build directory..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi
echo "✓ Build directory configured"

# Step 3: Build the project
echo ""
echo "[3/4] Building project (this may take 1-2 minutes)..."
cmake --build . --config Release
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi
echo "✓ Build successful"

# Step 4: Run the server
echo ""
echo "[4/4] Starting server..."
echo ""
echo "========================================"
echo "Server is running on http://localhost:8080"
echo "Keep this window open to use the system"
echo "Press Ctrl+C to stop the server"
echo "========================================"
echo ""

./cds_server
if [ $? -ne 0 ]; then
    echo "ERROR: Server failed to start"
    echo "Check:"
    echo " - Port 8080 is not in use"
    echo " - Data files exist (data/genes.csv, data/patients.csv)"
    exit 1
fi
