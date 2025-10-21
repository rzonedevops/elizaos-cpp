#include "elizaos/embodiment.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <sstream>

namespace elizaos {

// Helper functions for logging
[[maybe_unused]] static void elogInfo(const std::string& message) {
    AgentLogger logger;
    logger.log(message, "", "embodiment", LogLevel::INFO);
}

[[maybe_unused]] static void elogSuccess(const std::string& message) {
    AgentLogger logger;
    logger.log(message, "", "embodiment", LogLevel::SUCCESS);
}

[[maybe_unused]] static void elogError(const std::string& message) {
    AgentLogger logger;
    logger.log(message, "", "embodiment", LogLevel::ERROR);
}

[[maybe_unused]] static void elogSystem(const std::string& message) {
    AgentLogger logger;
    logger.log(message, "", "embodiment", LogLevel::SYSTEM);
}

[[maybe_unused]] static void elogWarning(const std::string& message) {
    AgentLogger logger;
    logger.log(message, "", "embodiment", LogLevel::WARNING);
}

/**
 * Perception-Action Loop Implementation
 */
PerceptionActionLoop::PerceptionActionLoop(std::shared_ptr<State> state, 
                                          std::shared_ptr<AgentMemoryManager> memory,
                                          std::shared_ptr<CognitiveFusionEngine> cognition)
    : state_(state), memory_(memory), cognition_(cognition) {
    
    if (!state_) {
        throw std::invalid_argument("State cannot be null");
    }
    if (!memory_) {
        throw std::invalid_argument("Memory manager cannot be null");
    }
}

PerceptionActionLoop::~PerceptionActionLoop() {
    if (running_) {
        stop();
    }
}

bool PerceptionActionLoop::initialize() {
    
    elogSystem("Initializing Perception-Action Loop");
    
    // Initialize all sensory interfaces
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    
    for (auto& pair : sensoryInterfaces_) {
        if (!pair.second->initialize()) {
            elogError("Failed to initialize sensory interface: " + pair.first);
            return false;
        }
    }
    
    // Initialize all motor interfaces
    for (auto& pair : motorInterfaces_) {
        if (!pair.second->initialize()) {
            elogError("Failed to initialize motor interface: " + pair.first);
            return false;
        }
    }
    
    elogSuccess("Perception-Action Loop initialized successfully");
    return true;
}

void PerceptionActionLoop::shutdown() {
    
    elogSystem("Shutting down Perception-Action Loop");
    
    if (running_) {
        stop();
    }
    
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    
    // Shutdown all interfaces
    for (auto& pair : sensoryInterfaces_) {
        pair.second->shutdown();
    }
    
    for (auto& pair : motorInterfaces_) {
        pair.second->shutdown();
    }
    
    elogInfo("Perception-Action Loop shutdown complete");
}

bool PerceptionActionLoop::start() {
    if (running_) {
        return true; // Already running
    }
    
    
    elogSystem("Starting Perception-Action Loop");
    
    if (!initialize()) {
        return false;
    }
    
    running_ = true;
    paused_ = false;
    cycleCount_ = 0;
    
    loopThread_ = std::make_unique<std::thread>(&PerceptionActionLoop::mainLoop, this);
    
    elogSuccess("Perception-Action Loop started");
    return true;
}

void PerceptionActionLoop::stop() {
    if (!running_) {
        return;
    }
    
    
    elogSystem("Stopping Perception-Action Loop");
    
    running_ = false;
    
    if (loopThread_ && loopThread_->joinable()) {
        loopThread_->join();
    }
    
    elogInfo("Perception-Action Loop stopped");
}

void PerceptionActionLoop::pause() {
    paused_ = true;
    
    
    elogInfo("Perception-Action Loop paused");
}

void PerceptionActionLoop::resume() {
    paused_ = false;
    
    
    elogInfo("Perception-Action Loop resumed");
}

void PerceptionActionLoop::addSensoryInterface(std::shared_ptr<SensoryInterface> interface) {
    if (!interface) return;
    
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    sensoryInterfaces_[interface->getName()] = interface;
    
    
    elogInfo("Added sensory interface: " + interface->getName());
}

void PerceptionActionLoop::addMotorInterface(std::shared_ptr<MotorInterface> interface) {
    if (!interface) return;
    
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    motorInterfaces_[interface->getName()] = interface;
    
    
    elogInfo("Added motor interface: " + interface->getName());
}

void PerceptionActionLoop::removeSensoryInterface(const std::string& name) {
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    
    auto it = sensoryInterfaces_.find(name);
    if (it != sensoryInterfaces_.end()) {
        it->second->shutdown();
        sensoryInterfaces_.erase(it);
        
        
        elogInfo("Removed sensory interface: " + name);
    }
}

void PerceptionActionLoop::removeMotorInterface(const std::string& name) {
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    
    auto it = motorInterfaces_.find(name);
    if (it != motorInterfaces_.end()) {
        it->second->shutdown();
        motorInterfaces_.erase(it);
        
        
        elogInfo("Removed motor interface: " + name);
    }
}

void PerceptionActionLoop::setPerceptionProcessingCallback(
    std::function<void(std::vector<std::shared_ptr<SensoryData>>)> callback) {
    perceptionCallback_ = callback;
}

void PerceptionActionLoop::setActionDecisionCallback(
    std::function<std::vector<std::shared_ptr<MotorAction>>(
        const State&, const std::vector<std::shared_ptr<SensoryData>>&)> callback) {
    actionDecisionCallback_ = callback;
}

std::chrono::milliseconds PerceptionActionLoop::getAverageLoopTime() const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    if (loopTimes_.empty()) {
        return std::chrono::milliseconds(0);
    }
    
