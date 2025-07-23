#include "elizaos/embodiment.hpp"
#include "elizaos/agentlogger.hpp"
#include <fstream>
#include <sstream>

namespace elizaos {

/**
 * File Sensory Interface Implementation
 */
FileSensoryInterface::FileSensoryInterface(SensoryDataType type, const std::string& filePath)
    : type_(type), filePath_(filePath) {}

bool FileSensoryInterface::initialize() {
    AgentLogger logger;
    logger.logInfo("Initializing File Sensory Interface: " + filePath_);
    
    if (active_) {
        return true; // Already initialized
    }
    
    fileStream_.open(filePath_, std::ios::binary);
    if (!fileStream_.is_open()) {
        logger.logError("Failed to open file: " + filePath_);
        return false;
    }
    
    active_ = true;
    logger.logSuccess("File Sensory Interface initialized: " + filePath_);
    return true;
}

void FileSensoryInterface::shutdown() {
    if (!active_) {
        return;
    }
    
    AgentLogger logger;
    logger.logInfo("Shutting down File Sensory Interface: " + filePath_);
    
    active_ = false;
    
    if (fileStream_.is_open()) {
        fileStream_.close();
    }
    
    logger.logInfo("File Sensory Interface shutdown complete");
}

std::shared_ptr<SensoryData> FileSensoryInterface::readData() {
    if (!active_ || !fileStream_.is_open()) {
        return nullptr;
    }
    
    std::shared_ptr<SensoryData> data;
    
    switch (type_) {
        case SensoryDataType::TEXTUAL: {
            std::string line;
            if (std::getline(fileStream_, line)) {
                auto textData = std::make_shared<TextualData>(line);
                textData->source = filePath_;
                textData->confidence = 1.0;
                data = textData;
            }
            break;
        }
        
        case SensoryDataType::VISUAL: {
            // Read binary image data (simplified)
            std::vector<char> buffer(1024); // Read 1KB chunks
            fileStream_.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = fileStream_.gcount();
            
            if (bytesRead > 0) {
                auto visualData = std::make_shared<VisualData>();
                visualData->rawData.assign(buffer.begin(), buffer.begin() + bytesRead);
                visualData->source = filePath_;
                visualData->confidence = 1.0;
                
                // Try to parse metadata from config
                std::lock_guard<std::mutex> lock(configMutex_);
                auto it = config_.find("width");
                if (it != config_.end()) {
                    visualData->width = std::stoi(it->second);
                }
                it = config_.find("height");
                if (it != config_.end()) {
                    visualData->height = std::stoi(it->second);
                }
                it = config_.find("channels");
                if (it != config_.end()) {
                    visualData->channels = std::stoi(it->second);
                }
                it = config_.find("format");
                if (it != config_.end()) {
                    visualData->format = it->second;
                }
                
                data = visualData;
            }
            break;
        }
        
        case SensoryDataType::AUDITORY: {
            // Read binary audio data (simplified)
            std::vector<char> buffer(4096); // Read 4KB chunks
            fileStream_.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = fileStream_.gcount();
            
            if (bytesRead > 0) {
                auto audioData = std::make_shared<AudioData>();
                audioData->rawData.assign(buffer.begin(), buffer.begin() + bytesRead);
                audioData->source = filePath_;
                audioData->confidence = 1.0;
                
                // Try to parse metadata from config
                std::lock_guard<std::mutex> lock(configMutex_);
                auto it = config_.find("sample_rate");
                if (it != config_.end()) {
                    audioData->sampleRate = std::stoi(it->second);
                }
                it = config_.find("channels");
                if (it != config_.end()) {
                    audioData->channels = std::stoi(it->second);
                }
                it = config_.find("duration");
                if (it != config_.end()) {
                    audioData->durationSeconds = std::stod(it->second);
                }
                it = config_.find("encoding");
                if (it != config_.end()) {
                    audioData->encoding = it->second;
                }
                
                data = audioData;
            }
            break;
        }
        
        case SensoryDataType::ENVIRONMENTAL: {
            // Read environmental data from CSV-like format
            std::string line;
            if (std::getline(fileStream_, line)) {
                auto envData = std::make_shared<EnvironmentalData>();
                envData->source = filePath_;
                envData->confidence = 1.0;
                
                // Parse CSV line: temp,humidity,pressure,light,ax,ay,az,gx,gy,gz
                std::istringstream iss(line);
                std::string token;
                std::vector<std::string> tokens;
                
                while (std::getline(iss, token, ',')) {
                    tokens.push_back(token);
                }
                
                if (tokens.size() >= 10) {
                    try {
                        envData->temperature = std::stod(tokens[0]);
                        envData->humidity = std::stod(tokens[1]);
                        envData->pressure = std::stod(tokens[2]);
                        envData->lightLevel = std::stod(tokens[3]);
                        envData->acceleration = {std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6])};
                        envData->gyroscope = {std::stod(tokens[7]), std::stod(tokens[8]), std::stod(tokens[9])};
                    } catch (const std::exception& e) {
                        AgentLogger logger;
                        logger.logWarning("Error parsing environmental data: " + std::string(e.what()));
                        envData->confidence = 0.5; // Reduce confidence due to parsing error
                    }
                }
                
                data = envData;
            }
            break;
        }
        
        default: {
            // Generic binary data
            std::vector<char> buffer(1024);
            fileStream_.read(buffer.data(), buffer.size());
            std::streamsize bytesRead = fileStream_.gcount();
            
            if (bytesRead > 0) {
                data = std::make_shared<SensoryData>(type_, filePath_);
                data->rawData.assign(buffer.begin(), buffer.begin() + bytesRead);
                data->confidence = 1.0;
            }
            break;
        }
    }
    
    // Call callback if real-time processing is enabled
    if (data && realTimeProcessing_ && dataCallback_) {
        try {
            dataCallback_(data);
        } catch (const std::exception& e) {
            AgentLogger logger;
            logger.logError("Error in data callback: " + std::string(e.what()));
        }
    }
    
    return data;
}

std::vector<std::shared_ptr<SensoryData>> FileSensoryInterface::readDataBuffer(size_t maxItems) {
    std::vector<std::shared_ptr<SensoryData>> buffer;
    
    for (size_t i = 0; i < maxItems; ++i) {
        auto data = readData();
        if (!data) {
            break; // No more data available
        }
        buffer.push_back(data);
    }
    
    return buffer;
}

bool FileSensoryInterface::hasData() const {
    if (!active_ || !fileStream_.is_open()) {
        return false;
    }
    
    // Check if we can read more data
    return !fileStream_.eof() && fileStream_.good();
}

void FileSensoryInterface::setConfiguration(const std::unordered_map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(configMutex_);
    config_ = config;
    
    AgentLogger logger;
    logger.logInfo("Updated configuration for File Sensory Interface: " + filePath_);
}

std::unordered_map<std::string, std::string> FileSensoryInterface::getConfiguration() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return config_;
}

void FileSensoryInterface::setDataCallback(std::function<void(std::shared_ptr<SensoryData>)> callback) {
    dataCallback_ = callback;
}

void FileSensoryInterface::enableRealTimeProcessing(bool enable) {
    realTimeProcessing_ = enable;
    
    AgentLogger logger;
    if (enable) {
        logger.logInfo("Enabled real-time processing for File Sensory Interface: " + filePath_);
    } else {
        logger.logInfo("Disabled real-time processing for File Sensory Interface: " + filePath_);
    }
}

} // namespace elizaos