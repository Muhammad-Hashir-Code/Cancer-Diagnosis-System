#include "CancerDiagnosisSystem.h"
#include <iostream>
#include <string>
#include <limits>
#include <iomanip>
#include <stdexcept>

void displayMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  AI Cancer Diagnosis System" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Load Data from Files" << std::endl;
    std::cout << "2. Add Patient Genetic Data" << std::endl;
    std::cout << "3. Run Diagnosis (Select Model)" << std::endl;
    std::cout << "4. View Result Summary" << std::endl;
    std::cout << "5. Display Genetic Data" << std::endl;
    std::cout << "6. Display Patient History" << std::endl;
    std::cout << "7. Display Mutation Mappings (Hash Table)" << std::endl;
    std::cout << "8. Display Decision Tree Structure" << std::endl;
    std::cout << "9. Schedule Test (Add to Queue)" << std::endl;
    std::cout << "10. Process Test Queue" << std::endl;
    std::cout << "11. Evaluate All Models" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Enter your choice: ";
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Helper function to trim whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

int getIntInput() {
    int value;
    while (true) {
        if (std::cin >> value) {
            return value;
        }

        if (std::cin.eof()) {
            throw std::runtime_error("Input stream closed unexpectedly while reading integer.");
        }

        std::cout << "Invalid input. Please enter a number: ";
        clearInput();
    }
}

double getDoubleInput() {
    double value;
    while (true) {
        if (std::cin >> value) {
            return value;
        }

        if (std::cin.eof()) {
            throw std::runtime_error("Input stream closed unexpectedly while reading number.");
        }

        std::cout << "Invalid input. Please enter a number: ";
        clearInput();
    }
}

CancerDiagnosisSystem::ModelType selectModel() {
    std::cout << "\nSelect ML Model:" << std::endl;
    std::cout << "1. Logistic Regression" << std::endl;
    std::cout << "2. K-Nearest Neighbors (KNN)" << std::endl;
    std::cout << "3. Decision Tree" << std::endl;
    std::cout << "4. Naive Bayes" << std::endl;
    std::cout << "Enter choice: ";
    
    int choice = getIntInput();
    clearInput();
    
    switch (choice) {
        case 1: return CancerDiagnosisSystem::ModelType::LOGISTIC;
        case 2: return CancerDiagnosisSystem::ModelType::KNN;
        case 3: return CancerDiagnosisSystem::ModelType::DECISION_TREE;
        case 4: return CancerDiagnosisSystem::ModelType::NAIVE_BAYES;
        default:
            std::cout << "Invalid choice. Using Logistic Regression." << std::endl;
            return CancerDiagnosisSystem::ModelType::LOGISTIC;
    }
}

std::string getModelName(CancerDiagnosisSystem::ModelType model) {
    switch (model) {
        case CancerDiagnosisSystem::ModelType::LOGISTIC:
            return "Logistic Regression";
        case CancerDiagnosisSystem::ModelType::KNN:
            return "K-Nearest Neighbors";
        case CancerDiagnosisSystem::ModelType::DECISION_TREE:
            return "Decision Tree";
        case CancerDiagnosisSystem::ModelType::NAIVE_BAYES:
            return "Naive Bayes";
        default:
            return "Unknown";
    }
}

void addPatientManually(CancerDiagnosisSystem& system) {
    std::cout << "\n=== Add Patient Genetic Data ===" << std::endl;
    
    std::string patientId, name;
    int age;
    
    std::cout << "Enter Patient ID: ";
    clearInput();
    std::getline(std::cin, patientId);
    
    std::cout << "Enter Patient Name: ";
    std::getline(std::cin, name);
    
    std::cout << "Enter Age: ";
    age = getIntInput();
    clearInput();
    
    Patient patient(patientId, name, age);
    
    std::cout << "How many genetic data records to add? ";
    int numRecords = getIntInput();
    clearInput();
    
    for (int i = 0; i < numRecords; ++i) {
        std::string geneId;
        double mutationScore;
        int label;
        
        std::cout << "\nRecord " << (i + 1) << ":" << std::endl;
        std::cout << "Enter Gene ID: ";
        std::getline(std::cin, geneId);
        
        std::cout << "Enter Mutation Score: ";
        mutationScore = getDoubleInput();
        clearInput();
        
        std::cout << "Enter Label (0 = Non-Cancerous, 1 = Cancerous): ";
        label = getIntInput();
        clearInput();
        
        GeneticData data(geneId, mutationScore, label);
        patient.addGeneticData(data);
    }
    
    system.addPatient(patient);
    std::cout << "Patient added successfully!" << std::endl;
}

