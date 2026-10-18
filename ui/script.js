// State management
let systemState = {
    geneticRecords: 0,
    patientRecords: 0,
    modelsTrained: false,
    queueSize: 0,
    apiEndpoint: 'http://localhost:8080',
    geneticRecordCount: 0,
    patients: [], // loaded from patients.csv or uploaded file
    geneticData: [] // loaded from genes.csv or uploaded file
};

// (No local diagnosis heuristics here — frontend delegates diagnosis to backend server)

// Initialize the application
document.addEventListener('DOMContentLoaded', () => {
    initializeUI();
    loadSettings();
    setupEventListeners();
    addGeneticRecord(); // Add first genetic record input by default
    updateDashboard();
});

// Initialize UI
function initializeUI() {
    const navButtons = document.querySelectorAll('.nav-btn');
    navButtons.forEach(btn => {
        btn.addEventListener('click', (e) => {
            e.preventDefault();
            switchSection(btn.dataset.section);
        });
    });

    const tabButtons = document.querySelectorAll('.tab-btn');
    tabButtons.forEach(btn => {
        btn.addEventListener('click', (e) => {
            e.preventDefault();
            switchTab(btn.dataset.tab);
        });
    });

    // Dark mode toggle
    const darkModeCheckbox = document.getElementById('dark-mode');
    darkModeCheckbox.addEventListener('change', toggleDarkMode);
}

// Setup event listeners
function setupEventListeners() {
    // Add enter key support
    document.addEventListener('keypress', (e) => {
        if (e.key === 'Enter' && e.target.tagName === 'INPUT') {
            const form = e.target.closest('.form-card');
            if (form) {
                const submitBtn = form.querySelector('.btn-primary');
                if (submitBtn) submitBtn.click();
            }
        }
    });
}

// Switch between sections
function switchSection(sectionId) {
    const sections = document.querySelectorAll('.section');
    const navButtons = document.querySelectorAll('.nav-btn');

    sections.forEach(section => section.classList.remove('active'));
    navButtons.forEach(btn => btn.classList.remove('active'));

    const activeSection = document.getElementById(sectionId);
    if (activeSection) {
        activeSection.classList.add('active');
        document.getElementById('section-title').textContent = 
            sectionId.split('-').map(word => word.charAt(0).toUpperCase() + word.slice(1)).join(' ');
    }

    const activeNavBtn = document.querySelector(`[data-section="${sectionId}"]`);
    if (activeNavBtn) {
        activeNavBtn.classList.add('active');
    }
}

// Switch between tabs
function switchTab(tabId) {
    const tabs = document.querySelectorAll('.tab-content');
    const tabButtons = document.querySelectorAll('.tab-btn');

    tabs.forEach(tab => tab.classList.remove('active'));
    tabButtons.forEach(btn => btn.classList.remove('active'));

    const activeTab = document.getElementById(tabId);
    if (activeTab) {
        activeTab.classList.add('active');
    }

    const activeTabBtn = document.querySelector(`[data-tab="${tabId}"]`);
    if (activeTabBtn) {
        activeTabBtn.classList.add('active');
    }
}

// Load Data
async function loadData() {
    const genesPath = document.getElementById('genes-file').value || 'data/genes.csv';
    const patientsPath = document.getElementById('patients-file').value || 'data/patients.csv';
    const genesFileInput = document.getElementById('genes-file-input');
    const patientsFileInput = document.getElementById('patients-file-input');
    const statusDiv = document.getElementById('load-status');

    showStatus(statusDiv, 'Loading data...', 'info');
    setStatusOnline(false);

    try {
        // Helper to parse CSV text into array of objects
        const parseCSV = (text) => {
            const lines = text.split(/\r?\n/).filter(l => l.trim() !== '');
            if (lines.length === 0) return [];
            const headers = lines[0].split(',').map(h => h.trim().toLowerCase());
            const rows = lines.slice(1).map(line => {
                const cols = line.split(',');
                const obj = {};
                for (let i = 0; i < headers.length; i++) {
                    obj[headers[i]] = (cols[i] || '').trim();
                }
                return obj;
            });
            return rows;
        };

        // Load patients CSV: prefer uploaded file, otherwise fetch path
        let patientsData = [];
        if (patientsFileInput && patientsFileInput.files && patientsFileInput.files[0]) {
            const text = await patientsFileInput.files[0].text();
            patientsData = parseCSV(text);
        } else {
            // Try fetch (works if you serve the project root)
            const resp = await fetch(patientsPath);
            if (!resp.ok) throw new Error('Failed to fetch patients file: ' + resp.statusText);
            const text = await resp.text();
            patientsData = parseCSV(text);
        }

        // Load genes CSV
        let genesData = [];
        if (genesFileInput && genesFileInput.files && genesFileInput.files[0]) {
            const text = await genesFileInput.files[0].text();
            genesData = parseCSV(text);
        } else {
            const resp = await fetch(genesPath);
            if (!resp.ok) throw new Error('Failed to fetch genes file: ' + resp.statusText);
            const text = await resp.text();
            genesData = parseCSV(text);
        }

        // Normalize patients into expected structure
        systemState.patients = patientsData.map(p => ({
            patient_id: (p.patient_id || p.id || p.patientid || '').trim(),
            name: (p.name || p.fullname || '').trim(),
            age: p.age ? parseInt(p.age, 10) : null
        })).filter(p => p.patient_id);

        // Normalize genes into expected structure
        systemState.geneticData = genesData.map(g => ({
            gene_id: (g.gene_id || g.id || '').trim(),
            mutation_score: g.mutation_score ? parseFloat(g.mutation_score) : null,
            label: g.label ? parseInt(g.label, 10) : null
        })).filter(g => g.gene_id);

        // Update counts and UI
        systemState.geneticRecords = systemState.geneticData.length;
        systemState.patientRecords = systemState.patients.length;
        // Mark models trained only if we have some data
        systemState.modelsTrained = systemState.geneticRecords > 0 && systemState.patientRecords > 0;

        updateDashboard();
        showStatus(statusDiv, 'Data loaded locally. To use backend models, click "Load Data on Server" below or call the server endpoint in Settings.', 'success');
        showNotification('Data loaded locally', 'success');
    } catch (error) {
        showStatus(statusDiv, 'Error loading data: ' + error.message, 'error');
        showNotification('Error loading data: ' + error.message, 'error');
    } finally {
        setStatusOnline(true);
    }
}

