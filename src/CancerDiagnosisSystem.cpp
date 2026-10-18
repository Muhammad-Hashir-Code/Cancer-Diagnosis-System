#include "../headers/CancerDiagnosisSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iomanip>


CancerDiagnosisSystem::CancerDiagnosisSystem() 
    : patientHistoryHead(nullptr), modelsTrained(false) {
    // Initialize ML models
    logisticModel = std::make_unique<LogisticRegressionModel>(0.01, 1000);
    knnModel = std::make_unique<KNNClassifier>(5);
    decisionTreeModel = std::make_unique<DecisionTreeClassifier>(10, 2);
    naiveBayesModel = std::make_unique<NaiveBayesClassifier>();
    
    // Initialize mutation mapper with default mappings
    mutationMapper.setLabelCategory(0, "Non-Cancerous");
    mutationMapper.setLabelCategory(1, "Cancerous");
}

CancerDiagnosisSystem::~CancerDiagnosisSystem() {
    // Clean up linked list
    while (patientHistoryHead) {
        auto temp = patientHistoryHead;
        patientHistoryHead = patientHistoryHead->next;
        // Shared pointer will handle deletion automatically
    }
}

void CancerDiagnosisSystem::loadGeneticDataFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    std::string line;
    bool isFirstLine = true;
    
    while (std::getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue; // Skip header
        }
        
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string geneId, mutationScoreStr, labelStr;
        
        if (std::getline(ss, geneId, ',') &&
            std::getline(ss, mutationScoreStr, ',') &&
            std::getline(ss, labelStr, ',')) {
            
            try {
                double mutationScore = std::stod(mutationScoreStr);
                int label = std::stoi(labelStr);
                
                GeneticData data(geneId, mutationScore, label);
                geneticDataArray.push_back(data);
                
                // Add to mutation mapper
                mutationMapper.addMutationMapping(geneId, mutationScore);
            } catch (const std::exception&) {
                std::cerr << "Error parsing line: " << line << std::endl;
            }
        }
    }
    
    file.close();
    std::cout << "Loaded " << geneticDataArray.size() << " genetic data records." << std::endl;
}

void CancerDiagnosisSystem::loadPatientsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    std::string line;
    bool isFirstLine = true;
    size_t patientIndex = 0; // Track patient index for genetic data assignment
    
    while (std::getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue; // Skip header
        }
        
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string patientId, name, ageStr;
        
        if (std::getline(ss, patientId, ',') &&
            std::getline(ss, name, ',') &&
            std::getline(ss, ageStr, ',')) {
            
            try {
                int age = std::stoi(ageStr);
                Patient patient(patientId, name, age);
                
                // Add a subset of genetic data to each patient (not all genes)
                // This ensures each patient has unique genetic profiles for different predictions
                if (!geneticDataArray.empty()) {
                    size_t dataStart = patientIndex % geneticDataArray.size();
                    size_t dataCount = std::min(static_cast<size_t>(5), geneticDataArray.size()); // Each patient gets ~5 genes
                    
                    for (size_t i = 0; i < dataCount; ++i) {
                        size_t idx = (dataStart + i) % geneticDataArray.size();
                        patient.addGeneticData(geneticDataArray[idx]);
                    }
                    patientIndex++;
                }
                
                addPatientToHistory(patient);
            } catch (const std::exception&) {
                std::cerr << "Error parsing line: " << line << std::endl;
            }
        }
    }
    
    file.close();
}

