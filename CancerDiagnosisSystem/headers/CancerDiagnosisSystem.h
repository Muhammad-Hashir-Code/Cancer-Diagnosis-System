#ifndef CANCER_DIAGNOSIS_SYSTEM_H
#define CANCER_DIAGNOSIS_SYSTEM_H

#include "Patient.h"
#include "GeneticData.h"
#include "DataPreprocessor.h"
#include "HashMapper.h"
#include "DecisionTreeClassifier.h"
#include "LogisticRegressionModel.h"
#include "KNNClassifier.h"
#include "NaiveBayesClassifier.h"
#include "EvaluationMetrics.h"
#include <vector>
#include <queue>
#include <list>
#include <string>
#include <memory>

/**
 * @struct PatientNode
 * @brief Node for linked list storing patient history
 */
struct PatientNode {
    Patient patient;
    std::shared_ptr<PatientNode> next;
    
    PatientNode(const Patient& p) : patient(p), next(nullptr) {}
};

/**
 * @class CancerDiagnosisSystem
 * @brief Main controller class for the cancer diagnosis system
 */
class CancerDiagnosisSystem {
private:
    // Data structures
    std::vector<GeneticData> geneticDataArray;
    std::shared_ptr<PatientNode> patientHistoryHead; // Linked list
    std::queue<Patient> testRequestQueue; // Queue for test scheduling
    HashMapper mutationMapper;
    
    // Preprocessing
    DataPreprocessor preprocessor;
    
    // ML Models
    std::unique_ptr<LogisticRegressionModel> logisticModel;
    std::unique_ptr<KNNClassifier> knnModel;
    std::unique_ptr<DecisionTreeClassifier> decisionTreeModel;
    std::unique_ptr<NaiveBayesClassifier> naiveBayesModel;
    
    // Evaluation
    EvaluationMetrics evaluator;
    
    // Training data
    std::vector<std::vector<double>> X_train;
    std::vector<int> y_train;
    bool modelsTrained;
    
    // Helper functions
    void loadGeneticDataFromFile(const std::string& filename);
    void loadPatientsFromFile(const std::string& filename);
    void addPatientToHistory(const Patient& patient);
    void prepareTrainingData();
    void trainAllModels();
    std::vector<double> extractFeatures(const Patient& patient) const;
    
public:
    CancerDiagnosisSystem();
    ~CancerDiagnosisSystem();
    
    // Data acquisition
    void loadData(const std::string& genesFile, const std::string& patientsFile);
    void addPatient(const Patient& patient);
    void addGeneticData(const GeneticData& data);
    
    // Test scheduling
    void scheduleTest(const Patient& patient);
    void processTestQueue();
    // Process test queue and return number processed
    int processTestQueueAndReturnCount();
    // Process test queue with selected model and return diagnosis results
    std::vector<std::string> processTestQueueWithModel(ModelType model);
    // Query queue state
    size_t getQueueSize() const;
    std::vector<std::string> getQueuedPatientIds() const;
    
    // Diagnosis
    enum class ModelType { LOGISTIC, KNN, DECISION_TREE, NAIVE_BAYES };
    double diagnosePatient(const Patient& patient, ModelType model);
    int predictPatient(const Patient& patient, ModelType model);
    
    // Evaluation
    void evaluateModels(const std::vector<Patient>& testPatients);
    void displayModelMetrics(ModelType model, const std::vector<int>& yTrue, 
                            const std::vector<int>& yPred) const;
    
    // Display
    void displayGeneticData() const;
    void displayPatientHistory() const;
    void displayMutationMappings() const;
    void displayDecisionTree() const;
    
    // Utility
    size_t getGeneticDataCount() const;
    size_t getPatientCount() const;
    bool areModelsTrained() const;
    bool getPatientById(const std::string& patientId, Patient& outPatient) const;
    
    // Data export
    std::vector<Patient> getAllPatients() const;
    std::vector<GeneticData> getAllGeneticData() const;
    void saveDataToFiles(const std::string& genesFile, const std::string& patientsFile) const;
};

#endif // CANCER_DIAGNOSIS_SYSTEM_H