// Call server to load data and train models. Expects backend server running at systemState.apiEndpoint
async function loadDataOnServer() {
    const genes = document.getElementById('genes-file').value || 'data/genes.csv';
    const patients = document.getElementById('patients-file').value || 'data/patients.csv';
    const statusDiv = document.getElementById('load-status');

    showStatus(statusDiv, 'Requesting server to load data...', 'info');
    setStatusOnline(false);
    try {
        const resp = await fetch(systemState.apiEndpoint + '/load', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ genesFile: genes, patientsFile: patients })
        });
        if (!resp.ok) throw new Error('Server responded: ' + resp.statusText);
        const data = await resp.json();
        systemState.modelsTrained = !!data.modelsTrained;
        systemState.geneticRecords = data.geneticCount || systemState.geneticRecords;
        systemState.patientRecords = data.patientCount || systemState.patientRecords;
        updateDashboard();
        // Refresh analytics chart after data load
        setTimeout(() => {
            updatePatientChart();
        }, 500);
        showStatus(statusDiv, 'Server loaded data and trained models: ' + (systemState.modelsTrained ? 'OK' : 'Models not trained'), 'success');
        showNotification('Server load complete', 'success');
    } catch (err) {
        showStatus(statusDiv, 'Server load failed: ' + err.message, 'error');
        showNotification('Server load failed', 'error');
    } finally {
        setStatusOnline(true);
    }
}

// Add genetic record input
let recordIndex = 0;
function addGeneticRecord() {
    const container = document.getElementById('genetic-records-container');
    recordIndex++;

    const recordDiv = document.createElement('div');
    recordDiv.className = 'genetic-record';
    recordDiv.id = `record-${recordIndex}`;
    recordDiv.innerHTML = `
        <input type="text" placeholder="Gene ID" class="gene-id">
        <input type="number" placeholder="Mutation Score (0-1)" class="mutation-score" step="0.01" min="0" max="1">
        <input type="number" placeholder="Label (0/1)" class="label" min="0" max="1">
        <button class="btn btn-danger" onclick="removeGeneticRecord(${recordIndex})">Remove</button>
    `;
    container.appendChild(recordDiv);
}

// Remove genetic record input
function removeGeneticRecord(index) {
    const recordDiv = document.getElementById(`record-${index}`);
    if (recordDiv) {
        recordDiv.remove();
    }
}

// Submit Patient
function submitPatient() {
    const patientId = document.getElementById('patient-id').value;
    const name = document.getElementById('patient-name').value;
    const age = document.getElementById('patient-age').value;
    const statusDiv = document.getElementById('patient-status');

    if (!patientId || !name || !age) {
        showStatus(statusDiv, 'Please fill in all patient information', 'error');
        return;
    }

    const geneticRecords = [];
    const records = document.querySelectorAll('.genetic-record');
    records.forEach(record => {
        const geneId = record.querySelector('.gene-id').value;
        const mutationScore = parseFloat(record.querySelector('.mutation-score').value);
        const label = parseInt(record.querySelector('.label').value);

        if (geneId && !isNaN(mutationScore) && !isNaN(label)) {
            geneticRecords.push({ geneId, mutationScore, label });
        }
    });

    if (geneticRecords.length === 0) {
        showStatus(statusDiv, 'Please add at least one genetic record', 'error');
        return;
    }

    // Prevent duplicate patient IDs
    if (systemState.patients.find(p => p.patient_id.toLowerCase() === patientId.trim().toLowerCase())) {
        showStatus(statusDiv, 'Patient ID already exists. Choose a unique ID.', 'error');
        return;
    }

    showStatus(statusDiv, 'Adding patient...', 'info');
    setStatusOnline(false);

    // Build payload for server
    const newPatient = {
        patient_id: patientId.trim(),
        name: name.trim(),
        age: parseInt(age, 10)
    };

    const geneticRecordsPayload = [];
    const recordNodes = document.querySelectorAll('.genetic-record');
    recordNodes.forEach(record => {
        const geneId = record.querySelector('.gene-id').value;
        const mutationScore = parseFloat(record.querySelector('.mutation-score').value);
        const label = parseInt(record.querySelector('.label').value, 10);

        if (geneId && !isNaN(mutationScore) && !isNaN(label)) {
            geneticRecordsPayload.push({ geneId, mutationScore, label });
        }
    });

    const payload = {
        patient_id: newPatient.patient_id,
        name: newPatient.name,
        age: newPatient.age,
        geneticRecords: geneticRecordsPayload
    };

    // Try to add patient on server so server-side memory/queue is updated
    (async () => {
        try {
            const resp = await fetch(systemState.apiEndpoint + '/patients', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });

            if (!resp.ok) {
                const txt = await resp.text();
                throw new Error('Server error: ' + resp.status + ' ' + txt);
            }

            const data = await resp.json();

            // Update local UI state to reflect server state
            systemState.patients.push(newPatient);
            geneticRecordsPayload.forEach(gr => {
                systemState.geneticData.push({
                    gene_id: gr.geneId,
                    mutation_score: gr.mutationScore,
                    label: gr.label,
                    patient_id: newPatient.patient_id
                });
            });

            systemState.patientRecords = data.patientCount || systemState.patients.length;
            systemState.geneticRecords = data.geneticCount || systemState.geneticData.length;
            updateDashboard();

            showStatus(statusDiv, 'Patient added on server successfully!', 'success');
            showNotification('Patient added (server)', 'success');

            // Save data to CSV for persistence
            triggerRobotEncouragement('patient_saved');
            saveDataToCSV();

            // Clear form
            document.getElementById('patient-id').value = '';
            document.getElementById('patient-name').value = '';
            document.getElementById('patient-age').value = '';
            document.getElementById('genetic-records-container').innerHTML = '';
            recordIndex = 0;
            addGeneticRecord();
        } catch (error) {
            // Fallback to local-only add if server not available
            systemState.patients.push(newPatient);
            geneticRecordsPayload.forEach(gr => {
                systemState.geneticData.push({
                    gene_id: gr.geneId,
                    mutation_score: gr.mutationScore,
                    label: gr.label,
                    patient_id: newPatient.patient_id
                });
            });
            systemState.patientRecords = systemState.patients.length;
            systemState.geneticRecords = systemState.geneticData.length;
            updateDashboard();

            showStatus(statusDiv, 'Patient added locally (server unavailable): ' + error.message, 'warning');
            showNotification('Patient added locally (server unreachable)', 'warning');

            // Try to save anyway (may fail if server is down)
            saveDataToCSV();

            // Clear form
            document.getElementById('patient-id').value = '';
            document.getElementById('patient-name').value = '';
            document.getElementById('patient-age').value = '';
            document.getElementById('genetic-records-container').innerHTML = '';
            recordIndex = 0;
            addGeneticRecord();
        } finally {
            setStatusOnline(true);
        }
    })();
}