void CancerDiagnosisSystem::loadData(const std::string& genesFile, 
                                     const std::string& patientsFile) {
    std::cout << "\n=== Loading Data ===" << std::endl;

    // Clear existing in-memory data to avoid duplication when loading multiple times
    geneticDataArray.clear();
    // Reset patient history linked list
    patientHistoryHead = nullptr;
    // Clear any pending test requests
    while (!testRequestQueue.empty()) testRequestQueue.pop();
    // Reset mutation mapper and preprocessing state
    mutationMapper = HashMapper();
    preprocessor.reset();
    // Clear training buffers and flags
    X_train.clear();
    y_train.clear();
    modelsTrained = false;

    loadGeneticDataFromFile(genesFile);
    loadPatientsFromFile(patientsFile);
    
    std::cout << "\n=== Data Summary ===" << std::endl;
    std::cout << "Genetic records loaded: " << geneticDataArray.size() << std::endl;
    std::cout << "Patient records loaded: " << getPatientCount() << std::endl;
    
    if (geneticDataArray.empty()) {
        std::cerr << "\n✗ ERROR: No genetic data loaded! Cannot train models." << std::endl;
        std::cerr << "  Please check that " << genesFile << " exists and contains data." << std::endl;
        modelsTrained = false;
        return;
    }
    
    prepareTrainingData();
    trainAllModels();
    
    if (modelsTrained) {
        std::cout << "\n✓ System ready for diagnosis!" << std::endl;
    } else {
        std::cerr << "\n✗ WARNING: Models were not trained successfully!" << std::endl;
    }
}

void CancerDiagnosisSystem::addPatient(const Patient& patient) {
    Patient patientCopy = patient;
    addPatientToHistory(patientCopy);
}

void CancerDiagnosisSystem::addPatientToHistory(const Patient& patient) {
    auto newNode = std::make_shared<PatientNode>(patient);
    newNode->next = patientHistoryHead;
    patientHistoryHead = newNode;
}

void CancerDiagnosisSystem::addGeneticData(const GeneticData& data) {
    geneticDataArray.push_back(data);
    mutationMapper.addMutationMapping(data.getGeneId(), data.getMutationScore());
}

void CancerDiagnosisSystem::scheduleTest(const Patient& patient) {
    testRequestQueue.push(patient);
}

void CancerDiagnosisSystem::processTestQueue() {
    std::cout << "\n=== Processing Test Queue ===" << std::endl;
    int count = 0;
    
    while (!testRequestQueue.empty()) {
        Patient patient = testRequestQueue.front();
        testRequestQueue.pop();
        
        // Perform diagnosis using default model (Logistic Regression)
        double riskScore = diagnosePatient(patient, ModelType::LOGISTIC);
        patient.setRiskScore(riskScore);
        patient.setPrediction(riskScore >= 0.5 ? 1 : 0);
        
        addPatientToHistory(patient);
        count++;
        
        std::cout << "Processed patient: " << patient.getName() 
                  << " - Risk Score: " << std::fixed << std::setprecision(4) << riskScore << std::endl;
    }
    
    std::cout << "Processed " << count << " patients from queue." << std::endl;
    std::cout << "============================\n" << std::endl;
}

int CancerDiagnosisSystem::processTestQueueAndReturnCount() {
    std::cout << "\n=== Processing Test Queue (API) ===" << std::endl;
    int count = 0;

    while (!testRequestQueue.empty()) {
        Patient patient = testRequestQueue.front();
        testRequestQueue.pop();

        double riskScore = diagnosePatient(patient, ModelType::LOGISTIC);
        patient.setRiskScore(riskScore);
        patient.setPrediction(riskScore >= 0.5 ? 1 : 0);

        addPatientToHistory(patient);
        count++;

        std::cout << "Processed patient: " << patient.getName()
                  << " - Risk Score: " << std::fixed << std::setprecision(4) << riskScore << std::endl;
    }

    std::cout << "Processed " << count << " patients from queue." << std::endl;
    std::cout << "============================\n" << std::endl;
    return count;
}

size_t CancerDiagnosisSystem::getQueueSize() const {
    return testRequestQueue.size();
}

std::vector<std::string> CancerDiagnosisSystem::getQueuedPatientIds() const {
    std::vector<std::string> ids;
    auto q = testRequestQueue; // copy
    while (!q.empty()) {
        ids.push_back(q.front().getPatientId());
        q.pop();
    }
    return ids;
}

