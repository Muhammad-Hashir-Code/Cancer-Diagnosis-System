#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include <vector>
#include "GeneticData.h"

/**
 * @class Patient
 * @brief Represents a patient with genetic data and medical history
 */
class Patient {
private:
    std::string patientId;
    std::string name;
    int age;
    std::vector<GeneticData> geneticData;
    double riskScore;
    int prediction; // 0 = non-cancerous, 1 = cancerous
    
public:
    // Constructors
    Patient();
    Patient(const std::string& id, const std::string& name, int age);
    
    // Getters
    std::string getPatientId() const;
    std::string getName() const;
    int getAge() const;
    std::vector<GeneticData> getGeneticData() const;
    double getRiskScore() const;
    int getPrediction() const;
    
    // Setters
    void setPatientId(const std::string& id);
    void setName(const std::string& name);
    void setAge(int age);
    void setRiskScore(double score);
    void setPrediction(int pred);
    
    // Data management
    void addGeneticData(const GeneticData& data);
    std::vector<double> getMutationScores() const;
    
    // Utility
    void display() const;
};

#endif // PATIENT_H