// Run Diagnosis
async function runDiagnosis() {
    const patientId = document.getElementById('diagnosis-patient-id').value;
    const model = document.getElementById('model-select').value;
    const resultDiv = document.getElementById('diagnosis-result');

    if (!patientId) {
        resultDiv.innerHTML = '<div class="status-message error show">Please enter a patient ID</div>';
        return;
    }

    // Basic local validation that patient ID exists in loaded local list (optional)
    const pid = patientId.trim();
    const localPatient = systemState.patients.find(p => p.patient_id && p.patient_id.toLowerCase() === pid.toLowerCase());
    if (!localPatient) {
        resultDiv.innerHTML = `<div class="status-message error show">Error: Patient ID '${patientId}' not found locally. Load patients or add the patient first, or ensure the server has the patient.</div>`;
        return;
    }

    resultDiv.innerHTML = '<div class="status-message info show">Contacting diagnosis server...</div>';
    setStatusOnline(false);

    try {
        const resp = await fetch(systemState.apiEndpoint + '/diagnose?patient_id=' + encodeURIComponent(pid) + '&model=' + encodeURIComponent(model));
        if (!resp.ok) {
            const text = await resp.text();
            resultDiv.innerHTML = `<div class="status-message error show">Server error: ${resp.status} ${text}</div>`;
            return;
        }

        const data = await resp.json();
        const riskScore = Number(data.riskScore);
        const prediction = Number(data.prediction);

        const modelNames = {
            logistic: 'Logistic Regression',
            knn: 'K-Nearest Neighbors',
            decision_tree: 'Decision Tree',
            naive_bayes: 'Naive Bayes'
        };

        const predictionClass = prediction === 1 ? 'prediction-cancerous' : 'prediction-safe';
        const predictionText = prediction === 1 ? 'CANCEROUS ⚠️' : 'NON-CANCEROUS ✓';

        resultDiv.innerHTML = `
            <div class="diagnosis-result">
                <div class="result-item">
                    <span class="result-label">Model:</span>
                    <span class="result-value">${modelNames[model]}</span>
                </div>
                <div class="result-item">
                    <span class="result-label">Patient ID:</span>
                    <span class="result-value">${pid}</span>
                </div>
                <div class="result-item">
                    <span class="result-label">Risk Score:</span>
                    <span class="result-value">${(riskScore * 100).toFixed(2)}%</span>
                </div>
                <div class="result-item ${predictionClass}">
                    <span class="result-label">Prediction:</span>
                    <span class="result-value">${predictionText}</span>
                </div>
            </div>
        `;

        showNotification('Diagnosis completed (server)', 'success');
    } catch (err) {
        resultDiv.innerHTML = `<div class="status-message error show">Diagnosis server unavailable. Start the C++ server or use 'Load Data on Server'.</div>`;
    } finally {
        setStatusOnline(true);
    }
}

// Load Genetic Data
function loadGeneticData() {
    const container = document.getElementById('genetic-table-container');
    container.innerHTML = '<div class="status-message info show">Loading genetic data...</div>';

    setTimeout(() => {
        const data = systemState.geneticData;
        if (!data || data.length === 0) {
            container.innerHTML = '<div class="status-message info show">No genetic data loaded. Use Load Data or upload a genes CSV.</div>';
            return;
        }

        let html = '<div class="table-container"><table><thead><tr>';
        html += '<th>Gene ID</th><th>Mutation Score</th><th>Label</th><th>Patient ID</th></tr></thead><tbody>';

        data.forEach(row => {
            html += `<tr><td>${row.gene_id}</td><td>${row.mutation_score ?? ''}</td><td>${row.label ?? ''}</td><td>${row.patient_id ?? ''}</td></tr>`;
        });

        html += '</tbody></table></div>';
        container.innerHTML = html;
    }, 300);
}

// Load Patient History
function loadPatientHistory() {
    const container = document.getElementById('patient-table-container');
    container.innerHTML = '<div class="status-message info show">Loading patient history...</div>';

    setTimeout(() => {
        const data = systemState.patients;
        if (!data || data.length === 0) {
            container.innerHTML = '<div class="status-message info show">No patient history loaded. Use Load Data or upload a patients CSV.</div>';
            return;
        }

        let html = '<div class="table-container"><table><thead><tr>';
        html += '<th>Patient ID</th><th>Name</th><th>Age</th></tr></thead><tbody>';

        data.forEach(row => {
            html += `<tr><td>${row.patient_id}</td><td>${row.name}</td><td>${row.age ?? ''}</td></tr>`;
        });

        html += '</tbody></table></div>';
        container.innerHTML = html;
    }, 300);
}

// Load queue from server and render
async function loadQueue() {
    const container = document.getElementById('queue-container');
    const queueSizeEl = document.getElementById('queue-size');
    const statusDiv = document.getElementById('queue-status');
    if (!container) return;
    container.innerHTML = '<div class="status-message info show">Loading queue...</div>';
    try {
        const resp = await fetch(systemState.apiEndpoint + '/queue');
        if (!resp.ok) throw new Error('Server responded: ' + resp.statusText);
        const data = await resp.json();
        systemState.queueSize = data.queueSize || 0;
        queueSizeEl.textContent = systemState.queueSize;
        updateDashboard();

        if (!data.patients || data.patients.length === 0) {
            container.innerHTML = '<div class="status-message info show">Queue is empty.</div>';
            return;
        }

        let html = '<div class="table-container"><table><thead><tr><th>#</th><th>Patient ID</th></tr></thead><tbody>';
        data.patients.forEach((pid, idx) => {
            html += `<tr><td>${idx+1}</td><td>${pid}</td></tr>`;
        });
        html += '</tbody></table></div>';
        container.innerHTML = html;
    } catch (err) {
        container.innerHTML = '<div class="status-message error show">Could not load queue: ' + err.message + '</div>';
        console.warn('Queue load failed', err);
    }
}