std::vector<std::string> CancerDiagnosisSystem::processTestQueueWithModel(ModelType model) {
    std::cout << "\n=== Processing Test Queue with Model (API) ===" << std::endl;
    std::vector<std::string> results; // JSON lines for each diagnosis
    int count = 0;

    while (!testRequestQueue.empty()) {
        Patient patient = testRequestQueue.front();
        testRequestQueue.pop();

        double riskScore = diagnosePatient(patient, model);
        int prediction = riskScore >= 0.5 ? 1 : 0;
        patient.setRiskScore(riskScore);
        patient.setPrediction(prediction);

        addPatientToHistory(patient);
        count++;

        // Build JSON-like result string for this patient
        std::ostringstream result;
        result << "{\"patient_id\":\"" << patient.getPatientId() 
               << "\",\"name\":\"" << patient.getName()
               << "\",\"riskScore\":" << std::fixed << std::setprecision(4) << riskScore
               << ",\"prediction\":" << prediction
               << ",\"status\":\"processed\"}"; 
        results.push_back(result.str());

        std::cout << "Processed patient: " << patient.getName()
                  << " - Risk Score: " << std::fixed << std::setprecision(4) << riskScore 
                  << " - Prediction: " << (prediction == 1 ? "CANCEROUS" : "NON-CANCEROUS") << std::endl;
    }

    std::cout << "Processed " << count << " patients from queue." << std::endl;
    std::cout << "====================================\n" << std::endl;
    return results;
}

void CancerDiagnosisSystem::prepareTrainingData() {
    X_train.clear();
    y_train.clear();
    
    if (geneticDataArray.empty()) {
        std::cerr << "\n✗ Warning: No genetic data available for training." << std::endl;
        std::cerr << "  Please ensure genes.csv file contains valid data." << std::endl;
        return;
    }
    
    std::cout << "\nPreparing training data from " << geneticDataArray.size() << " genetic records..." << std::endl;
    
    // Extract features from genetic data
    for (const auto& data : geneticDataArray) {
        std::vector<double> features;
        features.push_back(data.getMutationScore());
        // Can add more features here
        
        X_train.push_back(features);
        y_train.push_back(data.getLabel());
    }
    
    // Preprocess features
    if (!X_train.empty()) {
        std::vector<double> mutationScores;
        for (const auto& features : X_train) {
            mutationScores.push_back(features[0]);
        }
        
        preprocessor.fit(mutationScores);
        std::vector<double> normalized = preprocessor.standardize(mutationScores);
        
        // Update X_train with normalized values
        for (size_t i = 0; i < X_train.size(); ++i) {
            X_train[i][0] = normalized[i];
        }
    }
    
    std::cout << "✓ Prepared " << X_train.size() << " training samples." << std::endl;
}

void CancerDiagnosisSystem::trainAllModels() {
    if (X_train.empty() || y_train.empty()) {
        std::cerr << "Warning: Cannot train models with empty training data." << std::endl;
        std::cerr << "X_train size: " << X_train.size() << ", y_train size: " << y_train.size() << std::endl;
        modelsTrained = false;
        return;
    }
    
    std::cout << "\n=== Training ML Models ===" << std::endl;
    std::cout << "Training samples: " << X_train.size() << std::endl;
    
    modelsTrained = false;  // Reset flag
    
    try {
        std::cout << "Training Logistic Regression..." << std::endl;
        logisticModel->fit(X_train, y_train);
        std::cout << "  ✓ Logistic Regression trained" << std::endl;
        
        std::cout << "Training KNN Classifier..." << std::endl;
        knnModel->fit(X_train, y_train);
        std::cout << "  ✓ KNN trained" << std::endl;
        
        std::cout << "Training Decision Tree..." << std::endl;
        decisionTreeModel->fit(X_train, y_train);
        std::cout << "  ✓ Decision Tree trained" << std::endl;
        
        std::cout << "Training Naive Bayes..." << std::endl;
        naiveBayesModel->fit(X_train, y_train);
        std::cout << "  ✓ Naive Bayes trained" << std::endl;
        
        modelsTrained = true;
        std::cout << "\n✓ All models trained successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error training models: " << e.what() << std::endl;
        modelsTrained = false;
    }
    
    std::cout << "=========================\n" << std::endl;
}

