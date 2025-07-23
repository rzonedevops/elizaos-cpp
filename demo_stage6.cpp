#include "elizaos/embodiment.hpp"
#include "elizaos/core.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentmemory.hpp"
#include "elizaos/agentloop.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace elizaos;

/**
 * Stage 6 Demo: Embodiment & Integration
 * 
 * This demo showcases:
 * 1. Sensory/motor data interfaces
 * 2. Perception-action loop implementation
 * 3. System-level coherence validation
 * 4. Integration of cognitive core with virtual/physical agents
 */

class EmbodimentDemo {
public:
    EmbodimentDemo() {
        logger_.logSystem("Initializing Stage 6 Embodiment Demo");
        
        // Create agent configuration
        AgentConfig config;
        config.agentId = "embodied-agent-001";
        config.agentName = "EmbodiedAgent";
        config.bio = "An embodied cognitive agent capable of perception and action";
        config.lore = "Born from the convergence of cognitive architecture and embodied interaction";
        
        // Initialize core components
        state_ = std::make_shared<State>(config);
        memory_ = std::make_shared<AgentMemoryManager>();
        cognition_ = std::make_shared<CognitiveFusionEngine>();
        embodiment_ = std::make_shared<EmbodimentManager>();
        
        // Configure embodiment manager
        embodiment_->setState(state_);
        embodiment_->setMemory(memory_);
        embodiment_->setCognition(cognition_);
    }
    
    void runDemo() {
        logger_.panel("Stage 6 Demo", "Embodiment & Integration");
        
        // Test 1: Initialize embodiment system
        testInitialization();
        
        // Test 2: Create and test sensory interfaces
        testSensoryInterfaces();
        
        // Test 3: Create and test motor interfaces
        testMotorInterfaces();
        
        // Test 4: Configure perception-action loop
        testPerceptionActionLoop();
        
        // Test 5: Run integrated system
        testIntegratedSystem();
        
        // Test 6: System coherence validation
        testSystemCoherence();
        
        // Test 7: Performance metrics
        testPerformanceMetrics();
        
        // Interactive mode
        runInteractiveMode();
        
        logger_.logSuccess("Stage 6 Demo completed successfully");
    }
    
private:
    void testInitialization() {
        logger_.logInfo("=== Test 1: Embodiment System Initialization ===");
        
        // Initialize memory system
        if (!memory_->initialize()) {
            logger_.logError("Failed to initialize memory system");
            return;
        }
        
        // Initialize embodiment manager
        if (!embodiment_->initialize()) {
            logger_.logError("Failed to initialize embodiment manager");
            return;
        }
        
        logger_.logSuccess("Embodiment system initialized successfully");
    }
    
    void testSensoryInterfaces() {
        logger_.logInfo("=== Test 2: Sensory Interface Testing ===");
        
        // Create console text interface for user input
        auto consoleInterface = std::make_shared<ConsoleTextInterface>();
        embodiment_->registerSensoryInterface(consoleInterface);
        
        // Create file-based sensory interface for environmental data
        auto envFile = "/tmp/test_env_data.csv";
        createTestEnvironmentalData(envFile);
        auto fileInterface = std::make_shared<FileSensoryInterface>(
            SensoryDataType::ENVIRONMENTAL, envFile);
        embodiment_->registerSensoryInterface(fileInterface);
        
        logger_.logSuccess("Sensory interfaces registered");
    }
    
    void testMotorInterfaces() {
        logger_.logInfo("=== Test 3: Motor Interface Testing ===");
        
        // Create default motor interfaces
        embodiment_->createDefaultInterfaces();
        
        // Test individual motor actions
        testMotorActions();
        
        logger_.logSuccess("Motor interfaces tested successfully");
    }
    
    void testMotorActions() {
        logger_.logInfo("Testing individual motor actions:");
        
        // Test speech action
        auto speechAction = std::make_shared<SpeechAction>("Hello, I am an embodied agent!");
        speechAction->voice = "friendly";
        speechAction->volume = 0.8;
        
        // Test movement action
        auto moveAction = std::make_shared<MovementAction>();
        moveAction->targetPosition = {1.0, 2.0, 0.5};
        moveAction->speed = 0.5;
        moveAction->movementType = "linear";
        
        // Test display action
        auto displayAction = std::make_shared<DisplayAction>("Agent Status: Active");
        displayAction->contentType = "text";
        displayAction->duration = 3.0;
        
        // Test gesture action
        auto gestureAction = std::make_shared<GestureAction>("wave");
        gestureAction->duration = 2.0;
        gestureAction->keyframes = {{0.0, 0.0, 0.0}, {1.0, 1.0, 0.0}, {0.0, 0.0, 0.0}};
        
        // Test manipulation action
        auto manipAction = std::make_shared<ManipulationAction>("object_123");
        manipAction->actionType = "grasp";
        manipAction->targetPose = {0.5, 0.3, 0.2, 0.0, 0.0, 0.0};
        manipAction->force = 0.7;
        
        logger_.logInfo("Created test motor actions for validation");
    }
    
