#include "../headers/CancerDiagnosisSystem.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

// NOTE: This server uses the single-header cpp-httplib library.
// Download it from: https://github.com/yhirose/cpp-httplib (place httplib.h in a folder named third_party)
#include "../third_party/httplib.h"

using namespace std;

// Forward declare serverMain (defined below after all handlers)
int serverMain();

// Helper: escape JSON string
static string json_escape(const string &s) {
    string out;
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                out.push_back(c);
        }
    }
    return out;
}

int serverMain() {
    httplib::Server svr;
    CancerDiagnosisSystem system;

    svr.Get("/status", [&](const httplib::Request& req, httplib::Response& res) {
        std::ostringstream ss;
        ss << "{\"modelsTrained\":" << (system.areModelsTrained() ? "true" : "false")
           << ",\"geneticCount\":" << system.getGeneticDataCount()
           << ",\"patientCount\":" << system.getPatientCount() << "}";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // CORS preflight for /status
    svr.Options("/status", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    // POST /load {"genesFile":"data/genes.csv","patientsFile":"data/patients.csv"}
    svr.Post("/load", [&](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        // naive JSON parsing (just extract values)
        auto extract = [&](const string &key)->string {
            size_t p = body.find(key);
            if (p == string::npos) return string();
            size_t colon = body.find(':', p);
            if (colon == string::npos) return string();
            size_t start = body.find_first_of('\"', colon);
            if (start == string::npos) return string();
            start++;
            size_t end = body.find('"', start);
            if (end == string::npos) return string();
            return body.substr(start, end - start);
        };

        string genesFile = extract("genesFile");
        string patientsFile = extract("patientsFile");

        if (genesFile.empty() || patientsFile.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Missing genesFile or patientsFile\"}", "application/json");
            return;
        }

        // Load data and train models
        system.loadData(genesFile, patientsFile);

        std::ostringstream ss;
        ss << "{\"modelsTrained\":" << (system.areModelsTrained() ? "true" : "false")
           << ",\"geneticCount\":" << system.getGeneticDataCount()
           << ",\"patientCount\":" << system.getPatientCount() << "}";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // CORS preflight for /load
    svr.Options("/load", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    // POST /patients  { "patient_id":"P011","name":"Alice","age":30, "geneticRecords":[{"geneId":"GENE_010","mutationScore":0.45,"label":0}, ...] }
    svr.Post("/patients", [&](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        auto extract = [&](const string &key)->string {
            size_t p = body.find(key);
            if (p == string::npos) return string();
            size_t colon = body.find(':', p);
            if (colon == string::npos) return string();
            size_t start = body.find_first_of('"', colon);
            if (start == string::npos) return string();
            start++;
            size_t end = body.find('"', start);
            if (end == string::npos) return string();
            return body.substr(start, end - start);
        };

        // Support a few key name variants
        string pid = extract("patient_id");
        if (pid.empty()) pid = extract("patientId");
        if (pid.empty()) pid = extract("id");
        string name = extract("name");
        string ageStr = extract("age");
        int age = 0;
        try { if (!ageStr.empty()) age = stoi(ageStr); } catch(...) { age = 0; }

        if (pid.empty() || name.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Missing patient_id or name\"}", "application/json");
            return;
        }

        // Create patient and attach any genetic records found in body
        Patient patient(pid, name, age);

        // Naive scan for gene entries: look for occurrences of "geneId" or "gene_id"
        size_t pos = 0;
        while (true) {
            size_t gpos = body.find("geneId", pos);
            if (gpos == string::npos) gpos = body.find("gene_id", pos);
            if (gpos == string::npos) break;

            // extract gene id
            size_t colon = body.find(':', gpos);
            size_t start = body.find_first_of('"', colon);
            if (start == string::npos) break; start++;
            size_t end = body.find('"', start);
            if (end == string::npos) break;
            string geneId = body.substr(start, end - start);

            // mutationScore (try several variants)
            double mutationScore = 0.0;
            size_t mpos = body.find("mutationScore", end);
            if (mpos == string::npos) mpos = body.find("mutation_score", end);
            if (mpos != string::npos) {
                size_t mcolon = body.find(':', mpos);
                if (mcolon != string::npos) {
                    // read number after colon
                    size_t numStart = body.find_first_not_of(" \t\r\n\"", mcolon+1);
                    if (numStart != string::npos) {
                        size_t numEnd = body.find_first_of(",}]\n\r", numStart);
                        string numStr = body.substr(numStart, (numEnd==string::npos? body.size()-numStart: numEnd-numStart));
                        try { mutationScore = stod(numStr); } catch(...) { mutationScore = 0.0; }
                    }
                }
            }

            int label = 0;
            size_t lpos = body.find("label", end);
            if (lpos != string::npos) {
                size_t lcolon = body.find(':', lpos);
                if (lcolon != string::npos) {
                    size_t numStart = body.find_first_not_of(" \t\r\n\"", lcolon+1);
                    if (numStart != string::npos) {
                        size_t numEnd = body.find_first_of(",}]\n\r", numStart);
                        string numStr = body.substr(numStart, (numEnd==string::npos? body.size()-numStart: numEnd-numStart));
                        try { label = stoi(numStr); } catch(...) { label = 0; }
                    }
                }
            }

            // Create GeneticData and attach to global system and patient
            GeneticData gd(geneId, mutationScore, label);
            system.addGeneticData(gd);
            patient.addGeneticData(gd);

            pos = end + 1;
        }

        // Add patient to system history (makes patient immediately available for /diagnose)
        system.addPatient(patient);
        
        // IMPORTANT: Auto-save to CSV files immediately after adding patient
        system.saveDataToFiles("data/genes.csv", "data/patients.csv");

        std::ostringstream ss;
        ss << "{\"success\":true,\"patientCount\":" << system.getPatientCount()
           << ",\"geneticCount\":" << system.getGeneticDataCount() << ",\"message\":\"Patient added and data auto-saved to CSV\"}";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // CORS preflight for /patients
    svr.Options("/patients", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    svr.Get("/patients", [&](const httplib::Request& req, httplib::Response& res) {
        std::ostringstream ss;
        ss << "[";
        bool first = true;

        // iterate patients via displayPatientHistory would print; instead replicate traversal
        // We rely on getPatientCount() and getPatientById for access; but getPatientById requires id.
        // To extract patients, we will capture patient IDs by reading patients file again would be complex.
        // Simpler: provide a light-weight approach by calling displayPatientHistory into a stringstream? Not available.
        // Instead, use the public interface: iterate patient IDs  by invoking getPatientCount and trying common ids isn't possible.
        // As a pragmatic approach, we'll expose patients through storing them when loading data. Modify system to provide a getter would be better,
        // but to keep changes minimal we will call displayPatientHistory and parse the output. This is a hack but works for now.

        // Capture output of displayPatientHistory
        std::ostringstream capture;
        // Temporarily redirect cout
        auto cout_buf = std::cout.rdbuf();
        std::cout.rdbuf(capture.rdbuf());
        system.displayPatientHistory();
        std::cout.rdbuf(cout_buf);

        string out = capture.str();
        // Parse lines looking for "Patient ID:" lines from Patient::display()
        // Patient::display prints fields; to keep parsing simple, we look for lines with "Patient ID:" etc.
        std::istringstream iss(out);
        string line;
        string id, name;
        int age = 0;
        while (std::getline(iss, line)) {
            // trim
            auto trim = [](string s){ size_t a = s.find_first_not_of(" \t\r\n"); if (a==string::npos) return string(); s = s.substr(a); size_t b = s.find_last_not_of(" \t\r\n"); if (b!=string::npos) s = s.substr(0,b+1); return s; };
            line = trim(line);
            if (line.rfind("Patient ID:", 0) == 0) {
                id = trim(line.substr(11));
            } else if (line.rfind("Name:", 0) == 0) {
                name = trim(line.substr(5));
            } else if (line.rfind("Age:", 0) == 0) {
                string agestr = trim(line.substr(4));
                try { age = stoi(agestr); } catch(...) { age = 0; }
                if (!id.empty()) {
                    if (!first) ss << ",";
                    ss << "{\"patient_id\":\"" << json_escape(id) << "\",\"name\":\"" << json_escape(name) << "\",\"age\":" << age << "}";
                    first = false;
                    id.clear(); name.clear(); age = 0;
                }
            }
        }

        ss << "]";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // CORS preflight for /patients
    svr.Options("/patients", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    svr.Get(R"(/genetic)", [&](const httplib::Request& req, httplib::Response& res) {
        std::ostringstream ss;
        ss << "[";
        bool first = true;
        
        // Similar hack: capture displayGeneticData output and parse
        std::ostringstream capture;
        auto cout_buf = std::cout.rdbuf();
        std::cout.rdbuf(capture.rdbuf());
        system.displayGeneticData();
        std::cout.rdbuf(cout_buf);

        string out = capture.str();
        // Look for lines like: Gene ID: ...
        istringstream iss(out);
        string line;
        string gid; double score=0; int label=0;
        while (getline(iss, line)) {
            auto trim = [](string s){ size_t a = s.find_first_not_of(" \t\r\n"); if (a==string::npos) return string(); s = s.substr(a); size_t b = s.find_last_not_of(" \t\r\n"); if (b!=string::npos) s = s.substr(0,b+1); return s; };
            line = trim(line);
            if (line.rfind("Gene ID:", 0) == 0) {
                gid = trim(line.substr(8));
            } else if (line.rfind("Mutation Score:", 0) == 0) {
                string s = trim(line.substr(15)); try { score = stod(s); } catch(...) { score = 0; }
            } else if (line.rfind("Label:", 0) == 0) {
                string s = trim(line.substr(6)); try { label = stoi(s); } catch(...) { label = 0; }
                if (!gid.empty()) {
                    if (!first) ss << ",";
                    ss << "{\"gene_id\":\"" << json_escape(gid) << "\",\"mutation_score\":" << score << ",\"label\":" << label << "}";
                    first = false;
                    gid.clear(); score = 0; label = 0;
                }
            }
        }

        ss << "]";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // CORS preflight for /genetic
    svr.Options("/genetic", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    // GET /diagnose?patient_id=P001&model=logistic
    svr.Get(R"(/diagnose)", [&](const httplib::Request& req, httplib::Response& res) {
        auto params = req.params;
        string pid;
        string modelStr = "logistic";
        if (req.has_param("patient_id")) pid = req.get_param_value("patient_id");
        if (req.has_param("model")) modelStr = req.get_param_value("model");

        if (pid.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Missing patient_id\"}", "application/json");
            return;
        }

        Patient patient;
        if (!system.getPatientById(pid, patient)) {
            res.status = 404;
            std::ostringstream ss; ss << "{\"error\":\"Patient ID '" << json_escape(pid) << "' not found\"}";
            res.set_content(ss.str(), "application/json");
            return;
        }

        CancerDiagnosisSystem::ModelType model = CancerDiagnosisSystem::ModelType::LOGISTIC;
        if (modelStr == "knn") model = CancerDiagnosisSystem::ModelType::KNN;
        else if (modelStr == "decision_tree") model = CancerDiagnosisSystem::ModelType::DECISION_TREE;
        else if (modelStr == "naive_bayes") model = CancerDiagnosisSystem::ModelType::NAIVE_BAYES;

        double risk = system.diagnosePatient(patient, model);
        int pred = system.predictPatient(patient, model);

        std::ostringstream ss;
        ss << "{\"patient_id\":\"" << json_escape(patient.getPatientId()) << "\",\"riskScore\":" << risk << ",\"prediction\":" << pred << "}";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // CORS preflight for /diagnose
    svr.Options("/diagnose", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    // GET /queue -> { queueSize: N, patients: ["P1","P2"] }
    svr.Get("/queue", [&](const httplib::Request& req, httplib::Response& res) {
        std::ostringstream ss;
        auto ids = system.getQueuedPatientIds();
        ss << "{";
        ss << "\"queueSize\":" << system.getQueueSize() << ",";
        ss << "\"patients\":[";
        for (size_t i = 0; i < ids.size(); ++i) {
            if (i) ss << ",";
            ss << "\"" << json_escape(ids[i]) << "\"";
        }
        ss << "]}";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    svr.Options("/queue", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    // POST /queue { "patient_id":"P001" } -> schedule patient for testing
    svr.Post("/queue", [&](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        auto extract = [&](const string &key)->string {
            size_t p = body.find(key);
            if (p == string::npos) return string();
            size_t colon = body.find(':', p);
            if (colon == string::npos) return string();
            size_t start = body.find_first_of('"', colon);
            if (start == string::npos) return string();
            start++;
            size_t end = body.find('"', start);
            if (end == string::npos) return string();
            return body.substr(start, end - start);
        };

        string pid = extract("patient_id");
        if (pid.empty()) pid = extract("patientId");
        if (pid.empty()) {
            res.status = 400;
            res.set_content("{\"error\":\"Missing patient_id\"}", "application/json");
            return;
        }

        Patient patient;
        if (!system.getPatientById(pid, patient)) {
            res.status = 404;
            std::ostringstream ss; ss << "{\"error\":\"Patient ID '" << json_escape(pid) << "' not found\"}";
            res.set_content(ss.str(), "application/json");
            return;
        }

        system.scheduleTest(patient);

        std::ostringstream ss;
        ss << "{\"success\":true,\"queueSize\":" << system.getQueueSize() << ",\"message\":\"Patient scheduled for diagnosis\"}";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // POST /queue/process -> processes all queued patients with optional model selection
    // Body: { "model": "logistic" } or empty for default (logistic)
    svr.Post("/queue/process", [&](const httplib::Request& req, httplib::Response& res) {
        string body = req.body;
        
        // Extract model from request body
        string modelStr = "logistic"; // default
        size_t modelPos = body.find("\"model\"");
        if (modelPos != string::npos) {
            size_t colon = body.find(':', modelPos);
            if (colon != string::npos) {
                size_t start = body.find_first_of('"', colon);
                if (start != string::npos) {
                    start++;
                    size_t end = body.find('"', start);
                    if (end != string::npos) {
                        modelStr = body.substr(start, end - start);
                    }
                }
            }
        }
        
        // Map string to ModelType
        CancerDiagnosisSystem::ModelType model = CancerDiagnosisSystem::ModelType::LOGISTIC;
        if (modelStr == "knn") model = CancerDiagnosisSystem::ModelType::KNN;
        else if (modelStr == "decision_tree") model = CancerDiagnosisSystem::ModelType::DECISION_TREE;
        else if (modelStr == "naive_bayes") model = CancerDiagnosisSystem::ModelType::NAIVE_BAYES;

        // Process queue with selected model and get diagnosis results
        auto results = system.processTestQueueWithModel(model);

        // Persist changes to CSV files
        system.saveDataToFiles("data/genes.csv", "data/patients.csv");

        // Build JSON array of results
        std::ostringstream ss;
        ss << "{\"model\":\"" << modelStr << "\",\"processed\":" << results.size() 
           << ",\"results\":[";
        for (size_t i = 0; i < results.size(); ++i) {
            if (i) ss << ",";
            ss << results[i];
        }
        ss << "],\"patientCount\":" << system.getPatientCount()
           << ",\"geneticCount\":" << system.getGeneticDataCount() << "}";
        res.set_content(ss.str(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    svr.Options("/queue/process", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_content("", "text/plain");
    });

    std::cout << "Starting server on http://localhost:8080 ..." << std::endl;
        std::cout.flush();
    
        if (!svr.listen("localhost", 8080)) {
            std::cerr << "Failed to bind to port 8080. Is another server already running?" << std::endl;
            return 1;
        }

    return 0;
}

// Wrapper entry point for the server executable
int main() {
    return serverMain();
}
