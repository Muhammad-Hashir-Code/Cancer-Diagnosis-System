#include "../headers/DecisionTreeClassifier.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <iostream>

DecisionTreeClassifier::DecisionTreeClassifier(int maxDepth, int minSamplesSplit) 
    : maxDepth(maxDepth), minSamplesSplit(minSamplesSplit), root(nullptr) {}

double DecisionTreeClassifier::calculateGini(const std::vector<std::vector<double>>& X, 
                                             const std::vector<int>& y) const {
    if (y.empty()) return 1.0;
    
    std::map<int, int> classCounts;
    for (int label : y) {
        classCounts[label]++;
    }
    
    double gini = 1.0;
    double n = static_cast<double>(y.size());
    
    for (const auto& pair : classCounts) {
        double p = static_cast<double>(pair.second) / n;
        gini -= p * p;
    }
    
    return gini;
}

std::pair<int, double> DecisionTreeClassifier::findBestSplit(
    const std::vector<std::vector<double>>& X, 
    const std::vector<int>& y) const {
    if (X.empty() || X[0].empty()) {
        return {-1, 0.0};
    }
    
    int bestFeature = -1;
    double bestThreshold = 0.0;
    double bestGini = 1.0;
    
    int nFeatures = static_cast<int>(X[0].size());
    
    for (int feature = 0; feature < nFeatures; ++feature) {
        // Get unique values for this feature
        std::vector<double> values;
        for (const auto& sample : X) {
            values.push_back(sample[feature]);
        }
        std::sort(values.begin(), values.end());
        values.erase(std::unique(values.begin(), values.end()), values.end());
        
        // Try each threshold
        for (size_t i = 0; i < values.size() - 1; ++i) {
            double threshold = (values[i] + values[i + 1]) / 2.0;
            
            // Split data
            std::vector<std::vector<double>> XLeft, XRight;
            std::vector<int> yLeft, yRight;
            
            for (size_t j = 0; j < X.size(); ++j) {
                if (X[j][feature] <= threshold) {
                    XLeft.push_back(X[j]);
                    yLeft.push_back(y[j]);
                } else {
                    XRight.push_back(X[j]);
                    yRight.push_back(y[j]);
                }
            }
            
            if (XLeft.empty() || XRight.empty()) continue;
            
            // Calculate weighted Gini
            double giniLeft = calculateGini(XLeft, yLeft);
            double giniRight = calculateGini(XRight, yRight);
            
            double weightedGini = (static_cast<double>(yLeft.size()) / y.size()) * giniLeft +
                                  (static_cast<double>(yRight.size()) / y.size()) * giniRight;
            
            if (weightedGini < bestGini) {
                bestGini = weightedGini;
                bestFeature = feature;
                bestThreshold = threshold;
            }
        }
    }
    
    return {bestFeature, bestThreshold};
}

int DecisionTreeClassifier::getMajorityClass(const std::vector<int>& y) const {
    if (y.empty()) return 0;
    
    std::map<int, int> counts;
    for (int label : y) {
        counts[label]++;
    }
    
    int maxCount = 0;
    int majority = 0;
    for (const auto& pair : counts) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            majority = pair.first;
        }
    }
    
    return majority;
}

std::shared_ptr<TreeNode> DecisionTreeClassifier::buildTree(
    const std::vector<std::vector<double>>& X, 
    const std::vector<int>& y, 
    int depth) const {
    
    auto node = std::make_shared<TreeNode>();
    
    // Stopping conditions
    if (depth >= maxDepth || static_cast<int>(y.size()) < minSamplesSplit) {
        node->prediction = getMajorityClass(y);
        return node;
    }
    
    // Check if all labels are the same
    bool allSame = true;
    for (size_t i = 1; i < y.size(); ++i) {
        if (y[i] != y[0]) {
            allSame = false;
            break;
        }
    }
    if (allSame) {
        node->prediction = y[0];
        return node;
    }
    
    // Find best split
    auto [feature, threshold] = findBestSplit(X, y);
    
    if (feature == -1) {
        node->prediction = getMajorityClass(y);
        return node;
    }
    
    // Split data
    std::vector<std::vector<double>> XLeft, XRight;
    std::vector<int> yLeft, yRight;
    
    for (size_t i = 0; i < X.size(); ++i) {
        if (X[i][feature] <= threshold) {
            XLeft.push_back(X[i]);
            yLeft.push_back(y[i]);
        } else {
            XRight.push_back(X[i]);
            yRight.push_back(y[i]);
        }
    }
    
    if (XLeft.empty() || XRight.empty()) {
        node->prediction = getMajorityClass(y);
        return node;
    }
    
    // Build left and right subtrees
    node->featureIndex = feature;
    node->threshold = threshold;
    node->left = buildTree(XLeft, yLeft, depth + 1);
    node->right = buildTree(XRight, yRight, depth + 1);
    
    return node;
}

void DecisionTreeClassifier::fit(const std::vector<std::vector<double>>& X, 
                                 const std::vector<int>& y) {
    if (X.empty() || y.empty()) {
        throw std::runtime_error("Training data is empty");
    }
    
    if (X.size() != y.size()) {
        throw std::runtime_error("X and y must have the same size");
    }
    
    root = buildTree(X, y, 0);
}

int DecisionTreeClassifier::predictSample(const std::vector<double>& sample, 
                                          std::shared_ptr<TreeNode> node) const {
    if (!node) {
        return 0; // Default prediction
    }
    
    // If leaf node, return prediction
    if (node->featureIndex == -1) {
        return node->prediction;
    }
    
    // Traverse tree
    if (sample[node->featureIndex] <= node->threshold) {
        return predictSample(sample, node->left);
    } else {
        return predictSample(sample, node->right);
    }
}

std::vector<int> DecisionTreeClassifier::predict(
    const std::vector<std::vector<double>>& X) const {
    if (!root) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    std::vector<int> predictions;
    predictions.reserve(X.size());
    
    for (const auto& sample : X) {
        predictions.push_back(predictSample(sample, root));
    }
    
    return predictions;
}

int DecisionTreeClassifier::predictSingle(const std::vector<double>& sample) const {
    if (!root) {
        throw std::runtime_error("Model not trained. Call fit() first.");
    }
    
    return predictSample(sample, root);
}

void DecisionTreeClassifier::setMaxDepth(int depth) {
    maxDepth = depth;
}

void DecisionTreeClassifier::setMinSamplesSplit(int samples) {
    minSamplesSplit = samples;
}

void DecisionTreeClassifier::displayTree(std::shared_ptr<TreeNode> node, int depth) const {
    if (!node) return;
    
    std::string indent(depth * 2, ' ');
    
    if (node->featureIndex == -1) {
        std::cout << indent << "Leaf: Prediction = " << node->prediction << std::endl;
    } else {
        std::cout << indent << "Feature[" << node->featureIndex 
                  << "] <= " << node->threshold << std::endl;
        std::cout << indent << "  Left:" << std::endl;
        displayTree(node->left, depth + 1);
        std::cout << indent << "  Right:" << std::endl;
        displayTree(node->right, depth + 1);
    }
}

std::shared_ptr<TreeNode> DecisionTreeClassifier::getRoot() const {
    return root;
}