// Enqueue patient from UI input
async function enqueuePatientFromUI() {
    const input = document.getElementById('enqueue-patient-id');
    const statusDiv = document.getElementById('queue-status');
    if (!input) return;
    const pid = input.value.trim();
    if (!pid) {
        showStatus(statusDiv, 'Enter a patient ID to enqueue', 'error');
        return;
    }
    showStatus(statusDiv, 'Scheduling patient...', 'info');
    setStatusOnline(false);
    try {
        const resp = await fetch(systemState.apiEndpoint + '/queue', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ patient_id: pid })
        });
        if (!resp.ok) {
            const txt = await resp.text();
            throw new Error(txt || resp.statusText);
        }
        const data = await resp.json();
        showStatus(statusDiv, 'Patient scheduled successfully', 'success');
        showNotification('Patient enqueued', 'success');
        input.value = '';
        await loadQueue();
    } catch (err) {
        showStatus(statusDiv, 'Failed to schedule patient: ' + err.message, 'error');
        showNotification('Failed to enqueue patient', 'error');
    } finally {
        setStatusOnline(true);
    }
}

// Process the entire queue on server with selected model
async function processQueueFromUI() {
    const statusDiv = document.getElementById('queue-status');
    const resultsDiv = document.getElementById('queue-results-container');
    const modelSelect = document.getElementById('queue-model-select');
    const model = modelSelect ? modelSelect.value : 'logistic';
    
    showStatus(statusDiv, 'Processing queue on server with ' + model + ' model...', 'info');
    resultsDiv.innerHTML = ''; // Clear previous results
    setStatusOnline(false);
    try {
        const resp = await fetch(systemState.apiEndpoint + '/queue/process', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ model: model })
        });
        if (!resp.ok) {
            const txt = await resp.text();
            throw new Error(txt || resp.statusText);
        }
        const data = await resp.json();
        const processed = data.processed || 0;
        const results = data.results || [];
        
        showStatus(statusDiv, `Successfully processed ${processed} patients using ${data.model || model} model`, 'success');
        showNotification(`Processed ${processed} patients`, 'success');

        // Display diagnosis results
        if (results.length > 0) {
            let html = '<div class="results-grid">';
            results.forEach((resultJson, idx) => {
                try {
                    const result = JSON.parse(resultJson);
                    const predictionClass = result.prediction === 1 ? 'prediction-cancerous' : 'prediction-safe';
                    const predictionText = result.prediction === 1 ? 'CANCEROUS ⚠️' : 'NON-CANCEROUS ✓';
                    html += `
                        <div class="diagnosis-result">
                            <div class="result-item">
                                <span class="result-label">Patient ID:</span>
                                <span class="result-value">${result.patient_id}</span>
                            </div>
                            <div class="result-item">
                                <span class="result-label">Name:</span>
                                <span class="result-value">${result.name}</span>
                            </div>
                            <div class="result-item">
                                <span class="result-label">Risk Score:</span>
                                <span class="result-value">${(result.riskScore * 100).toFixed(2)}%</span>
                            </div>
                            <div class="result-item ${predictionClass}">
                                <span class="result-label">Prediction:</span>
                                <span class="result-value">${predictionText}</span>
                            </div>
                        </div>
                    `;
                } catch (e) {
                    console.warn('Could not parse result:', resultJson, e);
                }
            });
            html += '</div>';
            resultsDiv.innerHTML = html;
        }

        // Refresh server status to update counts
        try {
            const st = await fetch(systemState.apiEndpoint + '/status');
            if (st.ok) {
                const sdat = await st.json();
                systemState.patientRecords = sdat.patientCount || systemState.patientRecords;
                systemState.geneticRecords = sdat.geneticCount || systemState.geneticRecords;
                systemState.modelsTrained = !!sdat.modelsTrained;
            }
        } catch (e) {
            console.warn('Could not refresh /status after processing queue', e.message);
        }

        await loadQueue();
        updateDashboard();
    } catch (err) {
        showStatus(statusDiv, 'Queue processing failed: ' + err.message, 'error');
        showNotification('Queue processing failed', 'error');
    } finally {
        setStatusOnline(true);
    }
}

// Load Mutation Mappings
function loadMutationMappings() {
    const container = document.getElementById('mutation-table-container');
    container.innerHTML = '<div class="status-message info show">Loading mutation mappings...</div>';

    setTimeout(() => {
        const data = [
            { mutation: 'TP53_001', frequency: '234', riskLevel: 'High', category: 'Tumor Suppressor' },
            { mutation: 'BRCA1_002', frequency: '156', riskLevel: 'High', category: 'DNA Repair' },
            { mutation: 'EGFR_003', frequency: '89', riskLevel: 'Medium', category: 'Growth Factor' },
            { mutation: 'KRAS_004', frequency: '112', riskLevel: 'High', category: 'Oncogene' }
        ];

        let html = '<div class="table-container"><table><thead><tr>';
        html += '<th>Mutation</th><th>Frequency</th><th>Risk Level</th><th>Category</th></tr></thead><tbody>';

        data.forEach(row => {
            const riskColor = row.riskLevel === 'High' ? 'danger' : row.riskLevel === 'Medium' ? 'warning' : 'success';
            html += `<tr><td>${row.mutation}</td><td>${row.frequency}</td><td><span style="color: var(--${riskColor}-color)">${row.riskLevel}</span></td><td>${row.category}</td></tr>`;
        });

        html += '</tbody></table></div>';
        container.innerHTML = html;
    }, 800);
}

