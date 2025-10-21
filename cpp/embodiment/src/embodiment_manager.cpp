#include "elizaos/embodiment.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>
#include <sstream>
#include <chrono>

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
 * Embodiment Manager Implementation
 */
EmbodimentManager::EmbodimentManager() {}

EmbodimentManager::~EmbodimentManager() {
    if (running_) {
        stop();
    }
}

bool EmbodimentManager::initialize() {
    
    elogSystem("Initializing Embodiment Manager");
    
    if (!state_) {
        elogError("State not set - cannot initialize");
        return false;
    }
    
    if (!memory_) {
        elogError("Memory manager not set - cannot initialize");
        return false;
    }
    
    // Create perception-action loop
    perceptionActionLoop_ = std::make_shared<PerceptionActionLoop>(state_, memory_, cognition_);
    
    if (!perceptionActionLoop_->initialize()) {
        elogError("Failed to initialize perception-action loop");
        return false;
    }
    
    elogSuccess("Embodiment Manager initialized successfully");
    return true;
}

void EmbodimentManager::shutdown() {
    
    elogSystem("Shutting down Embodiment Manager");
    
    if (running_) {
        stop();
    }
    
    if (perceptionActionLoop_) {
        perceptionActionLoop_->shutdown();
    }
    
    elogInfo("Embodiment Manager shutdown complete");
}

bool EmbodimentManager::start() {
    if (running_) {
        return true; // Already running
    }
    
    
    elogSystem("Starting Embodiment Manager");
    
    if (!initialize()) {
        return false;
    }
    
    // Start perception-action loop
    if (perceptionActionLoop_ && !perceptionActionLoop_->start()) {
        elogError("Failed to start perception-action loop");
        return false;
    }
    
    // Start agent loop if available
    if (agentLoop_) {
        agentLoop_->start();
    }
    
    running_ = true;
    
    // Start continuous validation if enabled
    if (continuousValidation_) {
        validationThread_ = std::make_unique<std::thread>(&EmbodimentManager::coherenceValidationLoop, this);
    }
    
    elogSuccess("Embodiment Manager started");
    return true;
}

void EmbodimentManager::stop() {
    if (!running_) {
        return;
    }
    
    
    elogSystem("Stopping Embodiment Manager");
    
    running_ = false;
    
    // Stop validation thread
    if (validationThread_ && validationThread_->joinable()) {
        validationThread_->join();
    }
    
    // Stop perception-action loop
    if (perceptionActionLoop_) {
        perceptionActionLoop_->stop();
    }
    
    // Stop agent loop if available
    if (agentLoop_) {
        agentLoop_->stop();
    }
    
    elogInfo("Embodiment Manager stopped");
}

void EmbodimentManager::configurePerceptionActionLoop(std::chrono::milliseconds interval) {
    if (!perceptionActionLoop_) {
        perceptionActionLoop_ = std::make_shared<PerceptionActionLoop>(state_, memory_, cognition_);
    }
    
    perceptionActionLoop_->setLoopInterval(interval);
    
    
    elogInfo("Configured perception-action loop with " + std::to_string(interval.count()) + "ms interval");
}

void EmbodimentManager::registerSensoryInterface(std::shared_ptr<SensoryInterface> interface) {
    if (!interface) return;
    
    if (perceptionActionLoop_) {
        perceptionActionLoop_->addSensoryInterface(interface);
    }
    
    
    elogInfo("Registered sensory interface: " + interface->getName());
}

void EmbodimentManager::registerMotorInterface(std::shared_ptr<MotorInterface> interface) {
    if (!interface) return;
    
    if (perceptionActionLoop_) {
        perceptionActionLoop_->addMotorInterface(interface);
    }
    
    
    elogInfo("Registered motor interface: " + interface->getName());
}