std::vector<double> CancerDiagnosisSystem::extractFeatures(const Patient& patient) const {
    std::vector<double> features;
    
    // Extract mutation scores from patient's genetic data
    std::vector<double> mutationScores = patient.getMutationScores();
    
    if (mutationScores.empty()) {
        // Use average mutation score if patient has no genetic data
        if (!geneticDataArray.empty()) {
            double sum = 0.0;
            for (const auto& data : geneticDataArray) {
                sum += data.getMutationScore();
            }
            features.push_back(sum / geneticDataArray.size());
        } else {
            features.push_back(0.0);
        }
    } else {
        // Use average mutation score
        double sum = 0.0;
        for (double score : mutationScores) {
            sum += score;
        }
        features.push_back(sum / mutationScores.size());
    }
    
    // Normalize features
    if (preprocessor.getIsFitted() && !features.empty()) {
        std::vector<double> normalized = preprocessor.standardize(features);
        features = normalized;
    }
    
    return features;
}

double CancerDiagnosisSystem::diagnosePatient(const Patient& patient, ModelType model) {
    if (!modelsTrained) {
        std::cerr << "Error: Models not trained. Please load data first." << std::endl;
        return 0.0;
    }
    
    std::vector<double> features = extractFeatures(patient);
    std::vector<std::vector<double>> X = {features};
    
    switch (model) {
        case ModelType::LOGISTIC: {
            std::vector<double> probs = logisticModel->predictProbabilityBatch(X);
            return probs[0];
        }
        case ModelType::KNN: {
            std::vector<double> probs = knnModel->predictProbability(X);
            return probs[0];
        }
        case ModelType::DECISION_TREE: {
            std::vector<int> predictions = decisionTreeModel->predict(X);
            // Decision tree doesn't provide probabilities directly
            // Return 1.0 if prediction is 1, 0.0 otherwise
            return predictions[0] == 1 ? 1.0 : 0.0;
        }
        case ModelType::NAIVE_BAYES: {
            double prob = naiveBayesModel->predictProbabilitySingle(features);
            return prob;
        }
        default:
            return 0.0;
    }
}

int CancerDiagnosisSystem::predictPatient(const Patient& patient, ModelType model) {
    double riskScore = diagnosePatient(patient, model);
    return riskScore >= 0.5 ? 1 : 0;
}

void CancerDiagnosisSystem::evaluateModels(const std::vector<Patient>& testPatients) {
    if (!modelsTrained) {
        std::cerr << "Error: Models not trained." << std::endl;
        return;
    }
    
    // Extract test data
    std::vector<std::vector<double>> X_test;
    std::vector<int> y_test;
    
    for (const auto& patient : testPatients) {
        std::vector<double> features = extractFeatures(patient);
        X_test.push_back(features);
        
        // Use patient's prediction or genetic data label if available
        if (!patient.getGeneticData().empty()) {
            y_test.push_back(patient.getGeneticData()[0].getLabel());
        } else {
            y_test.push_back(patient.getPrediction());
        }
    }
    
    // Evaluate each model
    std::cout << "\n=== Model Evaluation ===" << std::endl;
    
    // Logistic Regression
    std::vector<int> y_pred_logistic = logisticModel->predict(X_test);
    std::cout << "\n--- Logistic Regression ---" << std::endl;
    evaluator.displayMetrics(y_test, y_pred_logistic);
    
    // KNN
    std::vector<int> y_pred_knn = knnModel->predict(X_test);
    std::cout << "\n--- KNN Classifier ---" << std::endl;
    evaluator.displayMetrics(y_test, y_pred_knn);
    
    // Decision Tree
    std::vector<int> y_pred_dt = decisionTreeModel->predict(X_test);
    std::cout << "\n--- Decision Tree ---" << std::endl;
    evaluator.displayMetrics(y_test, y_pred_dt);
    
    // Naive Bayes
    std::vector<int> y_pred_nb = naiveBayesModel->predict(X_test);
    std::cout << "\n--- Naive Bayes ---" << std::endl;
    evaluator.displayMetrics(y_test, y_pred_nb);
    
    std::cout << "=====================\n" << std::endl;
}