    auto total = std::chrono::milliseconds(0);
    for (const auto& time : loopTimes_) {
        total += time;
    }
    
    return total / loopTimes_.size();
}

void PerceptionActionLoop::processSingleCycle() {
    auto start = std::chrono::steady_clock::now();
    
    // 1. Gather sensory data
    auto sensoryData = gatherSensoryData();
    
    // 2. Update state with sensory information
    updateState(sensoryData);
    
    // 3. Process perception (optional callback)
    if (perceptionCallback_) {
        perceptionCallback_(sensoryData);
    }
    
    // 4. Determine actions based on perception
    auto actions = processPerception(sensoryData);
    
    // 5. Execute actions
    executeActions(actions);
    
    // 6. Update metrics
    auto end = std::chrono::steady_clock::now();
    auto cycleTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        loopTimes_.push_back(cycleTime);
        
        // Keep only recent loop times (last 100)
        if (loopTimes_.size() > 100) {
            loopTimes_.erase(loopTimes_.begin());
        }
    }
    
    cycleCount_++;
}

std::vector<std::shared_ptr<SensoryData>> PerceptionActionLoop::gatherSensoryData() {
    auto start = std::chrono::steady_clock::now();
    
    std::vector<std::shared_ptr<SensoryData>> allData;
    
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    
    for (const auto& pair : sensoryInterfaces_) {
        if (!pair.second->isActive()) continue;
        
        try {
            auto buffer = pair.second->readDataBuffer(10); // Read up to 10 items
            allData.insert(allData.end(), buffer.begin(), buffer.end());
        } catch (const std::exception& e) {
            
            elogError("Error reading from sensory interface " + pair.first + ": " + e.what());
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    perceptionLatency_ = duration.count();
    
    return allData;
}

std::vector<std::shared_ptr<MotorAction>> PerceptionActionLoop::processPerception(
    const std::vector<std::shared_ptr<SensoryData>>& sensoryData) {
    
    std::vector<std::shared_ptr<MotorAction>> actions;
    
    // Use custom decision callback if provided
    if (actionDecisionCallback_) {
        return actionDecisionCallback_(*state_, sensoryData);
    }
    
    // Default decision making logic
    if (!sensoryData.empty()) {
        // Simple reactive behavior: respond to text input
        for (const auto& data : sensoryData) {
            if (data->type == SensoryDataType::TEXTUAL) {
                auto textData = std::dynamic_pointer_cast<TextualData>(data);
                if (textData && !textData->text.empty()) {
                    // Create a communication response
                    auto response = std::make_shared<CommunicationAction>();
                    response->message = "Processed: " + textData->text;
                    response->channel = "default";
                    actions.push_back(response);
                }
            }
        }
    }
    
    // Use cognitive fusion engine if available
    if (cognition_) {
        for (const auto& data : sensoryData) {
            if (data->type == SensoryDataType::TEXTUAL) {
                auto textData = std::dynamic_pointer_cast<TextualData>(data);
                if (textData) {
                    auto reasoningResult = cognition_->processQuery(*state_, textData->text);
                    
                    // Convert reasoning results to actions
                    for (const auto& result : reasoningResult.fusedResults) {
                        auto action = std::make_shared<CommunicationAction>();
                        action->message = result;
                        action->channel = "cognitive";
                        actions.push_back(action);
                    }
                }
            }
        }
    }
    
    return actions;
}

void PerceptionActionLoop::executeActions(const std::vector<std::shared_ptr<MotorAction>>& actions) {
    auto start = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(interfacesMutex_);
    
    for (const auto& action : actions) {
        // Find appropriate motor interface for this action type
        for (const auto& pair : motorInterfaces_) {
            if (!pair.second->isActive()) continue;
            
            if (pair.second->canExecute(action)) {
                try {
                    pair.second->executeAction(action);
                } catch (const std::exception& e) {
                    
                    elogError("Error executing action via " + pair.first + ": " + e.what());
                }
                break; // Action executed, move to next
            }
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    actionLatency_ = duration.count();
}

void PerceptionActionLoop::mainLoop() {
    
    elogSystem("Perception-Action Loop main loop started");
    
    while (running_) {
        if (!paused_) {
            try {
                processSingleCycle();
                logCycleMetrics();
            } catch (const std::exception& e) {
                elogError("Error in perception-action cycle: " + std::string(e.what()));
            }
        }
        
        // Sleep for the configured interval
        std::this_thread::sleep_for(loopInterval_);
    }
    
    elogSystem("Perception-Action Loop main loop ended");
}

void PerceptionActionLoop::updateState(const std::vector<std::shared_ptr<SensoryData>>& sensoryData) {
    if (!memory_) return;
    
    // Convert sensory data to memories and add to state
    for (const auto& data : sensoryData) {
        if (data->type == SensoryDataType::TEXTUAL) {
            auto textData = std::dynamic_pointer_cast<TextualData>(data);
            if (textData) {
                auto memory = std::make_shared<Memory>(
                    generateUUID(),
                    "Sensory input: " + textData->text,
                    "sensory-entity",
                    state_->getAgentId()
                );
                
                // Add to state and memory manager
                state_->addRecentMessage(memory);
                
                try {
                    memory_->createMemory(memory);
                } catch (const std::exception& e) {
                    
                    elogWarning("Failed to store sensory memory: " + std::string(e.what()));
                }
            }
        }
    }
}

void PerceptionActionLoop::logCycleMetrics() {
    static size_t lastLoggedCycle = 0;
    static const size_t LOG_INTERVAL = 100; // Log every 100 cycles
    
    if (cycleCount_ % LOG_INTERVAL == 0 && cycleCount_ != lastLoggedCycle) {
        
        
        std::stringstream ss;
        ss << "Perception-Action Loop Metrics (Cycle " << cycleCount_ << "):\n";
        ss << "  Average loop time: " << getAverageLoopTime().count() << "ms\n";
        ss << "  Perception latency: " << perceptionLatency_.load() << "ms\n";
        ss << "  Action latency: " << actionLatency_.load() << "ms\n";
        ss << "  Active sensory interfaces: " << sensoryInterfaces_.size() << "\n";
        ss << "  Active motor interfaces: " << motorInterfaces_.size();
        
        elogInfo(ss.str());
        lastLoggedCycle = cycleCount_;
    }
}

} // namespace elizaos