#include "../headers/LogisticRegressionModel.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

LogisticRegressionModel::LogisticRegressionModel(double learningRate, int maxIterations) 
    : learningRate(learningRate), maxIterations(maxIterations), bias(0.0), isTrained(false) {}

double LogisticRegressionModel::sigmoid(double z) const {
    // Clamp z to prevent overflow
    if (z > 500) z = 500;
    if (z < -500) z = -500;
    return 1.0 / (1.0 + std::exp(-z));
}

double LogisticRegressionModel::rawPredictProbability(const std::vector<double>& features) const {
    if (features.size() != weights.size()) {
        throw std::runtime_error("Feature size mismatch");
    }
    
    double z = bias;
    for (size_t i = 0; i < features.size(); ++i) {
        z += weights[i] * features[i];
    }
    
    return sigmoid(z);
}

double LogisticRegressionModel::predictProbability(const std::vector<double>& features) const {
    if (!isTrained) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    return rawPredictProbability(features);
}

double LogisticRegressionModel::computeLoss(const std::vector<std::vector<double>>& X, 
                                           const std::vector<int>& y) const {
    double loss = 0.0;
    size_t n = X.size();
    
    for (size_t i = 0; i < n; ++i) {
        double prob = rawPredictProbability(X[i]);
        // Log loss with numerical stability
        double y_val = static_cast<double>(y[i]);
        loss -= y_val * std::log(prob + 1e-15) + (1.0 - y_val) * std::log(1.0 - prob + 1e-15);
    }
    
    return loss / n;
}

void LogisticRegressionModel::gradientDescent(const std::vector<std::vector<double>>& X, 
                                              const std::vector<int>& y) {
    size_t n = X.size();
    size_t nFeatures = X[0].size();
    
    // Initialize weights if not already initialized
    if (weights.empty()) {
        weights.resize(nFeatures, 0.0);
        bias = 0.0;
    }
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        // Calculate gradients
        std::vector<double> weightGradients(nFeatures, 0.0);
        double biasGradient = 0.0;
        
        for (size_t i = 0; i < n; ++i) {
            double prediction = rawPredictProbability(X[i]);
            double error = prediction - static_cast<double>(y[i]);
            
            // Update gradients
            for (size_t j = 0; j < nFeatures; ++j) {
                weightGradients[j] += error * X[i][j];
            }
            biasGradient += error;
        }
        
        // Update weights and bias
        for (size_t j = 0; j < nFeatures; ++j) {
            weights[j] -= learningRate * weightGradients[j] / n;
        }
        bias -= learningRate * biasGradient / n;
        
        // Optional: Print progress every 100 iterations
        if ((iter + 1) % 100 == 0) {
            double loss = computeLoss(X, y);
            // Uncomment for debugging
            // std::cout << "Iteration " << (iter + 1) << ", Loss: " << loss << std::endl;
        }
    }
}

void LogisticRegressionModel::fit(const std::vector<std::vector<double>>& X, 
                                  const std::vector<int>& y) {
    if (X.empty() || y.empty()) {
        throw std::runtime_error("Training data is empty");
    }
    
    if (X.size() != y.size()) {
        throw std::runtime_error("X and y must have the same size");
    }
    
    size_t nFeatures = X[0].size();
    for (const auto& sample : X) {
        if (sample.size() != nFeatures) {
            throw std::runtime_error("All samples must have the same number of features");
        }
    }
    
    // Initialize weights
    weights.resize(nFeatures, 0.0);
    bias = 0.0;
    
    // Train using gradient descent
    gradientDescent(X, y);
    
    isTrained = true;
}

std::vector<int> LogisticRegressionModel::predict(const std::vector<std::vector<double>>& X) const {
    std::vector<int> predictions;
    predictions.reserve(X.size());
    
    for (const auto& sample : X) {
        double prob = predictProbability(sample);
        predictions.push_back(prob >= 0.5 ? 1 : 0);
    }
    
    return predictions;
}

int LogisticRegressionModel::predictSingle(const std::vector<double>& features) const {
    double prob = predictProbability(features);
    return prob >= 0.5 ? 1 : 0;
}

std::vector<double> LogisticRegressionModel::predictProbabilityBatch(
    const std::vector<std::vector<double>>& X) const {
    std::vector<double> probabilities;
    probabilities.reserve(X.size());
    
    for (const auto& sample : X) {
        probabilities.push_back(predictProbability(sample));
    }
    
    return probabilities;
}

double LogisticRegressionModel::predictProbabilitySingle(const std::vector<double>& features) const {
    return predictProbability(features);
}

void LogisticRegressionModel::setLearningRate(double rate) {
    learningRate = rate;
}

void LogisticRegressionModel::setMaxIterations(int iterations) {
    maxIterations = iterations;
}

std::vector<double> LogisticRegressionModel::getWeights() const {
    return weights;
}

double LogisticRegressionModel::getBias() const {
    return bias;
}

bool LogisticRegressionModel::getIsTrained() const {
    return isTrained;
}

