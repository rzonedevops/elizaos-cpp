#include <gtest/gtest.h>
#include "elizaos/embodiment.hpp"
#include "elizaos/agentmemory.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace elizaos;

class EmbodimentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create agent configuration
        AgentConfig config;
        config.agentId = "test-agent";
        config.agentName = "TestAgent";
        config.bio = "Test agent for embodiment testing";
        config.lore = "Created for testing purposes";
        
        state_ = std::make_shared<State>(config);
        memory_ = std::make_shared<AgentMemoryManager>();
        cognition_ = std::make_shared<CognitiveFusionEngine>();
        
        // Initialize memory
        memory_->initialize();
    }
    
    void TearDown() override {
        // Clean up
    }
    
    std::shared_ptr<State> state_;
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<CognitiveFusionEngine> cognition_;
};

// Test sensory data structures
TEST_F(EmbodimentTest, SensoryDataCreation) {
    // Test TextualData
    auto textData = std::make_shared<TextualData>("Hello, world!");
    EXPECT_EQ(textData->type, SensoryDataType::TEXTUAL);
    EXPECT_EQ(textData->text, "Hello, world!");
    EXPECT_EQ(textData->language, "en");
    EXPECT_EQ(textData->encoding, "UTF-8");
    
    // Test VisualData
    auto visualData = std::make_shared<VisualData>();
    visualData->width = 640;
    visualData->height = 480;
    visualData->channels = 3;
    visualData->format = "RGB";
    EXPECT_EQ(visualData->type, SensoryDataType::VISUAL);
    EXPECT_EQ(visualData->width, 640);
    EXPECT_EQ(visualData->height, 480);
    
    // Test AudioData
    auto audioData = std::make_shared<AudioData>();
    audioData->sampleRate = 44100;
    audioData->channels = 2;
    audioData->durationSeconds = 5.0;
    EXPECT_EQ(audioData->type, SensoryDataType::AUDITORY);
    EXPECT_EQ(audioData->sampleRate, 44100);
    EXPECT_EQ(audioData->channels, 2);
    
    // Test EnvironmentalData
    auto envData = std::make_shared<EnvironmentalData>();
    envData->temperature = 23.5;
    envData->humidity = 45.0;
    envData->pressure = 1013.25;
    EXPECT_EQ(envData->type, SensoryDataType::ENVIRONMENTAL);
    EXPECT_DOUBLE_EQ(envData->temperature, 23.5);
    EXPECT_DOUBLE_EQ(envData->humidity, 45.0);
}

// Test motor action structures
TEST_F(EmbodimentTest, MotorActionCreation) {
    // Test MovementAction
    auto moveAction = std::make_shared<MovementAction>();
    moveAction->targetPosition = {1.0, 2.0, 3.0};
    moveAction->speed = 0.5;
    EXPECT_EQ(moveAction->type, MotorActionType::MOVEMENT);
    EXPECT_EQ(moveAction->targetPosition.size(), 3);
    EXPECT_DOUBLE_EQ(moveAction->targetPosition[0], 1.0);
    
    // Test SpeechAction
    auto speechAction = std::make_shared<SpeechAction>("Hello, I am speaking!");
    EXPECT_EQ(speechAction->type, MotorActionType::SPEECH);
    EXPECT_EQ(speechAction->text, "Hello, I am speaking!");
    EXPECT_EQ(speechAction->voice, "default");
    
    // Test CommunicationAction
    auto commAction = std::make_shared<CommunicationAction>("Test message", "user");
    EXPECT_EQ(commAction->type, MotorActionType::COMMUNICATION);
    EXPECT_EQ(commAction->message, "Test message");
    EXPECT_EQ(commAction->recipient, "user");
}