void EmbodimentManager::createDefaultInterfaces() {
    
    elogInfo("Creating default interfaces");
    
    // Create console text interfaces (both sensory and motor)
    auto consoleInput = std::make_shared<ConsoleTextInput>();
    auto consoleOutput = std::make_shared<ConsoleTextOutput>();
    registerSensoryInterface(consoleInput);
    registerMotorInterface(consoleOutput);
    
    // Create mock motor interfaces for different action types
    auto speechInterface = std::make_shared<MockMotorInterface>(MotorActionType::SPEECH);
    auto movementInterface = std::make_shared<MockMotorInterface>(MotorActionType::MOVEMENT);
    auto displayInterface = std::make_shared<MockMotorInterface>(MotorActionType::DISPLAY);
    auto gestureInterface = std::make_shared<MockMotorInterface>(MotorActionType::GESTURE);
    auto manipulationInterface = std::make_shared<MockMotorInterface>(MotorActionType::MANIPULATION);
    
    registerMotorInterface(speechInterface);
    registerMotorInterface(movementInterface);
    registerMotorInterface(displayInterface);
    registerMotorInterface(gestureInterface);
    registerMotorInterface(manipulationInterface);
    
    elogSuccess("Default interfaces created");
}

EmbodimentManager::CoherenceReport EmbodimentManager::validateSystemCoherence() {
    CoherenceReport report;
    report.timestamp = std::chrono::system_clock::now();
    
    
    elogInfo("Validating system coherence");
    
    std::vector<std::string> issues;
    std::vector<std::string> warnings;
    std::unordered_map<std::string, double> metrics;
    
    // Check core components
    if (!state_) {
        issues.push_back("State component is missing");
    } else {
        metrics["state_actors"] = state_->getActors().size();
        metrics["state_goals"] = state_->getGoals().size();
        metrics["state_recent_messages"] = state_->getRecentMessages().size();
    }
    
    if (!memory_) {
        issues.push_back("Memory manager is missing");
    } else {
        // Check memory health
        try {
            MemorySearchByEmbeddingParams params;
            params.embedding = EmbeddingVector(384, 0.0f); // Default empty embedding
            params.count = 1;
            auto memories = memory_->searchMemories(params);
            metrics["memory_accessible"] = 1.0;
        } catch (...) {
            warnings.push_back("Memory manager may not be functioning correctly");
            metrics["memory_accessible"] = 0.0;
        }
    }
    
    // Check perception-action loop
    if (!perceptionActionLoop_) {
        issues.push_back("Perception-action loop is missing");
    } else {
        metrics["pal_running"] = perceptionActionLoop_->isRunning() ? 1.0 : 0.0;
        metrics["pal_cycles"] = static_cast<double>(perceptionActionLoop_->getCycleCount());
        metrics["pal_avg_loop_time"] = perceptionActionLoop_->getAverageLoopTime().count();
        metrics["pal_perception_latency"] = perceptionActionLoop_->getPerceptionLatency();
        metrics["pal_action_latency"] = perceptionActionLoop_->getActionLatency();
        
        if (perceptionActionLoop_->getAverageLoopTime().count() > 1000) {
            warnings.push_back("Perception-action loop is running slowly (>1s per cycle)");
        }
    }
    
    // Check agent loop integration
    if (agentLoop_) {
        metrics["agent_loop_running"] = agentLoop_->isRunning() ? 1.0 : 0.0;
        metrics["agent_loop_paused"] = agentLoop_->isPaused() ? 1.0 : 0.0;
    } else {
        warnings.push_back("Agent loop not integrated");
    }
    
    // Check cognitive fusion engine
    if (cognition_) {
        metrics["cognition_available"] = 1.0;
        auto nodes = cognition_->getAtomSpaceNodes();
        auto edges = cognition_->getAtomSpaceEdges();
        metrics["atomspace_nodes"] = static_cast<double>(nodes.size());
        metrics["atomspace_edges"] = static_cast<double>(edges.size());
    } else {
        warnings.push_back("Cognitive fusion engine not available - using simple reactive behavior");
        metrics["cognition_available"] = 0.0;
    }
    
    // Overall coherence assessment
    bool coherent = issues.empty();
    if (coherent && warnings.size() > 3) {
        coherent = false;
        issues.push_back("Too many warnings indicate system instability");
    }
    
    // Performance metrics validation
    if (metrics["pal_avg_loop_time"] > 5000) { // 5 seconds
        coherent = false;
        issues.push_back("Perception-action loop performance is unacceptable");
    }
    
    report.overallCoherent = coherent;
    report.issues = issues;
    report.warnings = warnings;
    report.metrics = metrics;
    
    // Log report
    if (coherent) {
        elogSuccess("System coherence validation passed");
    } else {
        elogWarning("System coherence validation found issues");
        for (const auto& issue : issues) {
            elogError("  Issue: " + issue);
        }
    }
    
    for (const auto& warning : warnings) {
        elogWarning("  Warning: " + warning);
    }
    
    lastCoherenceReport_ = report;
    return report;
}

} // End anonymous namespace

