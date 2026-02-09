#include "../headers/KNNClassifier.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <map>

KNNClassifier::KNNClassifier(int k) : k(k), isTrained(false) {
    if (k <= 0) {
        throw std::runtime_error("K must be positive");
    }
}

double KNNClassifier::euclideanDistance(const std::vector<double>& a, 
                                        const std::vector<double>& b) const {
    if (a.size() != b.size()) {
        throw std::runtime_error("Feature vectors must have the same size");
    }
    
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    
    return std::sqrt(sum);
}

std::vector<std::pair<double, int>> KNNClassifier::findKNearest(
    const std::vector<double>& sample) const {
    if (!isTrained) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    std::vector<std::pair<double, int>> distances;
    distances.reserve(X_train.size());
    
    // Calculate distances to all training samples
    for (size_t i = 0; i < X_train.size(); ++i) {
        double dist = euclideanDistance(sample, X_train[i]);
        distances.push_back({dist, y_train[i]});
    }
    
    // Sort by distance
    std::sort(distances.begin(), distances.end(), 
              [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
                  return a.first < b.first;
              });
    
    // Return k nearest neighbors
    size_t kActual = std::min(static_cast<size_t>(k), distances.size());
    return std::vector<std::pair<double, int>>(distances.begin(), 
                                               distances.begin() + kActual);
}

int KNNClassifier::majorityVote(const std::vector<std::pair<double, int>>& neighbors) const {
    std::map<int, int> votes;
    
    // Count votes (can be weighted by inverse distance)
    for (const auto& neighbor : neighbors) {
        int label = neighbor.second;
        // Simple voting (can be improved with distance weighting)
        votes[label]++;
    }
    
    // Find label with most votes
    int maxVotes = 0;
    int predictedLabel = 0;
    for (const auto& vote : votes) {
        if (vote.second > maxVotes) {
            maxVotes = vote.second;
            predictedLabel = vote.first;
        }
    }
    
    return predictedLabel;
}

void KNNClassifier::fit(const std::vector<std::vector<double>>& X, 
                       const std::vector<int>& y) {
    if (X.empty() || y.empty()) {
        throw std::runtime_error("Training data is empty");
    }
    
    if (X.size() != y.size()) {
        throw std::runtime_error("X and y must have the same size");
    }
    
    X_train = X;
    y_train = y;
    isTrained = true;
}

std::vector<int> KNNClassifier::predict(const std::vector<std::vector<double>>& X) const {
    std::vector<int> predictions;
    predictions.reserve(X.size());
    
    for (const auto& sample : X) {
        auto neighbors = findKNearest(sample);
        predictions.push_back(majorityVote(neighbors));
    }
    
    return predictions;
}

int KNNClassifier::predictSingle(const std::vector<double>& sample) const {
    auto neighbors = findKNearest(sample);
    return majorityVote(neighbors);
}

std::vector<double> KNNClassifier::predictProbability(
    const std::vector<std::vector<double>>& X) const {
    std::vector<double> probabilities;
    probabilities.reserve(X.size());
    
    for (const auto& sample : X) {
        auto neighbors = findKNearest(sample);
        
        // Calculate probability as proportion of positive neighbors
        int positiveCount = 0;
        for (const auto& neighbor : neighbors) {
            if (neighbor.second == 1) {
                positiveCount++;
            }
        }
        
        double prob = static_cast<double>(positiveCount) / neighbors.size();
        probabilities.push_back(prob);
    }
    
    return probabilities;
}

void KNNClassifier::setK(int k) {
    if (k <= 0) {
        throw std::runtime_error("K must be positive");
    }
    this->k = k;
}

int KNNClassifier::getK() const {
    return k;
}

bool KNNClassifier::getIsTrained() const {
    return isTrained;
}