// Test ConsoleTextInterface
TEST_F(EmbodimentTest, ConsoleTextInterface) {
    auto consoleInterface = std::make_shared<ConsoleTextInterface>();
    
    // Test initialization
    EXPECT_TRUE(consoleInterface->initialize());
    EXPECT_TRUE(consoleInterface->isActive());
    EXPECT_EQ(consoleInterface->getName(), "ConsoleTextInput");
    EXPECT_EQ(consoleInterface->getType(), SensoryDataType::TEXTUAL);
    
    // Test motor capabilities
    EXPECT_EQ(consoleInterface->getType(), MotorActionType::COMMUNICATION);
    
    // Test action execution
    auto commAction = std::make_shared<CommunicationAction>("Test output", "console");
    EXPECT_TRUE(consoleInterface->canExecute(commAction));
    EXPECT_TRUE(consoleInterface->executeAction(commAction));
    
    // Test action completion
    EXPECT_TRUE(consoleInterface->isActionComplete("any-id"));
    EXPECT_EQ(consoleInterface->getActionProgress("any-id"), 1.0);
    
    // Test configuration
    std::unordered_map<std::string, std::string> config = {{"test_key", "test_value"}};
    consoleInterface->setConfiguration(config);
    auto retrievedConfig = consoleInterface->getConfiguration();
    EXPECT_EQ(retrievedConfig["test_key"], "test_value");
    
    // Test shutdown
    consoleInterface->shutdown();
    EXPECT_FALSE(consoleInterface->isActive());
}

// Test MockMotorInterface
TEST_F(EmbodimentTest, MockMotorInterface) {
    auto mockInterface = std::make_shared<MockMotorInterface>(MotorActionType::MOVEMENT);
    
    // Test initialization
    EXPECT_TRUE(mockInterface->initialize());
    EXPECT_TRUE(mockInterface->isActive());
    EXPECT_EQ(mockInterface->getType(), MotorActionType::MOVEMENT);
    
    // Test action execution
    auto moveAction = std::make_shared<MovementAction>();
    EXPECT_TRUE(mockInterface->canExecute(moveAction));
    EXPECT_TRUE(mockInterface->executeAction(moveAction));
    
    // Test executed actions tracking
    auto executedActions = mockInterface->getExecutedActions();
    EXPECT_EQ(executedActions.size(), 1);
    EXPECT_EQ(executedActions[0]->type, MotorActionType::MOVEMENT);
    
    // Test clearing actions
    mockInterface->clearExecutedActions();
    executedActions = mockInterface->getExecutedActions();
    EXPECT_EQ(executedActions.size(), 0);
    
    // Test action rejection
    auto speechAction = std::make_shared<SpeechAction>("Test");
    EXPECT_FALSE(mockInterface->canExecute(speechAction));
    EXPECT_FALSE(mockInterface->executeAction(speechAction));
    
    // Test shutdown
    mockInterface->shutdown();
    EXPECT_FALSE(mockInterface->isActive());
}

// Test PerceptionActionLoop
TEST_F(EmbodimentTest, PerceptionActionLoop) {
    auto loop = std::make_shared<PerceptionActionLoop>(state_, memory_, cognition_);
    
    // Test initialization
    EXPECT_TRUE(loop->initialize());
    
    // Add interfaces
    auto consoleInterface = std::make_shared<ConsoleTextInterface>();
    auto mockMotor = std::make_shared<MockMotorInterface>(MotorActionType::COMMUNICATION);
    
    loop->addSensoryInterface(consoleInterface);
    loop->addMotorInterface(mockMotor);
    
    // Test configuration
    loop->setLoopInterval(std::chrono::milliseconds(50));
    
    // Test single cycle processing
    loop->processSingleCycle();
    
    // Test metrics
    EXPECT_GE(loop->getCycleCount(), 1);
    EXPECT_GE(loop->getAverageLoopTime().count(), 0);
    
    // Test shutdown
    loop->shutdown();
}

// Test EmbodimentManager
TEST_F(EmbodimentTest, EmbodimentManager) {
    auto manager = std::make_shared<EmbodimentManager>();
    
    // Set components
    manager->setState(state_);
    manager->setMemory(memory_);
    manager->setCognition(cognition_);
    
    // Test initialization
    EXPECT_TRUE(manager->initialize());
    
    // Create default interfaces
    manager->createDefaultInterfaces();
    
    // Test integration tests
    EXPECT_TRUE(manager->testSensoryIntegration());
    EXPECT_TRUE(manager->testMotorIntegration());
    EXPECT_TRUE(manager->testPerceptionActionLoop());
    EXPECT_TRUE(manager->testSystemIntegration());
    
    // Test coherence validation
    auto report = manager->validateSystemCoherence();
    EXPECT_TRUE(report.overallCoherent);
    EXPECT_GE(report.metrics.size(), 0);
    
    // Test system status
    auto status = manager->getSystemStatus();
    EXPECT_GT(status.size(), 0);
    EXPECT_EQ(status["running"], "false"); // Not started yet
    
    // Test performance metrics
    auto metrics = manager->getPerformanceMetrics();
    EXPECT_GE(metrics.size(), 0);
    
    // Test shutdown
    manager->shutdown();
}