void runDiagnosis(CancerDiagnosisSystem& system) {
    if (!system.areModelsTrained()) {
        std::cout << "\nError: Models not trained. Please load data first (Option 1)." << std::endl;
        return;
    }
    
    std::cout << "\n=== Run Diagnosis ===" << std::endl;
    
    std::string patientId;
    std::cout << "Enter Patient ID to diagnose: ";
    std::getline(std::cin, patientId);
    patientId = trim(patientId); // Trim whitespace
    
    if (patientId.empty()) {
        std::cout << "\nError: Patient ID cannot be empty." << std::endl;
        return;
    }
    
    Patient patient;
    if (!system.getPatientById(patientId, patient)) {
        std::cout << "\nError: Patient ID '" << patientId << "' not found." << std::endl;
        std::cout << "Available patients: " << system.getPatientCount() << std::endl;
        std::cout << "Please add the patient first (Option 2) or load patient data (Option 1)." << std::endl;
        std::cout << "Tip: Use Option 6 to view all patient IDs in the system." << std::endl;
        return;
    }
    
    CancerDiagnosisSystem::ModelType model = selectModel();
    
    double riskScore = system.diagnosePatient(patient, model);
    int prediction = system.predictPatient(patient, model);
    
    std::cout << "\n=== Diagnosis Result ===" << std::endl;
    std::cout << "Model: " << getModelName(model) << std::endl;
    std::cout << "Patient ID: " << patient.getPatientId() << std::endl;
    std::cout << "Patient Name: " << patient.getName() << std::endl;
    std::cout << "Risk Score: " << std::fixed << std::setprecision(4) << riskScore 
              << " (" << (riskScore * 100) << "%)" << std::endl;
    std::cout << "Prediction: " << (prediction == 1 ? "CANCEROUS" : "NON-CANCEROUS") << std::endl;
    std::cout << "========================" << std::endl;
}

void viewResultSummary(CancerDiagnosisSystem& system) {
    std::cout << "\n=== System Summary ===" << std::endl;
    std::cout << "Genetic Data Records: " << system.getGeneticDataCount() << std::endl;
    std::cout << "Patient Records: " << system.getPatientCount() << std::endl;
    std::cout << "Models Trained: " << (system.areModelsTrained() ? "Yes" : "No") << std::endl;
    std::cout << "=====================" << std::endl;
}

void evaluateModels(CancerDiagnosisSystem& system) {
    if (!system.areModelsTrained()) {
        std::cout << "\nError: Models not trained. Please load data first." << std::endl;
        return;
    }
    
    // Get test patients from history
    // For demonstration, we'll use a subset of loaded data
    std::vector<Patient> testPatients;
    
    // Create some test patients
    Patient test1("TEST_001", "Test Patient 1", 45);
    GeneticData data1("GENE_TEST_001", 0.6, 1);
    test1.addGeneticData(data1);
    
    Patient test2("TEST_002", "Test Patient 2", 35);
    GeneticData data2("GENE_TEST_002", 0.3, 0);
    test2.addGeneticData(data2);
    
    testPatients.push_back(test1);
    testPatients.push_back(test2);
    
    system.evaluateModels(testPatients);
}

int main() {
    try {
        CancerDiagnosisSystem system;
        int choice;
        
        std::cout << "Welcome to AI Cancer Diagnosis System!" << std::endl;
        std::cout << "This system uses machine learning to predict cancer risk from genetic data." << std::endl;
        
        do {
            displayMenu();
            choice = getIntInput();
            clearInput();
            
            switch (choice) {
                case 1: {
                    std::string genesFile, patientsFile;
                    std::cout << "\n=== Load Data from Files ===" << std::endl;
                    std::cout << "Enter genes CSV file path (press Enter for default: data/genes.csv): ";
                    std::getline(std::cin, genesFile);
                    if (genesFile.empty()) {
                        genesFile = "data/genes.csv";
                        std::cout << "Using default: " << genesFile << std::endl;
                    }
                    
                    std::cout << "Enter patients CSV file path (press Enter for default: data/patients.csv): ";
                    std::getline(std::cin, patientsFile);
                    if (patientsFile.empty()) {
                        patientsFile = "data/patients.csv";
                        std::cout << "Using default: " << patientsFile << std::endl;
                    }
                    
                    std::cout << "\nLoading data..." << std::endl;
                    system.loadData(genesFile, patientsFile);
                    std::cout << "Data loaded successfully!" << std::endl;
                    break;
                }
                case 2:
                    addPatientManually(system);
                    break;
                case 3:
                    runDiagnosis(system);
                    break;
                case 4:
                    viewResultSummary(system);
                    break;
                case 5:
                    system.displayGeneticData();
                    break;
                case 6:
                    system.displayPatientHistory();
                    break;
                case 7:
                    system.displayMutationMappings();
                    break;
                case 8:
                    if (system.areModelsTrained()) {
                        system.displayDecisionTree();
                    } else {
                        std::cout << "\nError: Models not trained. Please load data first." << std::endl;
                    }
                    break;
                case 9: {
                    std::cout << "\n=== Schedule Test ===" << std::endl;
                    std::string patientId, name;
                    int age;
                    
                    std::cout << "Enter Patient ID: ";
                    std::getline(std::cin, patientId);
                    
                    std::cout << "Enter Patient Name: ";
                    std::getline(std::cin, name);
                    
                    std::cout << "Enter Age: ";
                    age = getIntInput();
                    clearInput();
                    
                    Patient patient(patientId, name, age);
                    GeneticData data("GENE_QUEUE_001", 0.5, 0);
                    patient.addGeneticData(data);
                    
                    system.scheduleTest(patient);
                    std::cout << "Test scheduled successfully!" << std::endl;
                    break;
                }
                case 10:
                    system.processTestQueue();
                    break;
                case 11:
                    evaluateModels(system);
                    break;
                case 0:
                    std::cout << "\nThank you for using AI Cancer Diagnosis System!" << std::endl;
                    break;
                default:
                    std::cout << "\nInvalid choice. Please try again." << std::endl;
            }
        } while (choice != 0);
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nFatal error: " << e.what() << std::endl;
        return 1;
    }
}