namespace elizaos {

void EmbodimentManager::enableContinuousValidation(bool enable, std::chrono::seconds interval) {
    continuousValidation_ = enable;
    validationInterval_ = interval;
    
    
    if (enable) {
        elogInfo("Enabled continuous validation with " + std::to_string(interval.count()) + "s interval");
        
        if (running_ && !validationThread_) {
            validationThread_ = std::make_unique<std::thread>(&EmbodimentManager::coherenceValidationLoop, this);
        }
    } else {
        elogInfo("Disabled continuous validation");
    }
}

bool EmbodimentManager::testSensoryIntegration() {
    
    elogInfo("Testing sensory integration");
    
    if (!perceptionActionLoop_) {
        elogError("Perception-action loop not available");
        return false;
    }
    
    // Create test sensory data
    auto testData = std::make_shared<TextualData>("Test sensory input");
    std::vector<std::shared_ptr<SensoryData>> testVector = {testData};
    
    try {
        // Test processing
        perceptionActionLoop_->processPerception(testVector);
        elogSuccess("Sensory integration test passed");
        return true;
    } catch (const std::exception& e) {
        elogError("Sensory integration test failed: " + std::string(e.what()));
        return false;
    }
}

bool EmbodimentManager::testMotorIntegration() {
    
    elogInfo("Testing motor integration");
    
    if (!perceptionActionLoop_) {
        elogError("Perception-action loop not available");
        return false;
    }
    
    // Create test motor action
    auto testAction = std::make_shared<CommunicationAction>("Test motor output", "test-recipient");
    std::vector<std::shared_ptr<MotorAction>> testVector = {testAction};
    
    try {
        // Test execution
        perceptionActionLoop_->executeActions(testVector);
        elogSuccess("Motor integration test passed");
        return true;
    } catch (const std::exception& e) {
        elogError("Motor integration test failed: " + std::string(e.what()));
        return false;
    }
}

bool EmbodimentManager::testPerceptionActionLoop() {
    
    elogInfo("Testing perception-action loop");
    
    if (!perceptionActionLoop_) {
        elogError("Perception-action loop not available");
        return false;
    }
    
    try {
        // Test single cycle
        perceptionActionLoop_->processSingleCycle();
        elogSuccess("Perception-action loop test passed");
        return true;
    } catch (const std::exception& e) {
        elogError("Perception-action loop test failed: " + std::string(e.what()));
        return false;
    }
}

bool EmbodimentManager::testSystemIntegration() {
    
    elogInfo("Testing complete system integration");
    
    bool sensoryOk = testSensoryIntegration();
    bool motorOk = testMotorIntegration();
    bool loopOk = testPerceptionActionLoop();
    auto coherenceReport = validateSystemCoherence();
    
    bool success = sensoryOk && motorOk && loopOk && coherenceReport.overallCoherent;
    
    if (success) {
        elogSuccess("System integration test passed");
    } else {
        elogError("System integration test failed");
    }
    
    return success;
}

std::unordered_map<std::string, std::string> EmbodimentManager::getSystemStatus() const {
    std::unordered_map<std::string, std::string> status;
    
    status["running"] = running_ ? "true" : "false";
    status["continuous_validation"] = continuousValidation_ ? "true" : "false";
    
    if (state_) {
        status["state_agent_id"] = state_->getAgentId();
        status["state_actors"] = std::to_string(state_->getActors().size());
        status["state_goals"] = std::to_string(state_->getGoals().size());
        status["state_messages"] = std::to_string(state_->getRecentMessages().size());
    } else {
        status["state"] = "not_available";
    }
    
    if (memory_) {
        status["memory"] = "available";
    } else {
        status["memory"] = "not_available";
    }
    
    if (perceptionActionLoop_) {
        status["pal_running"] = perceptionActionLoop_->isRunning() ? "true" : "false";
        status["pal_paused"] = perceptionActionLoop_->isPaused() ? "true" : "false";
        status["pal_cycles"] = std::to_string(perceptionActionLoop_->getCycleCount());
    } else {
        status["perception_action_loop"] = "not_available";
    }
    
    if (agentLoop_) {
        status["agent_loop_running"] = agentLoop_->isRunning() ? "true" : "false";
        status["agent_loop_paused"] = agentLoop_->isPaused() ? "true" : "false";
    } else {
        status["agent_loop"] = "not_integrated";
    }
    
    if (cognition_) {
        status["cognition"] = "available";
        status["atomspace_nodes"] = std::to_string(cognition_->getAtomSpaceNodes().size());
        status["atomspace_edges"] = std::to_string(cognition_->getAtomSpaceEdges().size());
    } else {
        status["cognition"] = "not_available";
    }
    
    return status;
}

std::unordered_map<std::string, double> EmbodimentManager::getPerformanceMetrics() const {
    updateSystemMetrics();
    return performanceMetrics_;
}

void EmbodimentManager::coherenceValidationLoop() {
    
    elogSystem("Continuous coherence validation started");
    
    while (running_ && continuousValidation_) {
        try {
            validateSystemCoherence();
            updateSystemMetrics();
        } catch (const std::exception& e) {
            elogError("Error in coherence validation: " + std::string(e.what()));
        }
        
        std::this_thread::sleep_for(validationInterval_);
    }
    
    elogSystem("Continuous coherence validation ended");
}

void EmbodimentManager::updateSystemMetrics() const {
    std::lock_guard<std::mutex> lock(systemMutex_);
    
    // Update performance metrics - need to make performanceMetrics_ mutable
    // For now, let's create a local copy and return it
    auto& metrics = const_cast<std::unordered_map<std::string, double>&>(performanceMetrics_);
    
    // Update performance metrics
    if (perceptionActionLoop_) {
        metrics["pal_avg_loop_time"] = static_cast<double>(
            perceptionActionLoop_->getAverageLoopTime().count());
        metrics["pal_perception_latency"] = perceptionActionLoop_->getPerceptionLatency();
        metrics["pal_action_latency"] = perceptionActionLoop_->getActionLatency();
        metrics["pal_cycle_count"] = static_cast<double>(perceptionActionLoop_->getCycleCount());
        metrics["pal_running"] = perceptionActionLoop_->isRunning() ? 1.0 : 0.0;
    }
    
    if (state_) {
        metrics["state_actors"] = static_cast<double>(state_->getActors().size());
        metrics["state_goals"] = static_cast<double>(state_->getGoals().size());
        metrics["state_messages"] = static_cast<double>(state_->getRecentMessages().size());
    }
    
    if (cognition_) {
        metrics["atomspace_nodes"] = static_cast<double>(cognition_->getAtomSpaceNodes().size());
        metrics["atomspace_edges"] = static_cast<double>(cognition_->getAtomSpaceEdges().size());
    }
    
    // System health metrics
    metrics["system_running"] = running_ ? 1.0 : 0.0;
    metrics["continuous_validation"] = continuousValidation_ ? 1.0 : 0.0;
    metrics["last_coherence_check"] = lastCoherenceReport_.overallCoherent ? 1.0 : 0.0;
}

} // namespace elizaos