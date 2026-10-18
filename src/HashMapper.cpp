#include "../headers/HashMapper.h"
#include <iostream>
#include <iomanip>

HashMapper::HashMapper() {
    // Initialize default label mappings
    labelToCategoryMap[0] = "Non-Cancerous";
    labelToCategoryMap[1] = "Cancerous";
}

void HashMapper::addMutationMapping(const std::string& mutation, double riskScore) {
    mutationToRiskMap[mutation] = riskScore;
}

double HashMapper::getRiskScore(const std::string& mutation) const {
    auto it = mutationToRiskMap.find(mutation);
    if (it != mutationToRiskMap.end()) {
        return it->second;
    }
    return 0.0; // Default risk score if mutation not found
}

bool HashMapper::hasMutation(const std::string& mutation) const {
    return mutationToRiskMap.find(mutation) != mutationToRiskMap.end();
}

void HashMapper::setLabelCategory(int label, const std::string& category) {
    labelToCategoryMap[label] = category;
}

std::string HashMapper::getCategory(int label) const {
    auto it = labelToCategoryMap.find(label);
    if (it != labelToCategoryMap.end()) {
        return it->second;
    }
    return "Unknown";
}

void HashMapper::buildMutationMap(const std::vector<std::string>& mutations, 
                                  const std::vector<double>& riskScores) {
    if (mutations.size() != riskScores.size()) {
        throw std::runtime_error("Mutations and risk scores must have the same size");
    }
    
    for (size_t i = 0; i < mutations.size(); ++i) {
        mutationToRiskMap[mutations[i]] = riskScores[i];
    }
}

void HashMapper::displayMappings() const {
    std::cout << "\n=== Mutation to Risk Mappings ===" << std::endl;
    for (const auto& pair : mutationToRiskMap) {
        std::cout << "Mutation: " << std::setw(15) << pair.first 
                  << " -> Risk Score: " << std::fixed << std::setprecision(4) << pair.second << std::endl;
    }
    std::cout << "\n=== Label to Category Mappings ===" << std::endl;
    for (const auto& pair : labelToCategoryMap) {
        std::cout << "Label: " << pair.first << " -> Category: " << pair.second << std::endl;
    }
    std::cout << "==================================\n" << std::endl;
}

size_t HashMapper::size() const {
    return mutationToRiskMap.size();
}

