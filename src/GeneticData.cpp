#include "../headers/GeneticData.h"
#include <iostream>
#include <iomanip>

GeneticData::GeneticData() : geneId(""), mutationScore(0.0), label(0) {}

GeneticData::GeneticData(const std::string& id, double score, int lbl) 
    : geneId(id), mutationScore(score), label(lbl) {}

std::string GeneticData::getGeneId() const {
    return geneId;
}

double GeneticData::getMutationScore() const {
    return mutationScore;
}

int GeneticData::getLabel() const {
    return label;
}

void GeneticData::setGeneId(const std::string& id) {
    geneId = id;
}

void GeneticData::setMutationScore(double score) {
    mutationScore = score;
}

void GeneticData::setLabel(int lbl) {
    label = lbl;
}

void GeneticData::display() const {
    std::cout << "Gene ID: " << geneId 
              << ", Mutation Score: " << std::fixed << std::setprecision(4) << mutationScore
              << ", Label: " << (label == 1 ? "Cancerous" : "Non-Cancerous") << std::endl;
}

