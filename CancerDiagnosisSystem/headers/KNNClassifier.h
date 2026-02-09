#ifndef KNN_CLASSIFIER_H
#define KNN_CLASSIFIER_H

#include <vector>
#include <utility>
#include <algorithm>

/**
 * @class KNNClassifier
 * @brief Implements K-Nearest Neighbors algorithm for classification
 */
class KNNClassifier {
private:
    std::vector<std::vector<double>> X_train;
    std::vector<int> y_train;
    int k;
    bool isTrained;
    
    // Helper functions
    double euclideanDistance(const std::vector<double>& a, 
                            const std::vector<double>& b) const;
    int majorityVote(const std::vector<std::pair<double, int>>& neighbors) const;
    std::vector<std::pair<double, int>> findKNearest(const std::vector<double>& sample) const;
    
public:
    KNNClassifier(int k = 5);
    
    // Training and prediction
    void fit(const std::vector<std::vector<double>>& X, const std::vector<int>& y);
    std::vector<int> predict(const std::vector<std::vector<double>>& X) const;
    int predictSingle(const std::vector<double>& sample) const;
    std::vector<double> predictProbability(const std::vector<std::vector<double>>& X) const;
    
    // Parameters
    void setK(int k);
    int getK() const;
    bool getIsTrained() const;
};

#endif // KNN_CLASSIFIER_H


