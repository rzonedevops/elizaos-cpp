#include "elizaos/hats.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace elizaos {

// JsonDataSource implementation
JsonDataSource::JsonDataSource(const DataSourceConfig& config) 
    : DataSource(config) {
}

HatsStatus JsonDataSource::connect() {
    // For file-based JSON, just check if file exists
    if (config_.type == DataSourceType::JSON) {
        std::ifstream file(config_.location);
        connected_ = file.good();
        return connected_ ? HatsStatus::SUCCESS : HatsStatus::ERROR_INVALID_SOURCE;
    }
    return HatsStatus::ERROR_INVALID_FORMAT;
}

HatsStatus JsonDataSource::disconnect() {
    connected_ = false;
    return HatsStatus::SUCCESS;
}

HatsStatus JsonDataSource::loadData(DataSet& data) {
    if (!connected_) {
        return HatsStatus::ERROR_INVALID_SOURCE;
    }
    
    std::ifstream file(config_.location);
    if (!file.is_open()) {
        return HatsStatus::ERROR_ACCESS_DENIED;
    }
    
    // Simple JSON parsing (basic implementation)
    std::string line;
    data.clear();
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        
        // Basic JSON object parsing (simplified)
        if (line.find('{') != std::string::npos && line.find('}') != std::string::npos) {
            DataRecord record;
            
            // Extract key-value pairs using a simpler approach (avoid regex issues)
            std::size_t pos = 0;
            while (pos < line.size()) {
                // Find key start
                std::size_t keyStart = line.find('"', pos);
                if (keyStart == std::string::npos) break;
                keyStart++; // Skip opening quote
                
                // Find key end
                std::size_t keyEnd = line.find('"', keyStart);
                if (keyEnd == std::string::npos) break;
                
                std::string key = line.substr(keyStart, keyEnd - keyStart);
                
                // Find colon
                std::size_t colonPos = line.find(':', keyEnd);
                if (colonPos == std::string::npos) break;
                
                // Find value start
                std::size_t valueStart = colonPos + 1;
                while (valueStart < line.size() && (line[valueStart] == ' ' || line[valueStart] == '\t')) {
                    valueStart++;
                }
                
                // Find value end
                std::size_t valueEnd;
                std::string value;
                if (valueStart < line.size() && line[valueStart] == '"') {
                    // Quoted string
                    valueStart++; // Skip opening quote
                    valueEnd = line.find('"', valueStart);
                    if (valueEnd == std::string::npos) break;
                    value = line.substr(valueStart, valueEnd - valueStart);
                    pos = valueEnd + 1;
                } else {
                    // Unquoted value
                    valueEnd = line.find_first_of(",}", valueStart);
                    if (valueEnd == std::string::npos) valueEnd = line.size();
                    value = line.substr(valueStart, valueEnd - valueStart);
                    // Trim trailing spaces
                    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
                        value.pop_back();
                    }
                    pos = valueEnd;
                }
                
                record[key] = hats_utils::parseDataValue(value);
            }
            
            if (!record.empty()) {
                data.push_back(record);
            }
        }
    }
    
    return HatsStatus::SUCCESS;
}

bool JsonDataSource::isConnected() const {
    return connected_;
}

// CsvDataSource implementation
CsvDataSource::CsvDataSource(const DataSourceConfig& config) 
    : DataSource(config) {
    // Check for delimiter parameter
    auto delimParam = config_.parameters.find("delimiter");
    if (delimParam != config_.parameters.end() && !delimParam->second.empty()) {
        delimiter_ = delimParam->second[0];
    }
    
    // Check for header parameter
    auto headerParam = config_.parameters.find("hasHeader");
    if (headerParam != config_.parameters.end()) {
        hasHeader_ = (headerParam->second == "true" || headerParam->second == "1");
    }
}

HatsStatus CsvDataSource::connect() {
    std::ifstream file(config_.location);
    connected_ = file.good();
    return connected_ ? HatsStatus::SUCCESS : HatsStatus::ERROR_INVALID_SOURCE;
}

HatsStatus CsvDataSource::disconnect() {
    connected_ = false;
    return HatsStatus::SUCCESS;
}

