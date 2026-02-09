@echo off
REM Quick setup script to build and run the Cancer Diagnosis System server
REM Run this from the project root directory

echo.
echo ========================================
echo Cancer Diagnosis System - Server Setup
echo ========================================
echo.

REM Check if we're in the right directory
if not exist "CancerDiagnosisSystem.h" if not exist "headers\CancerDiagnosisSystem.h" (
    echo ERROR: Please run this script from the project root directory
    echo Expected location: CancerDiagnosisSystem\
    pause
    exit /b 1
)

REM Step 1: Check if CMake is installed
echo [1/4] Checking CMake installation...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found. Please install CMake first.
    echo Download from: https://cmake.org/download/
    pause
    exit /b 1
)
echo ✓ CMake found

REM Step 2: Create and configure build directory
echo.
echo [2/4] Setting up build directory...
if not exist "build" mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)
echo ✓ Build directory configured

REM Step 3: Build the project
echo.
echo [3/4] Building project (this may take 1-2 minutes)...
cmake --build . --config Release
if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)
echo ✓ Build successful

REM Step 4: Run the server
echo.
echo [4/4] Starting server...
echo.
echo ========================================
echo Server is running on http://localhost:8080
echo Keep this window open to use the system
echo Press Ctrl+C to stop the server
echo ========================================
echo.

.\Release\cds_server.exe
if errorlevel 1 (
    echo ERROR: Server failed to start
    echo Check:
    echo  - Port 8080 is not in use
    echo  - Data files exist (data/genes.csv, data/patients.csv)
    pause
    exit /b 1
)

pause
