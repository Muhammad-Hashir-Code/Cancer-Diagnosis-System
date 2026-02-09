# 🔬 Cancer Diagnosis System - Complete Setup Guide

## Quick Answer: Why Data Isn't Loading

**The most common reason:** The C++ server is not running, or it's running from the wrong directory so it can't find the data files.

---

## ⚡ Quick Fix (5 minutes)

### Windows:
```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
setup_server.bat
```

This script will:
1. Check for CMake
2. Build the C++ server
3. Start the server automatically

Keep the terminal open. You'll see:
```
Starting server on http://localhost:8080 ...
```

### macOS/Linux:
```bash
cd "/path/to/CancerDiagnosisSystem"
chmod +x setup_server.sh
./setup_server.sh
```

---

## 📋 Manual Setup (if scripts don't work)

### Step 1: Build the Server

```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
```

### Step 2: Run the Server (from project root, not build/)

```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
./build/Release/cds_server.exe
```

**IMPORTANT:** Always run from project root so it can find `data/` folder!

### Step 3: Open UI in Browser

```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem\ui"
python -m http.server 8000
# Then open: http://localhost:8000/index.html
```

### Step 4: Load Data

1. Go to **Load Data** section in UI
2. Click **"Load Data on Server"** button
3. Wait 3-5 seconds
4. Should show success message

---

## 🐛 Troubleshooting

### Problem 1: "Server load failed" / "Failed to fetch"
**Cause:** Server not running

**Fix:**
```bash
# Check if server is running
# Terminal should show: "Starting server on http://localhost:8080 ..."
# If you don't see this, run:
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
./build/Release/cds_server.exe
```

### Problem 2: Server shows "Error: Could not open file data/genes.csv"
**Cause:** Server running from wrong directory

**Fix:**
```bash
# ❌ WRONG - won't find data files
cd build && ./Release/cds_server.exe

# ✅ RIGHT - will find data files
cd .. && ./build/Release/cds_server.exe
```

### Problem 3: "No genetic data loaded! Cannot train models"
**Cause:** CSV format is wrong

**Fix:** Check `data/genes.csv` headers:
```
Gene_ID,Mutation_Score,Label
GENE_001,0.85,1
```

And `data/patients.csv`:
```
Patient_ID,Name,Age
P001,John Doe,45
```

### Problem 4: Port 8080 already in use
**Cause:** Another process using the port

**Fix:**
- Find and close other servers
- Or modify `src/Server.cpp` line with `svr.listen("localhost", 8080)` to use different port

---

## 🔍 Diagnostic: Test Data Loading in Browser

1. Open **Settings** section
2. Verify endpoint is `http://localhost:8080`
3. Open browser console (F12)
4. Paste this command:

```javascript
fetch('http://localhost:8080/status')
  .then(r => r.json())
  .then(d => console.log('✓ Server OK:', d))
  .catch(e => console.error('✗ Server down:', e.message));
```

If you see:
```
✓ Server OK: {modelsTrained: false, geneticCount: 0, patientCount: 0}
```

Then server is running. Now test data loading:

```javascript
fetch('http://localhost:8080/load', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({genesFile: 'data/genes.csv', patientsFile: 'data/patients.csv'})
})
.then(r => r.json())
.then(d => console.log('✓ Data loaded:', d))
.catch(e => console.error('✗ Load failed:', e.message));
```

If this works, data loading is fixed!

---

## 📂 File Structure (for reference)

```
CancerDiagnosisSystem/
├── data/
│   ├── genes.csv          ← Server reads from here
│   ├── patients.csv       ← Server reads from here
│   ├── test_genes.csv
│   └── test_patients.csv
├── src/
│   ├── Server.cpp         ← HTTP server (port 8080)
│   ├── CancerDiagnosisSystem.cpp
│   └── ...
├── ui/
│   ├── index.html         ← Frontend (open in browser)
│   ├── script.js
│   ├── styles.css
│   ├── diagnostic.js      ← Debugging helper
│   └── ...
├── build/                 ← Created by cmake
│   ├── Release/
│   │   └── cds_server.exe ← Run this
│   └── ...
├── CMakeLists.txt
├── setup_server.bat       ← Windows quick setup
├── setup_server.sh        ← Linux/Mac quick setup
└── DATA_LOADING_TROUBLESHOOTING.md
```

---

## ✅ Complete Workflow

1. **Terminal 1 - Start Server:**
   ```bash
   cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
   ./build/Release/cds_server.exe
   # Keep this open!
   ```

2. **Terminal 2 - Start UI Server:**
   ```bash
   cd "e:\dsaproject - Copy\CancerDiagnosisSystem\ui"
   python -m http.server 8000
   ```

3. **Browser:**
   - Open: http://localhost:8000/index.html
   - Go to **Settings** → Verify endpoint is `http://localhost:8080`
   - Go to **Load Data** → Click **"Load Data on Server"**
   - Wait for success message
   - Check **Dashboard** for stats
   - Try **Diagnosis** section

---

## 💾 Data Files Format

### genes.csv (required)
```csv
Gene_ID,Mutation_Score,Label
GENE_001,0.85,1
GENE_002,0.72,1
GENE_003,0.45,0
```

### patients.csv (required)
```csv
Patient_ID,Name,Age
P001,John Doe,45
P002,Jane Smith,52
P003,Mike Johnson,38
```

---

## 📞 Still Need Help?

Run the diagnostic script in browser console:
1. Open http://localhost:8000/index.html
2. Press F12 to open console
3. Paste content from `ui/diagnostic.js`
4. Check output for exact error

---

## 🎯 Key Points to Remember

- ✅ Always run server from **project root**, not from build/ folder
- ✅ Server must be running to use "Load Data on Server"
- ✅ Data files must exist in `data/` folder with correct headers
- ✅ Check browser console (F12) for CORS or network errors
- ✅ Default server endpoint: `http://localhost:8080`
- ✅ Default UI server: `http://localhost:8000`

---

Generated: December 2, 2025