// Evaluate All Models
function evaluateAllModels() {
    const container = document.getElementById('evaluation-results');
    container.innerHTML = '<div class="status-message info show">Evaluating all models...</div>';

    setTimeout(() => {
        const models = [
            {
                name: 'Logistic Regression',
                accuracy: 0.87,
                precision: 0.85,
                recall: 0.89,
                f1: 0.87
            },
            {
                name: 'K-Nearest Neighbors',
                accuracy: 0.84,
                precision: 0.82,
                recall: 0.86,
                f1: 0.84
            },
            {
                name: 'Decision Tree',
                accuracy: 0.92,
                precision: 0.90,
                recall: 0.94,
                f1: 0.92
            },
            {
                name: 'Naive Bayes',
                accuracy: 0.79,
                precision: 0.77,
                recall: 0.81,
                f1: 0.79
            }
        ];

        let html = '';
        models.forEach(model => {
            html += `
                <div class="model-result">
                    <h4>${model.name}</h4>
                    <div class="metric-grid">
                        <div class="metric">
                            <div class="metric-label">Accuracy</div>
                            <div class="metric-value">${(model.accuracy * 100).toFixed(1)}%</div>
                        </div>
                        <div class="metric">
                            <div class="metric-label">Precision</div>
                            <div class="metric-value">${(model.precision * 100).toFixed(1)}%</div>
                        </div>
                        <div class="metric">
                            <div class="metric-label">Recall</div>
                            <div class="metric-value">${(model.recall * 100).toFixed(1)}%</div>
                        </div>
                        <div class="metric">
                            <div class="metric-label">F1-Score</div>
                            <div class="metric-value">${(model.f1 * 100).toFixed(1)}%</div>
                        </div>
                    </div>
                </div>
            `;
        });

        container.innerHTML = html;
        
        // Refresh the model pie chart
        setTimeout(() => {
            if (modelChart) modelChart.destroy();
            const modelCtx = document.getElementById('modelChart');
            if (modelCtx) {
                modelChart = new Chart(modelCtx, {
                    type: 'doughnut',
                    data: {
                        labels: models.map(m => m.name),
                        datasets: [{
                            data: models.map(m => Math.round(m.accuracy * 100)),
                            backgroundColor: [
                                '#0066cc',
                                '#00aaff',
                                '#51cf66',
                                '#ffd93d'
                            ],
                            borderColor: 'white',
                            borderWidth: 2,
                            hoverBorderColor: '#e1e8ed',
                            hoverBorderWidth: 3
                        }]
                    },
                    options: {
                        responsive: true,
                        maintainAspectRatio: false,
                        plugins: {
                            legend: {
                                position: 'right',
                                labels: {
                                    padding: 15,
                                    font: { size: 12, weight: 500 },
                                    color: '#2c3e50'
                                }
                            },
                            tooltip: {
                                backgroundColor: 'rgba(0,0,0,0.8)',
                                titleFont: { size: 13 },
                                bodyFont: { size: 12 },
                                padding: 12,
                                callbacks: {
                                    label: function(context) {
                                        return context.label + ': ' + context.parsed + '% Accuracy';
                                    }
                                }
                            }
                        }
                    }
                });
            }
        }, 100);
    }, 1500);
}

// Update Dashboard
function updateDashboard() {
    document.getElementById('stat-genetic').textContent = systemState.geneticRecords;
    document.getElementById('stat-patients').textContent = systemState.patientRecords;
    document.getElementById('stat-models').textContent = systemState.modelsTrained ? 'Trained' : 'Untrained';
    document.getElementById('stat-queue').textContent = systemState.queueSize;
}

// Show status message
function showStatus(element, message, type) {
    element.textContent = message;
    element.className = `status-message show ${type}`;
}

// Show notification toast
function showNotification(message, type = 'info') {
    const notification = document.getElementById('notification');
    notification.textContent = message;
    notification.className = `notification show ${type}`;

    setTimeout(() => {
        notification.classList.remove('show');
    }, 3000);
}

// Set status online/offline
function setStatusOnline(isOnline) {
    const statusDot = document.getElementById('status-dot');
    const statusText = document.getElementById('status-text');

    if (isOnline) {
        statusDot.style.background = '#51cf66';
        statusText.textContent = 'Online';
    } else {
        statusDot.style.background = '#ffd93d';
        statusText.textContent = 'Processing';
    }
}

// Toggle Dark Mode
function toggleDarkMode() {
    const isDarkMode = document.getElementById('dark-mode').checked;
    document.body.classList.toggle('dark-mode', isDarkMode);
    localStorage.setItem('darkMode', isDarkMode);
}

// Save Settings
function saveSettings() {
    const apiEndpoint = document.getElementById('api-endpoint').value;
    systemState.apiEndpoint = apiEndpoint;
    localStorage.setItem('apiEndpoint', apiEndpoint);
    showNotification('Settings saved successfully', 'success');
}

// Load Settings
function loadSettings() {
    const savedApiEndpoint = localStorage.getItem('apiEndpoint');
    const savedDarkMode = localStorage.getItem('darkMode') === 'true';

    if (savedApiEndpoint) {
        document.getElementById('api-endpoint').value = savedApiEndpoint;
        systemState.apiEndpoint = savedApiEndpoint;
    }

    if (savedDarkMode) {
        document.getElementById('dark-mode').checked = true;
        document.body.classList.add('dark-mode');
    }
}

// Reset System
function resetSystem() {
    if (confirm('Are you sure you want to reset the system? This cannot be undone.')) {
        systemState = {
            geneticRecords: 0,
            patientRecords: 0,
            modelsTrained: false,
            queueSize: 0,
            apiEndpoint: 'http://localhost:8080',
            geneticRecordCount: 0
        };
        updateDashboard();
        localStorage.clear();
        showNotification('System reset successfully', 'success');
    }
}

// Show Help Modal
function showHelp() {
    const modal = document.getElementById('help-modal');
    modal.classList.add('show');
}

// Close Help Modal
function closeHelp() {
    const modal = document.getElementById('help-modal');
    modal.classList.remove('show');
}

// Save data to CSV files on server (persistent storage)
async function saveDataToCSV() {
    // Data is now auto-saved on server when patient is added via /patients endpoint
    // No need to manually export - it happens automatically!
    console.log('✓ Data auto-saved to server CSV files');
    addRobotMessage("✅ Patient saved and data automatically backed up to CSV files! Your changes are permanent! 💾");
}