HatsStatus CsvDataSource::loadData(DataSet& data) {
    if (!connected_) {
        return HatsStatus::ERROR_INVALID_SOURCE;
    }
    
    std::ifstream file(config_.location);
    if (!file.is_open()) {
        return HatsStatus::ERROR_ACCESS_DENIED;
    }
    
    std::string line;
    std::vector<std::string> headers;
    data.clear();
    
    // Read header if present
    if (hasHeader_ && std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        
        while (std::getline(ss, cell, delimiter_)) {
            // Trim whitespace
            cell.erase(0, cell.find_first_not_of(" \t\r\n"));
            cell.erase(cell.find_last_not_of(" \t\r\n") + 1);
            headers.push_back(cell);
        }
    }
    
    // Read data rows
    int rowIndex = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::stringstream ss(line);
        std::string cell;
        DataRecord record;
        int colIndex = 0;
        
        while (std::getline(ss, cell, delimiter_)) {
            // Trim whitespace
            cell.erase(0, cell.find_first_not_of(" \t\r\n"));
            cell.erase(cell.find_last_not_of(" \t\r\n") + 1);
            
            std::string key;
            if (hasHeader_ && colIndex < static_cast<int>(headers.size())) {
                key = headers[colIndex];
            } else {
                key = "col_" + std::to_string(colIndex);
            }
            
            record[key] = hats_utils::parseDataValue(cell);
            colIndex++;
        }
        
        if (!record.empty()) {
            data.push_back(record);
        }
        rowIndex++;
    }
    
    return HatsStatus::SUCCESS;
}

bool CsvDataSource::isConnected() const {
    return connected_;
}

// DataProcessor implementation
void DataProcessor::addStep(const ProcessingStep& step) {
    steps_.push_back(step);
}

HatsStatus DataProcessor::process(const DataSet& input, DataSet& output) {
    if (steps_.empty()) {
        output = input;
        return HatsStatus::SUCCESS;
    }
    
    DataSet currentData = input;
    DataSet tempOutput;
    
    for (const auto& step : steps_) {
        tempOutput.clear();
        
        switch (step.operation) {
            case ProcessingOperation::FILTER:
                if (!step.condition) {
                    return HatsStatus::ERROR_PROCESSING_FAILED;
                }
                if (applyFilter(currentData, tempOutput, step.condition) != HatsStatus::SUCCESS) {
                    return HatsStatus::ERROR_PROCESSING_FAILED;
                }
                break;
                
            case ProcessingOperation::TRANSFORM:
                if (applyTransform(currentData, tempOutput, step.parameters) != HatsStatus::SUCCESS) {
                    return HatsStatus::ERROR_PROCESSING_FAILED;
                }
                break;
                
            case ProcessingOperation::SORT:
                tempOutput = currentData;
                // Basic sort implementation - sort by first column if no key specified
                std::sort(tempOutput.begin(), tempOutput.end(), 
                         [](const DataRecord& a, const DataRecord& b) {
                             if (!a.empty() && !b.empty()) {
                                 return hats_utils::dataValueToString(a.begin()->second) < 
                                        hats_utils::dataValueToString(b.begin()->second);
                             }
                             return false;
                         });
                break;
                
            default:
                // For other operations, just pass through for now
                tempOutput = currentData;
                break;
        }
        
        currentData = tempOutput;
    }
    
    output = currentData;
    return HatsStatus::SUCCESS;
}

void DataProcessor::clearSteps() {
    steps_.clear();
}

size_t DataProcessor::getStepCount() const {
    return steps_.size();
}

HatsStatus DataProcessor::applyFilter(const DataSet& input, DataSet& output, 
                                     const std::function<bool(const DataRecord&)>& filter) {
    output.clear();
    for (const auto& record : input) {
        if (filter(record)) {
            output.push_back(record);
        }
    }
    return HatsStatus::SUCCESS;
}

HatsStatus DataProcessor::applyTransform(const DataSet& input, DataSet& output,
                                        const std::unordered_map<std::string, std::any>& params) {
    (void)params; // Mark as unused to suppress warning
    // Basic transform implementation - just copy for now
    output = input;
    return HatsStatus::SUCCESS;
}

// HatsManager implementation
HatsStatus HatsManager::registerDataSource(std::unique_ptr<DataSource> source) {
    if (!source) {
        return HatsStatus::ERROR_INVALID_SOURCE;
    }
    
    std::string id = source->getId();
    if (dataSources_.find(id) != dataSources_.end()) {
        return HatsStatus::ERROR_INVALID_SOURCE; // Already exists
    }
    
    dataSources_[id] = std::move(source);
    return HatsStatus::SUCCESS;
}

HatsStatus HatsManager::unregisterDataSource(const std::string& sourceId) {
    auto it = dataSources_.find(sourceId);
    if (it == dataSources_.end()) {
        return HatsStatus::ERROR_NOT_FOUND;
    }
    
    // Disconnect before removing
    it->second->disconnect();
    dataSources_.erase(it);
    return HatsStatus::SUCCESS;
}

