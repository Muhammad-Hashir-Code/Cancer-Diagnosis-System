#include "../headers/Patient.h"
#include <iostream>
#include <iomanip>

Patient::Patient() : patientId(""), name(""), age(0), riskScore(0.0), prediction(0) {}

Patient::Patient(const std::string& id, const std::string& name, int age) 
    : patientId(id), name(name), age(age), riskScore(0.0), prediction(0) {}

std::string Patient::getPatientId() const {
    return patientId;
}

std::string Patient::getName() const {
    return name;
}

int Patient::getAge() const {
    return age;
}

std::vector<GeneticData> Patient::getGeneticData() const {
    return geneticData;
}

double Patient::getRiskScore() const {
    return riskScore;
}

int Patient::getPrediction() const {
    return prediction;
}

void Patient::setPatientId(const std::string& id) {
    patientId = id;
}

void Patient::setName(const std::string& name) {
    this->name = name;
}

void Patient::setAge(int age) {
    this->age = age;
}

void Patient::setRiskScore(double score) {
    riskScore = score;
}

void Patient::setPrediction(int pred) {
    prediction = pred;
}

void Patient::addGeneticData(const GeneticData& data) {
    geneticData.push_back(data);
}

std::vector<double> Patient::getMutationScores() const {
    std::vector<double> scores;
    for (const auto& data : geneticData) {
        scores.push_back(data.getMutationScore());
    }
    return scores;
}

void Patient::display() const {
    std::cout << "\n=== Patient Information ===" << std::endl;
    std::cout << "Patient ID: " << patientId << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Age: " << age << std::endl;
    std::cout << "Risk Score: " << std::fixed << std::setprecision(4) << riskScore << std::endl;
    std::cout << "Prediction: " << (prediction == 1 ? "Cancerous" : "Non-Cancerous") << std::endl;
    std::cout << "Genetic Data Count: " << geneticData.size() << std::endl;
    if (!geneticData.empty()) {
        std::cout << "\nGenetic Data:" << std::endl;
        for (const auto& data : geneticData) {
            std::cout << "  ";
            data.display();
        }
    }
    std::cout << "===========================\n" << std::endl;
}

