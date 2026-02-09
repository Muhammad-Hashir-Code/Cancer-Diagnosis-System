#ifndef DATA_PREPROCESSOR_H
#define DATA_PREPROCESSOR_H

#include <vector>
#include <string>

/**
 * @class DataPreprocessor
 * @brief Handles data normalization and preprocessing
 */
class DataPreprocessor {
private:
    double mean;
    double stdDev;
    double minVal;
    double maxVal;
    bool isFitted;
    
public:
    DataPreprocessor();
    
    // Fit preprocessing parameters on training data
    void fit(const std::vector<double>& data);
    
    // Transform data using fitted parameters
    std::vector<double> normalize(const std::vector<double>& data) const;
    std::vector<double> minMaxScale(const std::vector<double>& data) const;
    std::vector<double> standardize(const std::vector<double>& data) const;
    
    // Fit and transform in one step
    std::vector<double> fitTransform(const std::vector<double>& data);
    
    // Utility
    bool getIsFitted() const;
    void reset();
};

#endif // DATA_PREPROCESSOR_H


