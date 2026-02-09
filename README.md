# 🔬 Cancer Diagnosis System

> **An AI-powered web application for cancer diagnosis using multiple machine learning algorithms, built from scratch in C++ with a modern web interface.**

[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Tech Stack](#tech-stack)
- [Architecture](#architecture)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [API Documentation](#api-documentation)
- [Machine Learning Models](#machine-learning-models)
- [Screenshots](#screenshots)
- [Contributing](#contributing)
- [License](#license)
- [Author](#author)

## 🎯 Overview

The **Cancer Diagnosis System** is a comprehensive, full-stack application that leverages multiple machine learning algorithms to assist in cancer diagnosis based on genetic mutation data. This project demonstrates the implementation of core ML algorithms from scratch in C++, combined with a modern web-based user interface for real-time diagnosis and analysis.

### Key Highlights

- 🧠 **Four ML Models**: Decision Tree, K-Nearest Neighbors, Logistic Regression, and Naive Bayes
- ⚡ **High Performance**: C++ backend ensures fast processing and low latency
- 🌐 **Modern Web UI**: Intuitive interface with real-time data visualization
- 📊 **Model Comparison**: Evaluate and compare multiple ML models side-by-side
- 🔄 **Batch Processing**: Queue system for processing multiple patients efficiently
- 📈 **Comprehensive Metrics**: Accuracy, Precision, Recall, and F1-Score evaluation

## ✨ Features

### Core Functionality

- **Multi-Model Diagnosis**: Run predictions using four different ML algorithms
- **Real-time Analysis**: Instant diagnosis results with confidence scores
- **Data Management**: Load and manage patient and genetic data via CSV files
- **Batch Queue System**: Process multiple patients in a queue for efficient workflow
- **Model Evaluation**: Comprehensive performance metrics for all models
- **Data Visualization**: Interactive charts and graphs using Chart.js
- **Responsive Design**: Modern, user-friendly interface with dark mode support

### Advanced Features

- **Hash-based Data Mapping**: Efficient genetic data lookup using custom hash tables
- **Data Preprocessing**: Automated data cleaning and normalization
- **RESTful API**: Clean API design for easy integration
- **CORS Support**: Cross-origin resource sharing enabled for web access
- **Error Handling**: Robust error handling and user feedback
- **Settings Management**: Configurable API endpoints and preferences

## 🛠️ Tech Stack

### Backend
- **Language**: C++17
- **HTTP Server**: [cpp-httplib](https://github.com/yhirose/cpp-httplib) v0.28.0
- **Build System**: CMake 3.10+
- **Architecture**: RESTful API

### Frontend
- **HTML5**: Semantic markup
- **CSS3**: Modern styling with animations
- **JavaScript**: Vanilla JS (no frameworks)
- **Chart.js**: Data visualization library

### Machine Learning
- **Algorithms**: Implemented from scratch
  - Decision Tree Classifier
  - K-Nearest Neighbors (KNN)
  - Logistic Regression
  - Naive Bayes Classifier

### Data Structures
- **Hash Tables**: Custom implementation for genetic data mapping
- **Vectors & Arrays**: Efficient data storage and manipulation
- **Queues**: Patient processing queue system

## 🏗️ Architecture

```
┌─────────────────┐
│   Web Browser   │
│   (Frontend)    │
└────────┬────────┘
         │ HTTP/REST API
         │ (JSON)
┌────────▼────────┐
│  C++ HTTP Server │
│   (Port 8080)    │
└────────┬────────┘
         │
    ┌────┴────┐
    │         │
┌───▼───┐ ┌──▼────────┐
│  ML   │ │  Data     │
│ Models│ │ Structures│
└───────┘ └───────────┘
```

### System Flow

1. **Data Loading**: CSV files are loaded and preprocessed
2. **Model Training**: ML models are trained on genetic mutation data
3. **Diagnosis**: Patient genetic data is analyzed by selected model(s)
4. **Results**: Predictions are returned with confidence scores
5. **Visualization**: Results are displayed in the web interface

## 🚀 Getting Started

### Prerequisites

- **CMake** (3.10 or higher)
- **C++ Compiler**:
  - Windows: Visual Studio 2019+
  - Linux: GCC
  - macOS: Xcode Command Line Tools
- **Python 3** (optional, for serving UI)

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/Muhammad-Hashir-Code/Cancer-Diagnosis-System.git
   cd Cancer-Diagnosis-System/CancerDiagnosisSystem
   ```

2. **Build the project**
   ```bash
   # Create build directory
   mkdir build && cd build
   
   # Configure (Windows)
   cmake .. -G "Visual Studio 16 2019"
   
   # Configure (Linux/macOS)
   cmake .. -DCMAKE_BUILD_TYPE=Release
   
   # Build
   cmake --build . --config Release
   ```

3. **Run the server**
   ```bash
   # From project root directory
   ./build/Release/cds_server.exe  # Windows
   ./build/cds_server               # Linux/macOS
   ```

4. **Open the web interface**
   ```bash
   cd ui
   python -m http.server 8000
   # Open http://localhost:8000/index.html in your browser
   ```

For detailed setup instructions, see [SETUP_GUIDE.md](SETUP_GUIDE.md).

## 📖 Usage

### Loading Data

1. Navigate to the **"Load Data"** section
2. Click **"Load Data on Server"**
3. Wait for confirmation message
4. Verify data in the Dashboard

### Running Diagnosis

1. Go to **"Run Diagnosis"** section
2. Add patient genetic records (Gene ID and Mutation Score)
3. Select one or more ML models
4. Click **"Run Diagnosis"**
5. View results with confidence scores

### Evaluating Models

1. Navigate to **"Evaluate Models"** section
2. Click **"Evaluate All Models"**
3. Compare performance metrics:
   - Accuracy
   - Precision
   - Recall
   - F1-Score

### Queue System

1. Add patients to the queue
2. Process queue to run batch diagnosis
3. View results for all queued patients

## 📁 Project Structure

```
CancerDiagnosisSystem/
├── data/                      # CSV data files
│   ├── genes.csv             # Genetic mutation data
│   ├── patients.csv          # Patient information
│   └── test_*.csv            # Test datasets
├── headers/                   # C++ header files
│   ├── CancerDiagnosisSystem.h
│   ├── DecisionTreeClassifier.h
│   ├── KNNClassifier.h
│   ├── LogisticRegressionModel.h
│   ├── NaiveBayesClassifier.h
│   └── ...
├── src/                       # C++ source files
│   ├── Server.cpp            # HTTP server implementation
│   ├── CancerDiagnosisSystem.cpp
│   └── [ML Model implementations]
├── ui/                        # Web interface
│   ├── index.html            # Main HTML file
│   ├── script.js             # Frontend logic
│   └── styles.css            # Styling
├── third_party/               # External libraries
│   └── httplib.h             # HTTP server library
├── CMakeLists.txt            # Build configuration
├── SETUP_GUIDE.md            # Detailed setup guide
└── README.md                 # This file
```

## 🔌 API Documentation

### Endpoints

#### `GET /status`
Get system status and data counts.

**Response:**
```json
{
  "modelsTrained": true,
  "geneticCount": 150,
  "patientCount": 50
}
```

#### `POST /load`
Load data from CSV files.

**Request:**
```json
{
  "genesFile": "data/genes.csv",
  "patientsFile": "data/patients.csv"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Data loaded successfully",
  "geneticCount": 150,
  "patientCount": 50
}
```

#### `POST /train`
Train all ML models.

**Response:**
```json
{
  "success": true,
  "message": "Models trained successfully"
}
```

#### `POST /diagnose`
Run diagnosis on patient data.

**Request:**
```json
{
  "model": "decision_tree",
  "geneticRecords": [
    {"geneId": "GENE_001", "mutationScore": 0.85},
    {"geneId": "GENE_002", "mutationScore": 0.72}
  ]
}
```

**Response:**
```json
{
  "success": true,
  "model": "decision_tree",
  "prediction": 1,
  "confidence": 0.87,
  "message": "High risk detected"
}
```

#### `POST /evaluate`
Evaluate all models and return metrics.

**Response:**
```json
{
  "decision_tree": {
    "accuracy": 0.92,
    "precision": 0.89,
    "recall": 0.91,
    "f1Score": 0.90
  },
  "knn": { ... },
  "logistic_regression": { ... },
  "naive_bayes": { ... }
}
```

## 🤖 Machine Learning Models

### Decision Tree Classifier
- **Type**: Supervised Learning
- **Use Case**: Classification based on feature thresholds
- **Advantages**: Interpretable, handles non-linear relationships
- **Implementation**: Custom C++ implementation with recursive splitting

### K-Nearest Neighbors (KNN)
- **Type**: Instance-based Learning
- **Use Case**: Classification based on similarity
- **Advantages**: Simple, effective for non-linear data
- **Implementation**: Distance-based classification with configurable k

### Logistic Regression
- **Type**: Statistical Learning
- **Use Case**: Binary classification with probability estimates
- **Advantages**: Fast, interpretable coefficients
- **Implementation**: Gradient descent optimization

### Naive Bayes
- **Type**: Probabilistic Classifier
- **Use Case**: Classification based on Bayes' theorem
- **Advantages**: Fast, works well with small datasets
- **Implementation**: Gaussian Naive Bayes for continuous features

## 📊 Data Format

### genes.csv
```csv
Gene_ID,Mutation_Score,Label
GENE_001,0.85,1
GENE_002,0.72,1
GENE_003,0.45,0
```

### patients.csv
```csv
Patient_ID,Name,Age
P001,John Doe,45
P002,Jane Smith,52
```

## 🎨 Screenshots

> *Note: Add screenshots of your application here*

- Dashboard showing system statistics
- Diagnosis interface with model selection
- Results visualization with charts
- Model evaluation comparison

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

### Contribution Guidelines

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Muhammad Hashir**

- GitHub: [@Muhammad-Hashir-Code](https://github.com/Muhammad-Hashir-Code)
- Email: siddiquimuhammadhashir1@gmail.com

## 🙏 Acknowledgments

- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - HTTP server library
- [Chart.js](https://www.chartjs.org/) - Data visualization library
- CMake community for excellent build system documentation

## 📚 Additional Resources

- [Setup Guide](SETUP_GUIDE.md) - Detailed installation instructions
- [Data Loading Troubleshooting](DATA_LOADING_TROUBLESHOOTING.md) - Common issues and solutions
- [CMake Documentation](https://cmake.org/documentation/)

## ⭐ Show Your Support

If you find this project helpful, please consider giving it a ⭐ on GitHub!

---

**Built with ❤️ using C++ and modern web technologies**
