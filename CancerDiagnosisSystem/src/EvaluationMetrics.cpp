#include "../headers/EvaluationMetrics.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdexcept>

EvaluationMetrics::EvaluationMetrics() {}

ConfusionMatrix EvaluationMetrics::calculateConfusionMatrix(
    const std::vector<int>& yTrue, 
    const std::vector<int>& yPred) const {
    
    if (yTrue.size() != yPred.size()) {
        throw std::runtime_error("yTrue and yPred must have the same size");
    }
    
    ConfusionMatrix cm;
    
    for (size_t i = 0; i < yTrue.size(); ++i) {
        int trueLabel = yTrue[i];
        int predLabel = yPred[i];
        
        if (trueLabel == 1 && predLabel == 1) {
            cm.truePositive++;
        } else if (trueLabel == 0 && predLabel == 0) {
            cm.trueNegative++;
        } else if (trueLabel == 0 && predLabel == 1) {
            cm.falsePositive++;
        } else if (trueLabel == 1 && predLabel == 0) {
            cm.falseNegative++;
        }
    }
    
    return cm;
}

double EvaluationMetrics::accuracy(const std::vector<int>& yTrue, 
                                   const std::vector<int>& yPred) const {
    if (yTrue.empty()) return 0.0;
    
    ConfusionMatrix cm = calculateConfusionMatrix(yTrue, yPred);
    int total = cm.truePositive + cm.trueNegative + cm.falsePositive + cm.falseNegative;
    
    if (total == 0) return 0.0;
    
    return static_cast<double>(cm.truePositive + cm.trueNegative) / total;
}

double EvaluationMetrics::precision(const std::vector<int>& yTrue, 
                                    const std::vector<int>& yPred) const {
    ConfusionMatrix cm = calculateConfusionMatrix(yTrue, yPred);
    int denominator = cm.truePositive + cm.falsePositive;
    
    if (denominator == 0) return 0.0;
    
    return static_cast<double>(cm.truePositive) / denominator;
}

double EvaluationMetrics::recall(const std::vector<int>& yTrue, 
                                 const std::vector<int>& yPred) const {
    ConfusionMatrix cm = calculateConfusionMatrix(yTrue, yPred);
    int denominator = cm.truePositive + cm.falseNegative;
    
    if (denominator == 0) return 0.0;
    
    return static_cast<double>(cm.truePositive) / denominator;
}

double EvaluationMetrics::f1Score(const std::vector<int>& yTrue, 
                                  const std::vector<int>& yPred) const {
    double prec = precision(yTrue, yPred);
    double rec = recall(yTrue, yPred);
    
    if (prec + rec == 0.0) return 0.0;
    
    return 2.0 * (prec * rec) / (prec + rec);
}

ConfusionMatrix EvaluationMetrics::getConfusionMatrix(const std::vector<int>& yTrue, 
                                                      const std::vector<int>& yPred) const {
    return calculateConfusionMatrix(yTrue, yPred);
}

void EvaluationMetrics::displayConfusionMatrix(const ConfusionMatrix& cm) const {
    std::cout << "\n=== Confusion Matrix ===" << std::endl;
    std::cout << std::setw(20) << "Predicted" << std::endl;
    std::cout << std::setw(10) << " " 
              << std::setw(10) << "Negative" 
              << std::setw(10) << "Positive" << std::endl;
    std::cout << "Actual" << std::setw(5) << "Negative" 
              << std::setw(10) << cm.trueNegative 
              << std::setw(10) << cm.falsePositive << std::endl;
    std::cout << std::setw(10) << "Positive" 
              << std::setw(10) << cm.falseNegative 
              << std::setw(10) << cm.truePositive << std::endl;
    std::cout << "========================" << std::endl;
}

void EvaluationMetrics::displayMetrics(const std::vector<int>& yTrue, 
                                       const std::vector<int>& yPred) const {
    double acc = accuracy(yTrue, yPred);
    double prec = precision(yTrue, yPred);
    double rec = recall(yTrue, yPred);
    double f1 = f1Score(yTrue, yPred);
    ConfusionMatrix cm = getConfusionMatrix(yTrue, yPred);
    
    std::cout << "\n=== Evaluation Metrics ===" << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Accuracy:  " << acc << " (" << acc * 100 << "%)" << std::endl;
    std::cout << "Precision: " << prec << " (" << prec * 100 << "%)" << std::endl;
    std::cout << "Recall:    " << rec << " (" << rec * 100 << "%)" << std::endl;
    std::cout << "F1-Score:  " << f1 << std::endl;
    
    displayConfusionMatrix(cm);
    std::cout << std::endl;
}

