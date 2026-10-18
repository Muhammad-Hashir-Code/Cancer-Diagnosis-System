#ifndef HASH_MAPPER_H
#define HASH_MAPPER_H

#include <map>
#include <string>
#include <vector>

/**
 * @class HashMapper
 * @brief Maps genetic mutation patterns to risk scores using hash maps
 */
class HashMapper {
private:
    std::map<std::string, double> mutationToRiskMap;
    std::map<int, std::string> labelToCategoryMap;
    
public:
    HashMapper();
    
    // Mutation mapping
    void addMutationMapping(const std::string& mutation, double riskScore);
    double getRiskScore(const std::string& mutation) const;
    bool hasMutation(const std::string& mutation) const;
    
    // Label mapping
    void setLabelCategory(int label, const std::string& category);
    std::string getCategory(int label) const;
    
    // Bulk operations
    void buildMutationMap(const std::vector<std::string>& mutations, 
                         const std::vector<double>& riskScores);
    
    // Utility
    void displayMappings() const;
    size_t size() const;
};

#endif // HASH_MAPPER_H


