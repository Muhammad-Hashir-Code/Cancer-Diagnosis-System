#ifndef LOGISTIC_REGRESSION_MODEL_H
#define LOGISTIC_REGRESSION_MODEL_H

#include <vector>

/**
 * @class LogisticRegressionModel
 * @brief Implements Logistic Regression with manual gradient descent
 */
class LogisticRegressionModel {
private:
    std::vector<double> weights;
    double bias;
    double learningRate;
    int maxIterations;
    bool isTrained;
    
    // Helper functions
    double sigmoid(double z) const;
    double rawPredictProbability(const std::vector<double>& features) const;
    double computeLoss(const std::vector<std::vector<double>>& X, 
                      const std::vector<int>& y) const;
    void gradientDescent(const std::vector<std::vector<double>>& X, 
                        const std::vector<int>& y);
    double predictProbability(const std::vector<double>& features) const;
    
public:
    LogisticRegressionModel(double learningRate = 0.01, int maxIterations = 1000);
    
    // Training and prediction
    void fit(const std::vector<std::vector<double>>& X, const std::vector<int>& y);
    std::vector<int> predict(const std::vector<std::vector<double>>& X) const;
    int predictSingle(const std::vector<double>& features) const;
    std::vector<double> predictProbabilityBatch(const std::vector<std::vector<double>>& X) const;
    double predictProbabilitySingle(const std::vector<double>& features) const;
    
    // Parameters
    void setLearningRate(double rate);
    void setMaxIterations(int iterations);
    std::vector<double> getWeights() const;
    double getBias() const;
    bool getIsTrained() const;
};

#endif // LOGISTIC_REGRESSION_MODEL_H


