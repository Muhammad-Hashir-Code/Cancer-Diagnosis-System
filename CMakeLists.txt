cmake_minimum_required(VERSION 3.10)
project(CancerDiagnosisSystem)

set(CMAKE_CXX_STANDARD 17)

# Core sources (exclude main.cpp from here)
set(CORE_SOURCES
    src/CancerDiagnosisSystem.cpp
    src/DataPreprocessor.cpp
    src/DecisionTreeClassifier.cpp
    src/EvaluationMetrics.cpp
    src/GeneticData.cpp
    src/HashMapper.cpp
    src/KNNClassifier.cpp
    src/LogisticRegressionModel.cpp
    src/NaiveBayesClassifier.cpp
    src/Patient.cpp
)

# HTTP Server executable (includes Server.cpp which has its own main wrapper)
add_executable(cds_server 
    ${CORE_SOURCES}
    src/Server.cpp
)
target_include_directories(cds_server PRIVATE headers third_party)

# Optional: CLI executable (uses main.cpp)
# add_executable(cds_cli 
#     ${CORE_SOURCES}
#     main.cpp
# )
# target_include_directories(cds_cli PRIVATE headers third_party)