void CancerDiagnosisSystem::displayModelMetrics(ModelType model, 
                                                const std::vector<int>& yTrue, 
                                                const std::vector<int>& yPred) const {
    evaluator.displayMetrics(yTrue, yPred);
}

void CancerDiagnosisSystem::displayGeneticData() const {
    std::cout << "\n=== Genetic Data (" << geneticDataArray.size() << " records) ===" << std::endl;
    for (const auto& data : geneticDataArray) {
        data.display();
    }
    std::cout << "===========================\n" << std::endl;
}

void CancerDiagnosisSystem::displayPatientHistory() const {
    std::cout << "\n=== Patient History ===" << std::endl;
    int count = 0;
    auto current = patientHistoryHead;
    
    while (current) {
        current->patient.display();
        current = current->next;
        count++;
    }
    
    std::cout << "Total patients: " << count << std::endl;
    std::cout << "=======================\n" << std::endl;
}

void CancerDiagnosisSystem::displayMutationMappings() const {
    mutationMapper.displayMappings();
}

void CancerDiagnosisSystem::displayDecisionTree() const {
    std::cout << "\n=== Decision Tree Structure ===" << std::endl;
    decisionTreeModel->displayTree(decisionTreeModel->getRoot());
    std::cout << "===============================\n" << std::endl;
}

size_t CancerDiagnosisSystem::getGeneticDataCount() const {
    return geneticDataArray.size();
}

size_t CancerDiagnosisSystem::getPatientCount() const {
    size_t count = 0;
    auto current = patientHistoryHead;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

bool CancerDiagnosisSystem::areModelsTrained() const {
    return modelsTrained;
}

bool CancerDiagnosisSystem::getPatientById(const std::string& patientId, Patient& outPatient) const {
    // Helper to trim whitespace
    auto trim = [](const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return std::string("");
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    };
    
    std::string trimmedId = trim(patientId);
    auto current = patientHistoryHead;
    while (current) {
        std::string storedId = trim(current->patient.getPatientId());
        if (storedId == trimmedId) {
            outPatient = current->patient;
            return true;
        }
        current = current->next;
    }
    return false;
}

std::vector<Patient> CancerDiagnosisSystem::getAllPatients() const {
    std::vector<Patient> patients;
    auto current = patientHistoryHead;
    while (current) {
        patients.push_back(current->patient);
        current = current->next;
    }
    return patients;
}

std::vector<GeneticData> CancerDiagnosisSystem::getAllGeneticData() const {
    return geneticDataArray;
}

void CancerDiagnosisSystem::saveDataToFiles(const std::string& genesFile, const std::string& patientsFile) const {
    // Save genetic data to genes file
    std::ofstream genesOut(genesFile);
    if (genesOut.is_open()) {
        genesOut << "Gene_ID,Mutation_Score,Label\n";
        for (const auto& data : geneticDataArray) {
            genesOut << data.getGeneId() << ","
                     << std::fixed << std::setprecision(4) << data.getMutationScore() << ","
                     << data.getLabel() << "\n";
        }
        genesOut.close();
        std::cout << "✓ Saved " << geneticDataArray.size() << " genetic records to " << genesFile << std::endl;
    } else {
        std::cerr << "✗ Error: Could not open " << genesFile << " for writing" << std::endl;
    }
    
    // Save patient data to patients file
    std::ofstream patientsOut(patientsFile);
    if (patientsOut.is_open()) {
        patientsOut << "Patient_ID,Name,Age\n";
        auto current = patientHistoryHead;
        int count = 0;
        while (current) {
            patientsOut << current->patient.getPatientId() << ","
                        << current->patient.getName() << ","
                        << current->patient.getAge() << "\n";
            current = current->next;
            count++;
        }
        patientsOut.close();
        std::cout << "✓ Saved " << count << " patient records to " << patientsFile << std::endl;
    } else {
        std::cerr << "✗ Error: Could not open " << patientsFile << " for writing" << std::endl;
    }
}