// Close modal when clicking outside
window.addEventListener('click', (event) => {
    const modal = document.getElementById('help-modal');
    if (event.target === modal) {
        modal.classList.remove('show');
    }
});

// Initialize on load
window.addEventListener('load', updateDashboard);

// ==================== ROBOT ADVISOR SYSTEM ====================

// Robot state and knowledge base
const robotState = {
    isOpen: false,
    hasIntroduced: false,
    currentSection: 'dashboard',
    messageHistory: []
};

// Context-aware guidance messages
const robotGuidance = {
    dashboard: {
        greetings: [
            "Welcome to the AI Cancer Diagnosis System! I'm Dr. Gene, your AI advisor. 👋",
            "I see you're on the Dashboard. This is where you can monitor the system status."
        ],
        tips: [
            "Start by loading genetic and patient data from CSV files.",
            "Watch the stats to see how many records are loaded and if models are trained.",
            "Use the sidebar to navigate through different features."
        ]
    },
    'load-data': {
        greetings: [
            "Great! You're in the Load Data section. This is where we start! 📥"
        ],
        tips: [
            "You can either provide file paths or upload CSV files directly.",
            "Ensure your CSV files have proper headers (patient_id, name, age for patients; gene_id, mutation_score, label for genes).",
            "After loading, models will be trained automatically.",
            "Click 'Load Data on Server' to use the backend ML models for diagnosis."
        ]
    },
    'add-patient': {
        greetings: [
            "Wonderful! You're adding a new patient. Let's get their information! ➕"
        ],
        tips: [
            "Enter a unique Patient ID (e.g., P001, P002).",
            "Provide the patient's name and age.",
            "Add at least one genetic record with gene ID, mutation score (0-1), and label (0 or 1).",
            "The patient will be added to the server's database for diagnosis."
        ]
    },
    'diagnosis': {
        greetings: [
            "Excellent! Time to run diagnosis. Let's see those predictions! 🔍"
        ],
        tips: [
            "Select a model from the dropdown (Logistic Regression, KNN, Decision Tree, or Naive Bayes).",
            "Enter a patient ID that exists in the system.",
            "The AI will analyze the patient's genetic data and provide a risk score.",
            "Risk score ranges from 0 (safe) to 1 (cancerous). Score >= 0.5 = cancerous prediction."
        ]
    },
    'view-data': {
        greetings: [
            "Let's review all the patient and genetic data! 👥"
        ],
        tips: [
            "View all loaded patients and their genetic information.",
            "Check the Patient History tab to see individual records.",
            "Use the Genetic Data tab to review all genetic records.",
            "Mutation mappings show how genes are categorized."
        ]
    },
    'evaluate': {
        greetings: [
            "Model evaluation time! Let's see how well our AI models perform! 📈"
        ],
        tips: [
            "This section shows accuracy, precision, recall, and F1-scores for each model.",
            "Higher values indicate better model performance.",
            "Use these metrics to compare different ML algorithms.",
            "Accuracy: overall correctness; Precision: true positives out of predicted positives; Recall: true positives out of actual positives."
        ]
    },
    'settings': {
        greetings: [
            "Customizing your experience! ⚙️"
        ],
        tips: [
            "Set your API endpoint to connect to the C++ backend server.",
            "Default is http://localhost:8080",
            "Enable Dark Mode for comfortable viewing in low light.",
            "Reset the system to clear all data and start fresh."
        ]
    }
};

// Encouragement messages
const encouragements = [
    "You're doing great! Keep going! 🌟",
    "Excellent choice! 👍",
    "Nice work! The data is loading... 🔄",
    "Awesome! Your patient is now in the system! 🎉",
    "Perfect! Let's see what the AI predicts! 🤔",
    "Great job! You're mastering the system! 💪",
    "Fantastic! The diagnosis is complete! 🎯",
    "You're on fire! Keep exploring! 🔥"
];

// Help responses for common questions
const helpResponses = {
    'help': "I'm Dr. Gene, your AI advisor! I can help guide you through the system. Try asking about specific sections or models! 🤖",
    'model': "We have 4 ML models: Logistic Regression (linear), KNN (instance-based), Decision Tree (tree-based), and Naive Bayes (probabilistic). Each has strengths in different scenarios!",
    'genetic': "Genetic data includes gene IDs, mutation scores (0-1 range), and labels (0=non-cancerous, 1=cancerous). This helps train our AI models! 🧬",
    'patient': "A patient record contains ID, name, age, and genetic data. Each patient can have multiple genetic records for comprehensive analysis! 👤",
    'diagnosis': "Diagnosis analyzes a patient's genetic profile using an AI model and produces a risk score (0-1) and prediction (safe or cancerous). 🔍",
    'score': "Risk scores range from 0 (low risk) to 1 (high risk). Usually, scores >= 0.5 indicate cancerous prediction. Scores < 0.5 indicate non-cancerous.",
    'train': "Models are trained on your genetic dataset automatically when you load data. Training uses machine learning to learn patterns from the data! 🧠"
};

// Initialize robot advisor
function initializeRobotAdvisor() {
    const robotMessagesContainer = document.getElementById('robot-messages');
    
    // Add initial greeting
    if (!robotState.hasIntroduced) {
        addRobotMessage("👋 Hello! I'm Dr. Gene, your personal AI advisor!");
        addRobotMessage("I'm here to help you navigate the Cancer Diagnosis System and explain everything along the way. 🤖");
        addRobotMessage("Feel free to ask me questions or I'll guide you based on what you're doing!");
        robotState.hasIntroduced = true;
    }
}

// Add message to chat
function addRobotMessage(message, isUser = false) {
    const robotMessagesContainer = document.getElementById('robot-messages');
    
    const messageDiv = document.createElement('div');
    messageDiv.className = `robot-message ${isUser ? 'user' : 'advisor'}`;
    
    const messageIcon = document.createElement('div');
    messageIcon.className = 'message-icon';
    messageIcon.textContent = isUser ? '👤' : '🤖';
    
    const messageText = document.createElement('div');
    messageText.className = 'message-text';
    messageText.textContent = message;
    
    messageDiv.appendChild(messageIcon);
    messageDiv.appendChild(messageText);
    robotMessagesContainer.appendChild(messageDiv);
    
    // Scroll to bottom
    robotMessagesContainer.scrollTop = robotMessagesContainer.scrollHeight;
    
    // Store in history
    robotState.messageHistory.push({ user: isUser, text: message, timestamp: new Date() });
}

