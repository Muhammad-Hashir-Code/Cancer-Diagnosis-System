#ifndef NAIVE_BAYES_CLASSIFIER_H
#define NAIVE_BAYES_CLASSIFIER_H

#include <vector>
#include <map>
#include <cmath>

/**
 * @class NaiveBayesClassifier
 * @brief Implements Naive Bayes algorithm for probabilistic classification
 */
class NaiveBayesClassifier {
private:
    std::map<int, double> classPrior; // P(class)
    std::map<int, std::vector<double>> classMean; // Mean for each feature per class
    std::map<int, std::vector<double>> classStd; // Std dev for each feature per class
    std::vector<int> uniqueClasses;
    bool isTrained;
    
    // Helper functions
    double calculateProbability(double x, double mean, double std) const;
    double calculateClassProbability(const std::vector<double>& features, int classLabel) const;
    void calculateClassStatistics(const std::vector<std::vector<double>>& X, 
                                  const std::vector<int>& y);
    
public:
    NaiveBayesClassifier();
    
    // Training and prediction
    void fit(const std::vector<std::vector<double>>& X, const std::vector<int>& y);
    std::vector<int> predict(const std::vector<std::vector<double>>& X) const;
    int predictSingle(const std::vector<double>& features) const;
    std::vector<double> predictProbability(const std::vector<std::vector<double>>& X) const;
    double predictProbabilitySingle(const std::vector<double>& features) const;
    
    // Utility
    bool getIsTrained() const;
    std::map<int, double> getClassPrior() const;
};

#endif // NAIVE_BAYES_CLASSIFIER_H


