# Data Loading Troubleshooting Guide

## Problem: Data Not Loading on Server

The data loading fails when clicking **"Load Data on Server"** button. Here are the common causes and solutions:

---

## **Issue 1: Server Not Running**

### Symptom:
- Button click shows: `"Server load failed: Failed to fetch"`
- Error appears immediately (no timeout)

### Solution:
1. Build the C++ server:
```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
```

2. Run the server **from the project root directory** (important!):
```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
./build/Release/cds_server.exe
# or on Linux/Mac:
# ./build/cds_server
```

You should see:
```
Starting server on http://localhost:8080 ...
```

3. Keep the server terminal open while using the UI.

---

## **Issue 2: Working Directory Mismatch (Most Common)**

### Symptom:
- Server shows error: `Error: Could not open file data/genes.csv`
- Status shows: `"Server load failed"`

### Root Cause:
The server looks for `data/genes.csv` **relative to where the server executable is running**, not where the executable is located.

### Solution:
**Always run the server from the project root:**
```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
./build/Release/cds_server.exe
```

**NOT from build/ directory:**
```bash
# ❌ WRONG - will fail to find data files
cd "e:\dsaproject - Copy\CancerDiagnosisSystem\build"
./Release/cds_server.exe
```

---

## **Issue 3: Network/CORS Issues**

### Symptom:
- Browser console shows CORS error
- Network tab shows OPTIONS request failing

### Solution:
The server already has CORS headers. If still failing:
1. Check browser console (F12 > Console)
2. Verify server endpoint in UI Settings matches where server is running (default: `http://localhost:8080`)
3. Make sure server is actually listening (check terminal output)

---

## **Issue 4: File Format Issues**

### Symptom:
- Server loads but shows: `"No genetic data loaded! Cannot train models"`

### Root Cause:
CSV format is incorrect. The files expect:

**genes.csv format:**
```
Gene_ID,Mutation_Score,Label
GENE_001,0.85,1
GENE_002,0.72,1
```

**patients.csv format:**
```
Patient_ID,Name,Age
P001,John Doe,45
P002,Jane Smith,52
```

### Solution:
1. Verify CSV headers match exactly (case-sensitive)
2. Check that data is comma-separated (no extra spaces)
3. Replace data files with valid CSV format

---

## **Step-by-Step: Correct Data Loading Process**

### 1. Start the Server
```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem"
./build/Release/cds_server.exe
```

Expected output:
```
Starting server on http://localhost:8080 ...
```

### 2. Open UI in Browser
- Serve the UI folder:
```bash
cd "e:\dsaproject - Copy\CancerDiagnosisSystem\ui"
python -m http.server 8000
# Open: http://localhost:8000/index.html
```

### 3. Verify Settings
- Go to **Settings** section in UI
- Check API Endpoint: `http://localhost:8080`
- Click **Save Settings**

### 4. Load Data
- Go to **Load Data** section
- Click **"Load Data on Server"** button
- Wait 3-5 seconds for models to train

### 5. Verify Success
- Check browser console (F12) for any errors
- Look for success message in green
- Check **Dashboard** - stats should update

---

## **Debugging Tips**

### Check Server Console Output
The server outputs loading progress:
```
=== Loading Data ===
Loaded 20 genetic data records.
=== Data Summary ===
Genetic records loaded: 20
Patient records loaded: 10

=== Training Models ===
Training Logistic Regression...
✓ System ready for diagnosis!
```

If you don't see this, the data loading failed on the server side.

### Check Browser Network Tab
1. Open DevTools (F12)
2. Go to **Network** tab
3. Click "Load Data on Server"
4. Look for `POST /load` request
5. Check response (should show geneticCount and patientCount > 0)

### Enable Debug Logging
Add to script.js before loadData call:
```javascript
// Add after fetch request
.then(resp => {
    console.log('Response status:', resp.status);
    console.log('Response headers:', resp.headers);
    return resp.json();
})
.then(data => {
    console.log('Server returned:', data);
    // ... rest of code
})
```

---

## **Quick Checklist**

- [ ] Server built with `cmake --build . --config Release`
- [ ] Server running from project root (not from build/ folder)
- [ ] Server shows "Starting server on http://localhost:8080"
- [ ] UI endpoint in Settings is `http://localhost:8080`
- [ ] CSV files exist in `data/` folder with correct headers
- [ ] Browser console shows no errors (F12 > Console)
- [ ] Network request shows 200 response (F12 > Network)

---

## **Still Not Working?**

Run this diagnostic in browser console:
```javascript
// Test if server is reachable
fetch('http://localhost:8080/status')
  .then(r => r.json())
  .then(data => console.log('✓ Server is reachable:', data))
  .catch(e => console.error('✗ Server error:', e.message));

// Test load endpoint
fetch('http://localhost:8080/load', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({genesFile: 'data/genes.csv', patientsFile: 'data/patients.csv'})
})
  .then(r => r.json())
  .then(data => console.log('✓ Data load result:', data))
  .catch(e => console.error('✗ Load error:', e.message));
```

Check console output - this will tell you exactly where the failure is.