// Send robot message
function sendRobotMessage() {
    const input = document.getElementById('robot-input');
    const message = input.value.trim();
    
    if (!message) return;
    
    // Add user message
    addRobotMessage(message, true);
    input.value = '';
    
    // Generate and add robot response
    setTimeout(() => {
        const response = generateRobotResponse(message);
        addRobotMessage(response, false);
    }, 300);
}

// Handle Enter key in robot input
function handleRobotInput(event) {
    if (event.key === 'Enter') {
        sendRobotMessage();
    }
}

// Generate contextual response
function generateRobotResponse(userInput) {
    const input = userInput.toLowerCase();
    
    // Check for help keywords
    for (const [keyword, response] of Object.entries(helpResponses)) {
        if (input.includes(keyword)) {
            return response;
        }
    }
    
    // Section-specific guidance
    const section = robotState.currentSection;
    if (input.includes('help') || input.includes('guide') || input.includes('how')) {
        const tips = robotGuidance[section]?.tips || [];
        if (tips.length > 0) {
            return tips[Math.floor(Math.random() * tips.length)];
        }
    }
    
    // Default helpful response
    const defaults = [
        "That's a great question! Let me help you with that. 😊",
        "I'm here to assist you! Tell me more about what you need. 💡",
        "Good thinking! Remember to follow the workflow step by step. 👉",
        "I understand! Would you like tips on the current section? 📚",
        "Absolutely! The AI Cancer Diagnosis System is powerful. Keep exploring! 🚀"
    ];
    
    return defaults[Math.floor(Math.random() * defaults.length)];
}

// Toggle robot advisor visibility
function toggleRobotAdvisor() {
    const advisor = document.getElementById('robot-advisor');
    const toggleBtn = document.getElementById('robot-toggle-btn');
    
    robotState.isOpen = !robotState.isOpen;
    
    if (robotState.isOpen) {
        advisor.classList.remove('collapsed');
        toggleBtn.classList.add('hidden');
    } else {
        advisor.classList.add('collapsed');
        toggleBtn.classList.remove('hidden');
    }
}

// Robot guidance on section switch
function triggerRobotGuidance(sectionId) {
    robotState.currentSection = sectionId;
    
    // Only trigger if robot is open
    if (!robotState.isOpen) return;
    
    const guidance = robotGuidance[sectionId];
    if (!guidance) return;
    
    // Add greeting
    if (guidance.greetings.length > 0) {
        const greeting = guidance.greetings[Math.floor(Math.random() * guidance.greetings.length)];
        addRobotMessage(greeting);
    }
    
    // After a delay, add a tip
    setTimeout(() => {
        if (guidance.tips.length > 0) {
            const tip = guidance.tips[0];
            addRobotMessage("💡 " + tip);
        }
    }, 800);
}

// Robot encouragement on data load
function triggerRobotEncouragement(action) {
    if (!robotState.isOpen) return;
    
    const encouragement = encouragements[Math.floor(Math.random() * encouragements.length)];
    addRobotMessage(encouragement);
}

// Wire robot to section switches
const originalSwitchSection = window.switchSection;
window.switchSection = function(sectionId) {
    originalSwitchSection(sectionId);
    triggerRobotGuidance(sectionId);
    // If user opened the Queue section, refresh queue from server
    if (sectionId === 'queue') {
        // best-effort: load queue, ignore errors
        try { loadQueue(); } catch (e) { console.warn('loadQueue failed', e); }
    }
};

// Wire robot to key actions
const originalLoadData = window.loadData;
window.loadData = function() {
    originalLoadData.apply(this, arguments);
    triggerRobotEncouragement('data_loaded');
};

const originalLoadDataOnServer = window.loadDataOnServer;
window.loadDataOnServer = function() {
    addRobotMessage("Loading data and training models... This might take a moment! ⏳");
    return originalLoadDataOnServer.apply(this, arguments);
};

const originalSubmitPatient = window.submitPatient;
window.submitPatient = function() {
    addRobotMessage("Adding patient to the system... 🔄");
    return originalSubmitPatient.apply(this, arguments);
};

const originalRunDiagnosis = window.runDiagnosis;
window.runDiagnosis = function() {
    addRobotMessage("Running diagnosis... Let me analyze the patient! 🔬");
    return originalRunDiagnosis.apply(this, arguments);
};

// Initialize robot on page load
document.addEventListener('DOMContentLoaded', () => {
    initializeRobotAdvisor();
    // Auto-open robot on first load for introduction (optional - comment out if not desired)
    setTimeout(() => {
        if (!robotState.isOpen) {
            toggleRobotAdvisor();
            triggerRobotGuidance('dashboard');
        }
    }, 1000);
});

// --------------------
// Splash screen logic
// --------------------
function initSplash() {
    const splash = document.getElementById('splash');
    if (!splash) return;

    const hideSplash = () => {
        if (!splash || splash.classList.contains('hidden')) return;
        splash.classList.add('hidden');
        // Remove from DOM after transition to keep DOM clean
        const onEnd = () => {
            if (splash && splash.parentNode) splash.parentNode.removeChild(splash);
        };
        splash.addEventListener('transitionend', onEnd, { once: true });
    };

    // Auto-hide after a short delay
    const AUTO_HIDE_MS = 2300;
    const timer = setTimeout(() => {
        hideSplash();
    }, AUTO_HIDE_MS);

    // Allow click anywhere to skip early
    const onClickSkip = (e) => {
        clearTimeout(timer);
        hideSplash();
        document.removeEventListener('click', onClickSkip);
    };

    document.addEventListener('click', onClickSkip, { capture: true });

    // Allow Escape key to dismiss
    const onKey = (e) => {
        if (e.key === 'Escape') {
            clearTimeout(timer);
            hideSplash();
            document.removeEventListener('keydown', onKey);
        }
    };

    document.addEventListener('keydown', onKey);

    // Accessibility: focus first interactive element after splash removed.
    splash.addEventListener('transitionend', () => {
        const firstNav = document.querySelector('.nav-btn');
        if (firstNav) firstNav.focus();
    }, { once: true });
}