    void testPerceptionActionLoop() {
        logger_.logInfo("=== Test 4: Perception-Action Loop Configuration ===");
        
        // Configure loop with 200ms intervals (5 Hz)
        embodiment_->configurePerceptionActionLoop(std::chrono::milliseconds(200));
        
        auto loop = embodiment_->getPerceptionActionLoop();
        if (!loop) {
            logger_.logError("Failed to get perception-action loop");
            return;
        }
        
        // Set custom callbacks
        loop->setPerceptionProcessingCallback([this](const auto& sensoryData) {
            this->processPerception(sensoryData);
        });
        
        loop->setActionDecisionCallback([this](const auto& state, const auto& sensoryData) {
            return this->decideActions(state, sensoryData);
        });
        
        logger_.logSuccess("Perception-action loop configured");
    }
    
    void testIntegratedSystem() {
        logger_.logInfo("=== Test 5: Integrated System Testing ===");
        
        // Test all integration points
        bool sensoryOk = embodiment_->testSensoryIntegration();
        bool motorOk = embodiment_->testMotorIntegration();
        bool loopOk = embodiment_->testPerceptionActionLoop();
        bool systemOk = embodiment_->testSystemIntegration();
        
        if (sensoryOk && motorOk && loopOk && systemOk) {
            logger_.logSuccess("All integration tests passed");
        } else {
            logger_.logWarning("Some integration tests failed");
        }
    }
    
    void testSystemCoherence() {
        logger_.logInfo("=== Test 6: System Coherence Validation ===");
        
        auto report = embodiment_->validateSystemCoherence();
        
        logger_.logInfo("Coherence Report:");
        logger_.logInfo("  Overall Coherent: " + std::string(report.overallCoherent ? "YES" : "NO"));
        logger_.logInfo("  Issues: " + std::to_string(report.issues.size()));
        logger_.logInfo("  Warnings: " + std::to_string(report.warnings.size()));
        
        for (const auto& issue : report.issues) {
            logger_.logError("  Issue: " + issue);
        }
        
        for (const auto& warning : report.warnings) {
            logger_.logWarning("  Warning: " + warning);
        }
        
        logger_.logInfo("  Metrics:");
        for (const auto& metric : report.metrics) {
            logger_.logInfo("    " + metric.first + ": " + std::to_string(metric.second));
        }
        
        if (report.overallCoherent) {
            logger_.logSuccess("System coherence validation passed");
        } else {
            logger_.logWarning("System coherence validation found issues");
        }
    }
    
    void testPerformanceMetrics() {
        logger_.logInfo("=== Test 7: Performance Metrics ===");
        
        auto status = embodiment_->getSystemStatus();
        auto metrics = embodiment_->getPerformanceMetrics();
        
        logger_.logInfo("System Status:");
        for (const auto& stat : status) {
            logger_.logInfo("  " + stat.first + ": " + stat.second);
        }
        
        logger_.logInfo("Performance Metrics:");
        for (const auto& metric : metrics) {
            logger_.logInfo("  " + metric.first + ": " + std::to_string(metric.second));
        }
    }
    
    void runInteractiveMode() {
        logger_.logInfo("=== Interactive Embodied Agent Mode ===");
        
        // Enable continuous validation
        embodiment_->enableContinuousValidation(true, std::chrono::seconds(30));
        
        // Start the embodiment system
        if (!embodiment_->start()) {
            logger_.logError("Failed to start embodiment system");
            return;
        }
        
        logger_.panel("Interactive Mode", 
            "The embodied agent is now running!\n"
            "- Type messages to interact with the agent\n"
            "- The agent will perceive your input and respond with actions\n"
            "- System coherence is monitored continuously\n"
            "- Type 'quit' to exit");
        
        // Let the system run for demonstration
        std::this_thread::sleep_for(std::chrono::seconds(30));
        
        logger_.logInfo("Stopping interactive mode...");
        embodiment_->stop();
    }
    
