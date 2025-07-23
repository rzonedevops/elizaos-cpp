#include "elizaos/embodiment.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace elizaos {

/**
 * Console Text Interface Implementation
 */
ConsoleTextInterface::ConsoleTextInterface() {}

bool ConsoleTextInterface::initialize() {
    AgentLogger logger;
    logger.logInfo("Initializing Console Text Interface");
    
    if (active_) {
        return true; // Already initialized
    }
    
    active_ = true;
    
    // Start input thread for sensory input
    inputThread_ = std::make_unique<std::thread>(&ConsoleTextInterface::inputThread, this);
    
    logger.logSuccess("Console Text Interface initialized");
    return true;
}

void ConsoleTextInterface::shutdown() {
    if (!active_) {
        return;
    }
    
    AgentLogger logger;
    logger.logInfo("Shutting down Console Text Interface");
    
    active_ = false;
    
    if (inputThread_ && inputThread_->joinable()) {
        inputThread_->join();
    }
    
    logger.logInfo("Console Text Interface shutdown complete");
}

std::shared_ptr<SensoryData> ConsoleTextInterface::readData() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    
    if (inputBuffer_.empty()) {
        return nullptr;
    }
    
    std::string input = inputBuffer_.front();
    inputBuffer_.erase(inputBuffer_.begin());
    
    auto textData = std::make_shared<TextualData>(input);
    textData->source = "console";
    textData->confidence = 1.0; // Console input is always certain
    
    return textData;
}

std::vector<std::shared_ptr<SensoryData>> ConsoleTextInterface::readDataBuffer(size_t maxItems) {
    std::vector<std::shared_ptr<SensoryData>> result;
    
    std::lock_guard<std::mutex> lock(bufferMutex_);
    
    size_t count = std::min(maxItems, inputBuffer_.size());
    for (size_t i = 0; i < count; ++i) {
        auto textData = std::make_shared<TextualData>(inputBuffer_[i]);
        textData->source = "console";
        textData->confidence = 1.0;
        result.push_back(textData);
    }
    
    // Remove processed items
    inputBuffer_.erase(inputBuffer_.begin(), inputBuffer_.begin() + count);
    
    return result;
}

bool ConsoleTextInterface::hasData() const {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    return !inputBuffer_.empty();
}

void ConsoleTextInterface::setConfiguration(const std::unordered_map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(configMutex_);
    config_ = config;
}

std::unordered_map<std::string, std::string> ConsoleTextInterface::getConfiguration() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return config_;
}

void ConsoleTextInterface::setDataCallback(std::function<void(std::shared_ptr<SensoryData>)> callback) {
    dataCallback_ = callback;
}

void ConsoleTextInterface::enableRealTimeProcessing(bool enable) {
    realTimeProcessing_ = enable;
}

// Motor interface implementation
bool ConsoleTextInterface::executeAction(std::shared_ptr<MotorAction> action) {
    if (!active_) {
        return false;
    }
    
    // Handle communication actions (text output)
    if (action->type == MotorActionType::COMMUNICATION) {
        auto commAction = std::dynamic_pointer_cast<CommunicationAction>(action);
        if (commAction) {
            AgentLogger logger;
            
            // Use panel display for agent responses
            logger.panel("Agent Response", commAction->message);
            
            return true;
        }
    }
    
    // Handle display actions
    if (action->type == MotorActionType::DISPLAY) {
        auto displayAction = std::dynamic_pointer_cast<DisplayAction>(action);
        if (displayAction) {
            std::cout << "[DISPLAY] " << displayAction->content << std::endl;
            return true;
        }
    }
    
    // Handle speech actions (as text output)
    if (action->type == MotorActionType::SPEECH) {
        auto speechAction = std::dynamic_pointer_cast<SpeechAction>(action);
        if (speechAction) {
            std::cout << "[SPEECH] " << speechAction->text << std::endl;
            return true;
        }
    }
    
    return false;
}

bool ConsoleTextInterface::canExecute(std::shared_ptr<MotorAction> action) const {
    if (!action) return false;
    
    // Console interface can handle communication, display, and speech actions
    return (action->type == MotorActionType::COMMUNICATION ||
            action->type == MotorActionType::DISPLAY ||
            action->type == MotorActionType::SPEECH);
}

void ConsoleTextInterface::stopAction(const std::string& actionId) {
    // Console actions are immediate, no stopping needed
    (void)actionId; // Suppress unused parameter warning
}

void ConsoleTextInterface::stopAllActions() {
    // Console actions are immediate, no stopping needed
}

bool ConsoleTextInterface::isActionComplete(const std::string& actionId) const {
    // Console actions complete immediately
    (void)actionId; // Suppress unused parameter warning
    return true;
}

std::vector<std::string> ConsoleTextInterface::getActiveActions() const {
    // Console actions don't remain active
    return {};
}

double ConsoleTextInterface::getActionProgress(const std::string& actionId) const {
    // Console actions complete immediately
    (void)actionId; // Suppress unused parameter warning
    return 1.0;
}

void ConsoleTextInterface::inputThread() {
    AgentLogger logger;
    logger.logSystem("Console input thread started");
    
    std::cout << std::endl;
    std::cout << "=== ElizaOS Console Interface ===" << std::endl;
    std::cout << "Type messages to interact with the agent. Type 'quit' to exit." << std::endl;
    std::cout << std::endl;
    
    while (active_) {
        std::cout << "> ";
        std::cout.flush();
        
        std::string input;
        if (std::getline(std::cin, input)) {
            if (!active_) break; // Check if shutdown was requested
            
            if (input == "quit" || input == "exit") {
                AgentLogger logger;
                logger.logInfo("User requested exit");
                break;
            }
            
            if (!input.empty()) {
                // Add to input buffer
                {
                    std::lock_guard<std::mutex> lock(bufferMutex_);
                    inputBuffer_.push_back(input);
                }
                
                // If real-time processing is enabled, call callback immediately
                if (realTimeProcessing_ && dataCallback_) {
                    auto textData = std::make_shared<TextualData>(input);
                    textData->source = "console";
                    textData->confidence = 1.0;
                    
                    try {
                        dataCallback_(textData);
                    } catch (const std::exception& e) {
                        AgentLogger logger;
                        logger.logError("Error in data callback: " + std::string(e.what()));
                    }
                }
            }
        } else {
            // EOF or error condition
            break;
        }
        
        // Small delay to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    logger.logSystem("Console input thread ended");
}

} // namespace elizaos