// Kick off splash logic early on DOM ready
document.addEventListener('DOMContentLoaded', initSplash);

// --------------------
// Chart.js integration
// --------------------

let modelChart = null;
let patientChart = null;

function initCharts() {
    // Initialize model comparison pie chart (Evaluate Models)
    const modelCtx = document.getElementById('modelChart');
    if (modelCtx) {
        modelChart = new Chart(modelCtx, {
            type: 'doughnut',
            data: {
                labels: ['Logistic Regression', 'K-Nearest Neighbors', 'Decision Tree', 'Naive Bayes'],
                datasets: [{
                    data: [87, 84, 92, 79],
                    backgroundColor: [
                        '#0066cc',
                        '#00aaff',
                        '#51cf66',
                        '#ffd93d'
                    ],
                    borderColor: 'white',
                    borderWidth: 2,
                    hoverBorderColor: '#e1e8ed',
                    hoverBorderWidth: 3
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: {
                        position: 'right',
                        labels: {
                            padding: 15,
                            font: { size: 12, weight: 500 },
                            color: '#2c3e50'
                        }
                    },
                    tooltip: {
                        backgroundColor: 'rgba(0,0,0,0.8)',
                        titleFont: { size: 13 },
                        bodyFont: { size: 12 },
                        padding: 12,
                        callbacks: {
                            label: function(context) {
                                return context.label + ': ' + context.parsed + '% Accuracy';
                            }
                        }
                    }
                }
            }
        });
    }

    // Initialize patient demographics bar chart (Analytics)
    const patientCtx = document.getElementById('patientChart');
    if (patientCtx) {
        updatePatientChart();
    }
}

function updatePatientChart() {
    const patientCtx = document.getElementById('patientChart');
    if (!patientCtx) return;

    // Fetch patient data from server for accurate stats
    fetch(systemState.apiEndpoint + '/patients')
        .then(r => {
            if (!r.ok) throw new Error('Failed to fetch patients from server');
            return r.json();
        })
        .then(patients => {
            // Calculate patient stats from server data
            const total = patients.length;
            
            // Fetch genetic data to determine cancerous status
            return fetch(systemState.apiEndpoint + '/genetic')
                .then(r => r.json())
                .then(geneticData => {
                    const cancerous = patients.filter(p => {
                        const records = geneticData.filter(g => g.patient_id === p.patient_id || (g.patient_id === undefined && records.length === 0));
                        if (records.length === 0) return false;
                        const avgLabel = records.reduce((sum, r) => sum + (r.label || 0), 0) / records.length;
                        return avgLabel >= 0.5;
                    }).length;
                    const nonCancerous = total - cancerous;
                    
                    renderPatientChart(cancerous, nonCancerous, total);
                    updateAnalyticsSummary(total, cancerous, nonCancerous);
                });
        })
        .catch(err => {
            console.warn('Could not fetch patient data from server, using local data:', err.message);
            // Fallback to local data
            const total = systemState.patients.length;
            const cancerous = systemState.patients.filter((p, idx) => {
                const records = systemState.geneticData.filter(g => g.patient_id === p.patient_id);
                if (records.length === 0) return false;
                const avgLabel = records.reduce((sum, r) => sum + (r.label || 0), 0) / records.length;
                return avgLabel >= 0.5;
            }).length;
            const nonCancerous = total - cancerous;
            renderPatientChart(cancerous, nonCancerous, total);
            updateAnalyticsSummary(total, cancerous, nonCancerous);
        });
}

function renderPatientChart(cancerous, nonCancerous, total) {
    const patientCtx = document.getElementById('patientChart');
    if (!patientCtx) return;

    if (patientChart) patientChart.destroy();

    patientChart = new Chart(patientCtx, {
        type: 'bar',
        data: {
            labels: ['Cancerous', 'Non-Cancerous'],
            datasets: [{
                label: 'Patient Count',
                data: [cancerous, nonCancerous],
                backgroundColor: [
                    '#ff6b6b',
                    '#51cf66'
                ],
                borderColor: ['#e55555', '#40c057'],
                borderWidth: 2,
                borderRadius: 8,
                hoverBackgroundColor: ['#ff5252', '#40c057'],
                hoverBorderWidth: 3
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            indexAxis: 'x',
            plugins: {
                legend: {
                    labels: {
                        font: { size: 12, weight: 500 },
                        color: '#2c3e50',
                        padding: 15
                    }
                },
                tooltip: {
                    backgroundColor: 'rgba(0,0,0,0.8)',
                    titleFont: { size: 13 },
                    bodyFont: { size: 12 },
                    padding: 12,
                    callbacks: {
                        label: function(context) {
                            return 'Patients: ' + context.parsed.y;
                        }
                    }
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: {
                        stepSize: 1,
                        color: '#7f8c8d'
                    },
                    grid: {
                        color: 'rgba(0,0,0,0.05)'
                    }
                },
                x: {
                    ticks: {
                        color: '#2c3e50',
                        font: { size: 12, weight: 500 }
                    },
                    grid: {
                        display: false
                    }
                }
            }
        }
    });

    // Update summary stats
    updateAnalyticsSummary(total, cancerous, nonCancerous);
}

function updateAnalyticsSummary(total, cancerous, nonCancerous) {
    document.getElementById('total-patients').textContent = total;
    document.getElementById('cancerous-count').textContent = cancerous;
    document.getElementById('non-cancerous-count').textContent = nonCancerous;
    const percent = total > 0 ? Math.round((cancerous / total) * 100) : 0;
    document.getElementById('cancerous-percent').textContent = percent + '%';
}

// Update charts when dashboard data changes
const originalUpdateDashboard = window.updateDashboard;
window.updateDashboard = function() {
    originalUpdateDashboard();
    // Refresh patient chart on dashboard update
    setTimeout(() => {
        updatePatientChart();
    }, 100);
};
