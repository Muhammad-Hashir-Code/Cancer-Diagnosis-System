#ifndef GENETIC_DATA_H
#define GENETIC_DATA_H

#include <string>
#include <vector>

/**
 * @class GeneticData
 * @brief Represents genetic test data including gene sequences and mutation scores
 */
class GeneticData {
private:
    std::string geneId;
    double mutationScore;
    int label; // 0 = non-cancerous, 1 = cancerous
    
public:
    // Constructors
    GeneticData();
    GeneticData(const std::string& id, double score, int lbl);
    
    // Getters
    std::string getGeneId() const;
    double getMutationScore() const;
    int getLabel() const;
    
    // Setters
    void setGeneId(const std::string& id);
    void setMutationScore(double score);
    void setLabel(int lbl);
    
    // Utility
    void display() const;
};

#endif // GENETIC_DATA_H


