#include "../headers/DataPreprocessor.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>

DataPreprocessor::DataPreprocessor() 
    : mean(0.0), stdDev(0.0), minVal(0.0), maxVal(0.0), isFitted(false) {}

void DataPreprocessor::fit(const std::vector<double>& data) {
    if (data.empty()) {
        throw std::runtime_error("Cannot fit preprocessor on empty data");
    }
    
    // Calculate mean
    mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    
    // Calculate standard deviation
    double variance = 0.0;
    for (double val : data) {
        variance += (val - mean) * (val - mean);
    }
    stdDev = std::sqrt(variance / data.size());
    
    // Find min and max
    minVal = *std::min_element(data.begin(), data.end());
    maxVal = *std::max_element(data.begin(), data.end());
    
    isFitted = true;
}

std::vector<double> DataPreprocessor::normalize(const std::vector<double>& data) const {
    if (!isFitted) {
        throw std::runtime_error("Preprocessor not fitted. Call fit() first.");
    }
    
    return standardize(data);
}

std::vector<double> DataPreprocessor::minMaxScale(const std::vector<double>& data) const {
    if (!isFitted) {
        throw std::runtime_error("Preprocessor not fitted. Call fit() first.");
    }
    
    if (maxVal == minVal) {
        return std::vector<double>(data.size(), 0.5);
    }
    
    std::vector<double> scaled;
    scaled.reserve(data.size());
    
    for (double val : data) {
        double scaledVal = (val - minVal) / (maxVal - minVal);
        scaled.push_back(scaledVal);
    }
    
    return scaled;
}

std::vector<double> DataPreprocessor::standardize(const std::vector<double>& data) const {
    if (!isFitted) {
        throw std::runtime_error("Preprocessor not fitted. Call fit() first.");
    }
    
    if (stdDev == 0.0) {
        return std::vector<double>(data.size(), 0.0);
    }
    
    std::vector<double> standardized;
    standardized.reserve(data.size());
    
    for (double val : data) {
        double stdVal = (val - mean) / stdDev;
        standardized.push_back(stdVal);
    }
    
    return standardized;
}

std::vector<double> DataPreprocessor::fitTransform(const std::vector<double>& data) {
    fit(data);
    return standardize(data);
}

bool DataPreprocessor::getIsFitted() const {
    return isFitted;
}

void DataPreprocessor::reset() {
    mean = 0.0;
    stdDev = 0.0;
    minVal = 0.0;
    maxVal = 0.0;
    isFitted = false;
}