DataSource* HatsManager::getDataSource(const std::string& sourceId) {
    auto it = dataSources_.find(sourceId);
    return (it != dataSources_.end()) ? it->second.get() : nullptr;
}

std::vector<std::string> HatsManager::getDataSourceIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : dataSources_) {
        ids.push_back(pair.first);
    }
    return ids;
}

HatsStatus HatsManager::loadFromSource(const std::string& sourceId, DataSet& data) {
    DataSource* source = getDataSource(sourceId);
    if (!source) {
        return HatsStatus::ERROR_NOT_FOUND;
    }
    
    if (!source->isConnected()) {
        HatsStatus status = source->connect();
        if (status != HatsStatus::SUCCESS) {
            return status;
        }
    }
    
    return source->loadData(data);
}

HatsStatus HatsManager::loadFromMultipleSources(const std::vector<std::string>& sourceIds, DataSet& data) {
    std::vector<DataSet> datasets;
    
    for (const auto& sourceId : sourceIds) {
        DataSet sourceData;
        HatsStatus status = loadFromSource(sourceId, sourceData);
        if (status != HatsStatus::SUCCESS) {
            return status;
        }
        datasets.push_back(sourceData);
    }
    
    return mergeDataSets(datasets, data);
}

HatsStatus HatsManager::processData(const std::string& sourceId, 
                                   const std::vector<ProcessingStep>& steps,
                                   DataSet& output) {
    DataSet sourceData;
    HatsStatus status = loadFromSource(sourceId, sourceData);
    if (status != HatsStatus::SUCCESS) {
        return status;
    }
    
    // Create temporary processor
    DataProcessor tempProcessor;
    for (const auto& step : steps) {
        tempProcessor.addStep(step);
    }
    
    return tempProcessor.process(sourceData, output);
}

HatsStatus HatsManager::mergeDataSets(const std::vector<DataSet>& inputs, DataSet& merged) {
    merged.clear();
    
    for (const auto& dataset : inputs) {
        for (const auto& record : dataset) {
            merged.push_back(record);
        }
    }
    
    return HatsStatus::SUCCESS;
}

size_t HatsManager::getRegisteredSourceCount() const {
    return dataSources_.size();
}

bool HatsManager::isSourceRegistered(const std::string& sourceId) const {
    return dataSources_.find(sourceId) != dataSources_.end();
}

// Utility functions implementation
namespace hats_utils {

std::string dataValueToString(const DataValue& value) {
    return std::visit([](const auto& v) -> std::string {
        if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) {
            return v;
        } else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, bool>) {
            return v ? "true" : "false";
        } else {
            return std::to_string(v);
        }
    }, value);
}

DataValue parseDataValue(const std::string& str) {
    // Try to parse as different types
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
    
    if (trimmed.empty()) {
        return std::string("");
    }
    
    // Check for boolean
    if (trimmed == "true" || trimmed == "TRUE") {
        return true;
    }
    if (trimmed == "false" || trimmed == "FALSE") {
        return false;
    }
    
    // Try to parse as integer
    try {
        size_t pos;
        int intVal = std::stoi(trimmed, &pos);
        if (pos == trimmed.length()) {
            return intVal;
        }
    } catch (...) {
        // Not an integer
    }
    
    // Try to parse as double
    try {
        size_t pos;
        double doubleVal = std::stod(trimmed, &pos);
        if (pos == trimmed.length()) {
            return doubleVal;
        }
    } catch (...) {
        // Not a double
    }
    
    // Default to string
    return trimmed;
}

std::unique_ptr<DataSource> createDataSource(const DataSourceConfig& config) {
    switch (config.type) {
        case DataSourceType::JSON:
            return std::make_unique<JsonDataSource>(config);
        case DataSourceType::CSV:
            return std::make_unique<CsvDataSource>(config);
        default:
            return nullptr;
    }
}

std::string statusToString(HatsStatus status) {
    switch (status) {
        case HatsStatus::SUCCESS:
            return "SUCCESS";
        case HatsStatus::ERROR_INVALID_SOURCE:
            return "ERROR_INVALID_SOURCE";
        case HatsStatus::ERROR_INVALID_FORMAT:
            return "ERROR_INVALID_FORMAT";
        case HatsStatus::ERROR_PROCESSING_FAILED:
            return "ERROR_PROCESSING_FAILED";
        case HatsStatus::ERROR_NOT_FOUND:
            return "ERROR_NOT_FOUND";
        case HatsStatus::ERROR_ACCESS_DENIED:
            return "ERROR_ACCESS_DENIED";
        default:
            return "UNKNOWN_ERROR";
    }
}

} // namespace hats_utils

} // namespace elizaos