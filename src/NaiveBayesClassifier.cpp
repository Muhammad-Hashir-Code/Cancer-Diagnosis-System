#include "../headers/NaiveBayesClassifier.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <numeric>

NaiveBayesClassifier::NaiveBayesClassifier() : isTrained(false) {}

void NaiveBayesClassifier::calculateClassStatistics(
    const std::vector<std::vector<double>>& X, 
    const std::vector<int>& y) {
    
    // Find unique classes
    std::map<int, bool> classMap;
    for (int label : y) {
        classMap[label] = true;
    }
    
    uniqueClasses.clear();
    for (const auto& pair : classMap) {
        uniqueClasses.push_back(pair.first);
    }
    
    if (X.empty()) return;
    size_t nFeatures = X[0].size();
    
    // Initialize statistics for each class
    for (int cls : uniqueClasses) {
        classPrior[cls] = 0.0;
        classMean[cls] = std::vector<double>(nFeatures, 0.0);
        classStd[cls] = std::vector<double>(nFeatures, 1.0); // Default std dev
    }
    
    // Calculate class priors
    size_t n = y.size();
    for (int label : y) {
        classPrior[label] += 1.0;
    }
    for (int cls : uniqueClasses) {
        classPrior[cls] /= n;
    }
    
    // Calculate mean for each feature per class
    std::map<int, int> classCounts;
    for (size_t i = 0; i < n; ++i) {
        int label = y[i];
        classCounts[label]++;
        
        for (size_t j = 0; j < nFeatures; ++j) {
            classMean[label][j] += X[i][j];
        }
    }
    
    for (int cls : uniqueClasses) {
        int count = classCounts[cls];
        if (count > 0) {
            for (size_t j = 0; j < nFeatures; ++j) {
                classMean[cls][j] /= count;
            }
        }
    }
    
    // Calculate standard deviation for each feature per class
    for (int cls : uniqueClasses) {
        int count = classCounts[cls];
        if (count <= 1) continue;
        
        std::vector<double> variance(nFeatures, 0.0);
        for (size_t i = 0; i < n; ++i) {
            if (y[i] == cls) {
                for (size_t j = 0; j < nFeatures; ++j) {
                    double diff = X[i][j] - classMean[cls][j];
                    variance[j] += diff * diff;
                }
            }
        }
        
        for (size_t j = 0; j < nFeatures; ++j) {
            classStd[cls][j] = std::sqrt(variance[j] / (count - 1));
            // Avoid zero standard deviation
            if (classStd[cls][j] < 1e-10) {
                classStd[cls][j] = 1.0;
            }
        }
    }
}

double NaiveBayesClassifier::calculateProbability(double x, double mean, double std) const {
    // Gaussian probability density function
    const double PI = 3.14159265358979323846;
    double exponent = -0.5 * std::pow((x - mean) / std, 2);
    return (1.0 / (std * std::sqrt(2.0 * PI))) * std::exp(exponent);
}

double NaiveBayesClassifier::calculateClassProbability(
    const std::vector<double>& features, int classLabel) const {
    
    // Start with class prior
    double probability = std::log(classPrior.at(classLabel) + 1e-10);
    
    // Multiply by feature probabilities (using log to avoid underflow)
    const auto& means = classMean.at(classLabel);
    const auto& stds = classStd.at(classLabel);
    
    for (size_t i = 0; i < features.size(); ++i) {
        double prob = calculateProbability(features[i], means[i], stds[i]);
        probability += std::log(prob + 1e-10);
    }
    
    return probability;
}

void NaiveBayesClassifier::fit(const std::vector<std::vector<double>>& X, 
                               const std::vector<int>& y) {
    if (X.empty() || y.empty()) {
        throw std::runtime_error("Training data is empty");
    }
    
    if (X.size() != y.size()) {
        throw std::runtime_error("X and y must have the same size");
    }
    
    calculateClassStatistics(X, y);
    isTrained = true;
}

std::vector<int> NaiveBayesClassifier::predict(
    const std::vector<std::vector<double>>& X) const {
    if (!isTrained) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    std::vector<int> predictions;
    predictions.reserve(X.size());
    
    for (const auto& sample : X) {
        predictions.push_back(predictSingle(sample));
    }
    
    return predictions;
}

int NaiveBayesClassifier::predictSingle(const std::vector<double>& features) const {
    if (!isTrained) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    int bestClass = uniqueClasses[0];
    double bestProbability = calculateClassProbability(features, bestClass);
    
    for (size_t i = 1; i < uniqueClasses.size(); ++i) {
        int cls = uniqueClasses[i];
        double prob = calculateClassProbability(features, cls);
        
        if (prob > bestProbability) {
            bestProbability = prob;
            bestClass = cls;
        }
    }
    
    return bestClass;
}

std::vector<double> NaiveBayesClassifier::predictProbability(
    const std::vector<std::vector<double>>& X) const {
    if (!isTrained) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    std::vector<double> probabilities;
    probabilities.reserve(X.size());
    
    for (const auto& sample : X) {
        probabilities.push_back(predictProbabilitySingle(sample));
    }
    
    return probabilities;
}

double NaiveBayesClassifier::predictProbabilitySingle(
    const std::vector<double>& features) const {
    if (!isTrained) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    // Calculate probabilities for each class
    std::vector<double> classProbs;
    double maxProb = std::numeric_limits<double>::lowest();
    
    for (int cls : uniqueClasses) {
        double prob = calculateClassProbability(features, cls);
        classProbs.push_back(prob);
        if (prob > maxProb) {
            maxProb = prob;
        }
    }
    
    // Normalize probabilities using softmax
    double sum = 0.0;
    for (double& prob : classProbs) {
        prob = std::exp(prob - maxProb); // Subtract max for numerical stability
        sum += prob;
    }
    
    // Find probability of class 1 (cancerous)
    for (size_t i = 0; i < uniqueClasses.size(); ++i) {
        if (uniqueClasses[i] == 1) {
            return classProbs[i] / sum;
        }
    }
    
    return 0.0;
}

bool NaiveBayesClassifier::getIsTrained() const {
    return isTrained;
}

std::map<int, double> NaiveBayesClassifier::getClassPrior() const {
    return classPrior;
}