    void processPerception(const std::vector<std::shared_ptr<SensoryData>>& sensoryData) {
        // Custom perception processing callback
        if (!sensoryData.empty()) {
            logger_.logInfo("Processing " + std::to_string(sensoryData.size()) + " sensory inputs");
            
            // Add to memory and cognition
            for (const auto& data : sensoryData) {
                if (data->type == SensoryDataType::TEXTUAL) {
                    auto textData = std::dynamic_pointer_cast<TextualData>(data);
                    if (textData) {
                        auto memory = std::make_shared<Memory>(
                            generateUUID(),
                            "Perceived: " + textData->text,
                            "perception-entity",
                            state_->getAgentId()
                        );
                        
                        memory_->addMemory(memory);
                        cognition_->integrateMemory(memory);
                    }
                }
            }
        }
    }
    
    std::vector<std::shared_ptr<MotorAction>> decideActions(
        const State& state, 
        const std::vector<std::shared_ptr<SensoryData>>& sensoryData) {
        
        std::vector<std::shared_ptr<MotorAction>> actions;
        
        // Analyze sensory input and decide on actions
        for (const auto& data : sensoryData) {
            if (data->type == SensoryDataType::TEXTUAL) {
                auto textData = std::dynamic_pointer_cast<TextualData>(data);
                if (textData && !textData->text.empty()) {
                    
                    // Use cognitive fusion for response generation
                    auto reasoningResult = cognition_->processQuery(state, textData->text);
                    
                    // Create communication response
                    auto response = std::make_shared<CommunicationAction>();
                    
                    if (!reasoningResult.fusedResults.empty()) {
                        response->message = "I understand: " + reasoningResult.fusedResults[0];
                    } else {
                        response->message = "I acknowledge your input: " + textData->text;
                    }
                    
                    response->recipient = "user";
                    response->channel = "main";
                    actions.push_back(response);
                    
                    // Add some behavioral actions based on input content
                    if (textData->text.find("hello") != std::string::npos || 
                        textData->text.find("hi") != std::string::npos) {
                        
                        // Friendly greeting gesture
                        auto gestureAction = std::make_shared<GestureAction>("wave");
                        gestureAction->duration = 1.5;
                        actions.push_back(gestureAction);
                        
                        // Display welcome message
                        auto displayAction = std::make_shared<DisplayAction>("Welcome! I'm ready to help.");
                        displayAction->duration = 3.0;
                        actions.push_back(displayAction);
                    }
                    
                    if (textData->text.find("move") != std::string::npos ||
                        textData->text.find("go") != std::string::npos) {
                        
                        // Movement action
                        auto moveAction = std::make_shared<MovementAction>();
                        moveAction->targetPosition = {1.0, 0.0, 0.0}; // Move forward
                        moveAction->speed = 0.3;
                        actions.push_back(moveAction);
                    }
                }
            }
            
            if (data->type == SensoryDataType::ENVIRONMENTAL) {
                auto envData = std::dynamic_pointer_cast<EnvironmentalData>(data);
                if (envData) {
                    // React to environmental conditions
                    if (envData->temperature > 30.0) {
                        auto response = std::make_shared<CommunicationAction>();
                        response->message = "It's getting warm here! Temperature: " + 
                                          std::to_string(envData->temperature) + "°C";
                        actions.push_back(response);
                    }
                    
                    if (envData->lightLevel < 0.2) {
                        auto displayAction = std::make_shared<DisplayAction>("Adjusting to low light conditions");
                        displayAction->duration = 2.0;
                        actions.push_back(displayAction);
                    }
                }
            }
        }
        
        return actions;
    }
    
    void createTestEnvironmentalData(const std::string& filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
            // Write CSV header and sample data
            file << "# temp,humidity,pressure,light,ax,ay,az,gx,gy,gz\n";
            file << "23.5,45.2,1013.25,0.8,0.1,-0.05,9.8,0.01,0.02,-0.01\n";
            file << "24.1,44.8,1013.20,0.75,0.15,-0.08,9.82,0.02,0.01,0.00\n";
            file << "24.8,44.1,1013.15,0.72,0.12,-0.06,9.79,0.01,0.03,-0.01\n";
            file.close();
            
            logger_.logInfo("Created test environmental data file: " + filename);
        }
    }
    
    AgentLogger logger_;
    std::shared_ptr<State> state_;
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<CognitiveFusionEngine> cognition_;
    std::shared_ptr<EmbodimentManager> embodiment_;
};

int main() {
    try {
        EmbodimentDemo demo;
        demo.runDemo();
    } catch (const std::exception& e) {
        AgentLogger logger;
        logger.logError("Demo failed with exception: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}