// Test system integration
TEST_F(EmbodimentTest, SystemIntegration) {
    auto manager = std::make_shared<EmbodimentManager>();
    
    // Setup complete system
    manager->setState(state_);
    manager->setMemory(memory_);
    manager->setCognition(cognition_);
    manager->createDefaultInterfaces();
    
    // Configure perception-action loop
    manager->configurePerceptionActionLoop(std::chrono::milliseconds(100));
    
    // Initialize and start
    EXPECT_TRUE(manager->initialize());
    EXPECT_TRUE(manager->start());
    EXPECT_TRUE(manager->isRunning());
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Test that system is functioning
    auto loop = manager->getPerceptionActionLoop();
    EXPECT_TRUE(loop->isRunning());
    EXPECT_GT(loop->getCycleCount(), 0);
    
    // Test coherence during operation
    auto report = manager->validateSystemCoherence();
    EXPECT_TRUE(report.overallCoherent);
    
    // Stop system
    manager->stop();
    EXPECT_FALSE(manager->isRunning());
}

// Test cognitive integration
TEST_F(EmbodimentTest, CognitiveIntegration) {
    // Test PLN inference engine
    auto plnEngine = std::make_shared<PLNInferenceEngine>();
    
    // Add some test rules
    InferenceRule rule1("test_rule1", "A", "B", TruthValue(0.8, 0.9), 1.0);
    InferenceRule rule2("test_rule2", "B", "C", TruthValue(0.7, 0.8), 1.0);
    
    plnEngine->addRule(rule1);
    plnEngine->addRule(rule2);
    
    // Test forward chaining
    auto results = plnEngine->forwardChain(*state_, "A", 3);
    EXPECT_GT(results.size(), 0);
    
    // Test best inference
    auto bestResult = plnEngine->bestInference(*state_, "A");
    EXPECT_GT(bestResult.confidence, 0.0);
    
    // Test cognitive fusion engine
    cognition_->registerPLNEngine(plnEngine);
    
    auto reasoning = cognition_->processQueryWithUncertainty(*state_, "test query");
    EXPECT_GE(reasoning.confidence, 0.0);
}

// Test performance under load
TEST_F(EmbodimentTest, PerformanceTest) {
    auto manager = std::make_shared<EmbodimentManager>();
    manager->setState(state_);
    manager->setMemory(memory_);
    manager->setCognition(cognition_);
    manager->createDefaultInterfaces();
    manager->configurePerceptionActionLoop(std::chrono::milliseconds(10)); // High frequency
    
    EXPECT_TRUE(manager->initialize());
    EXPECT_TRUE(manager->start());
    
    // Run for a short time to generate metrics
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto loop = manager->getPerceptionActionLoop();
    EXPECT_GT(loop->getCycleCount(), 5); // Should have completed several cycles
    EXPECT_LT(loop->getAverageLoopTime().count(), 100); // Should be reasonably fast
    
    manager->stop();
}

// Test error handling
TEST_F(EmbodimentTest, ErrorHandling) {
    // Test null pointer handling
    EXPECT_THROW(PerceptionActionLoop(nullptr, memory_, cognition_), std::invalid_argument);
    EXPECT_THROW(PerceptionActionLoop(state_, nullptr, cognition_), std::invalid_argument);
    
    // Test manager without required components
    auto manager = std::make_shared<EmbodimentManager>();
    EXPECT_FALSE(manager->initialize()); // Should fail without state and memory
    
    // Test interface with invalid actions
    auto mockInterface = std::make_shared<MockMotorInterface>(MotorActionType::SPEECH);
    mockInterface->initialize();
    
    auto moveAction = std::make_shared<MovementAction>();
    EXPECT_FALSE(mockInterface->canExecute(moveAction)); // Wrong type
    EXPECT_FALSE(mockInterface->executeAction(moveAction));
}