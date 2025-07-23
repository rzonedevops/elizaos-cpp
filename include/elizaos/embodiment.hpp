#pragma once

#include "elizaos/core.hpp"
#include "elizaos/agentloop.hpp"
#include "elizaos/agentaction.hpp"
#include "elizaos/agentmemory.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>

namespace elizaos {

/**
 * Stage 6 - Embodiment & Integration
 * 
 * This module provides interfaces and implementations for connecting
 * the cognitive core to virtual or physical agents through:
 * 1. Sensory data interfaces (multi-modal input)
 * 2. Motor control interfaces (action output)
 * 3. Perception-action loop integration
 * 4. System-level coherence validation
 */

// Forward declarations
class SensoryInterface;
class MotorInterface;
class PerceptionActionLoop;
class EmbodimentManager;

/**
 * Sensory data types and structures
 */
enum class SensoryDataType {
    VISUAL,      // Camera/image data
    AUDITORY,    // Microphone/sound data  
    TEXTUAL,     // Text input/messages
    HAPTIC,      // Touch/force feedback
    TEMPORAL,    // Time-based events
    ENVIRONMENTAL, // Environmental sensors
    CUSTOM       // Custom sensor types
};

struct SensoryData {
    SensoryDataType type;
    std::chrono::system_clock::time_point timestamp;
    std::vector<uint8_t> rawData;
    std::unordered_map<std::string, std::string> metadata;
    double confidence = 1.0;
    std::string source;
    
    SensoryData(SensoryDataType t, const std::string& src = "") 
        : type(t), timestamp(std::chrono::system_clock::now()), source(src) {}
    
    virtual ~SensoryData() = default; // Make polymorphic
};

// Specialized sensory data types
struct VisualData : public SensoryData {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string format = "RGB";
    
    VisualData() : SensoryData(SensoryDataType::VISUAL) {}
};

struct AudioData : public SensoryData {
    int sampleRate = 44100;
    int channels = 1;
    double durationSeconds = 0.0;
    std::string encoding = "PCM";
    
    AudioData() : SensoryData(SensoryDataType::AUDITORY) {}
};

struct TextualData : public SensoryData {
    std::string text;
    std::string language = "en";
    std::string encoding = "UTF-8";
    
    TextualData(const std::string& content = "") 
        : SensoryData(SensoryDataType::TEXTUAL), text(content) {}
};

struct HapticData : public SensoryData {
    double force = 0.0;
    double pressure = 0.0;
    std::vector<double> position = {0.0, 0.0, 0.0}; // x, y, z
    std::vector<double> orientation = {0.0, 0.0, 0.0}; // roll, pitch, yaw
    
    HapticData() : SensoryData(SensoryDataType::HAPTIC) {}
};

struct EnvironmentalData : public SensoryData {
    double temperature = 0.0;
    double humidity = 0.0;
    double pressure = 0.0;
    double lightLevel = 0.0;
    std::vector<double> acceleration = {0.0, 0.0, 0.0};
    std::vector<double> gyroscope = {0.0, 0.0, 0.0};
    
    EnvironmentalData() : SensoryData(SensoryDataType::ENVIRONMENTAL) {}
};

/**
 * Motor action types and structures
 */
enum class MotorActionType {
    MOVEMENT,    // Physical movement
    SPEECH,      // Speech synthesis
    DISPLAY,     // Visual display/output
    GESTURE,     // Gestural actions
    MANIPULATION, // Object manipulation
    COMMUNICATION, // Network communication
    CUSTOM       // Custom motor actions
};

struct MotorAction {
    MotorActionType type;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> parameters;
    std::vector<uint8_t> actionData;
    double priority = 1.0;
    std::string target;
    bool blocking = false; // Whether to wait for completion
    
    MotorAction(MotorActionType t, const std::string& tgt = "") 
        : type(t), timestamp(std::chrono::system_clock::now()), target(tgt) {}
    
    virtual ~MotorAction() = default; // Make polymorphic
};

// Specialized motor action types
struct MovementAction : public MotorAction {
    std::vector<double> targetPosition = {0.0, 0.0, 0.0}; // x, y, z
    std::vector<double> targetOrientation = {0.0, 0.0, 0.0}; // roll, pitch, yaw
    double speed = 1.0;
    double acceleration = 1.0;
    std::string movementType = "linear"; // linear, angular, trajectory
    
    MovementAction() : MotorAction(MotorActionType::MOVEMENT) {}
};

struct SpeechAction : public MotorAction {
    std::string text;
    std::string voice = "default";
    double volume = 1.0;
    double pitch = 1.0;
    double speed = 1.0;
    std::string language = "en";
    
    SpeechAction(const std::string& content = "") 
        : MotorAction(MotorActionType::SPEECH), text(content) {}
};

struct DisplayAction : public MotorAction {
    std::string content;
    std::string contentType = "text"; // text, image, video, html
    std::vector<double> position = {0.0, 0.0}; // x, y screen coordinates
    std::vector<double> size = {100.0, 100.0}; // width, height
    double duration = -1.0; // -1 = permanent, >0 = seconds
    
    DisplayAction(const std::string& content = "") 
        : MotorAction(MotorActionType::DISPLAY), content(content) {}
};

struct GestureAction : public MotorAction {
    std::string gestureName;
    std::vector<std::vector<double>> keyframes; // sequence of positions
    double duration = 1.0;
    bool loop = false;
    
    GestureAction(const std::string& name = "") 
        : MotorAction(MotorActionType::GESTURE), gestureName(name) {}
};

struct ManipulationAction : public MotorAction {
    std::string objectId;
    std::string actionType = "grasp"; // grasp, release, move, rotate
    std::vector<double> targetPose = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // x,y,z,roll,pitch,yaw
    double force = 1.0;
    
    ManipulationAction(const std::string& objId = "") 
        : MotorAction(MotorActionType::MANIPULATION), objectId(objId) {}
};

struct CommunicationAction : public MotorAction {
    std::string message;
    std::string recipient;
    std::string channel = "default";
    std::string messageType = "text"; // text, json, binary
    
    CommunicationAction(const std::string& msg = "", const std::string& rcpt = "") 
        : MotorAction(MotorActionType::COMMUNICATION), message(msg), recipient(rcpt) {}
};

/**
 * Sensory interface for input processing
 */
class SensoryInterface {
public:
    virtual ~SensoryInterface() = default;
    
    virtual std::string getName() const = 0;
    virtual SensoryDataType getType() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isActive() const = 0;
    
    // Data acquisition
    virtual std::shared_ptr<SensoryData> readData() = 0;
    virtual std::vector<std::shared_ptr<SensoryData>> readDataBuffer(size_t maxItems = 10) = 0;
    virtual bool hasData() const = 0;
    
    // Configuration
    virtual void setConfiguration(const std::unordered_map<std::string, std::string>& config) = 0;
    virtual std::unordered_map<std::string, std::string> getConfiguration() const = 0;
    
    // Callbacks for real-time processing
    virtual void setDataCallback(std::function<void(std::shared_ptr<SensoryData>)> callback) = 0;
    virtual void enableRealTimeProcessing(bool enable) = 0;
};

/**
 * Motor interface for output control
 */
class MotorInterface {
public:
    virtual ~MotorInterface() = default;
    
    virtual std::string getName() const = 0;
    virtual MotorActionType getType() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isActive() const = 0;
    
    // Action execution
    virtual bool executeAction(std::shared_ptr<MotorAction> action) = 0;
    virtual bool canExecute(std::shared_ptr<MotorAction> action) const = 0;
    virtual void stopAction(const std::string& actionId = "") = 0;
    virtual void stopAllActions() = 0;
    
    // Status and feedback
    virtual bool isActionComplete(const std::string& actionId) const = 0;
    virtual std::vector<std::string> getActiveActions() const = 0;
    virtual double getActionProgress(const std::string& actionId) const = 0;
    
    // Configuration
    virtual void setConfiguration(const std::unordered_map<std::string, std::string>& config) = 0;
    virtual std::unordered_map<std::string, std::string> getConfiguration() const = 0;
};

/**
 * Perception-Action Loop - Core embodiment processing cycle
 */
class PerceptionActionLoop {
public:
    PerceptionActionLoop(std::shared_ptr<State> state, 
                        std::shared_ptr<AgentMemoryManager> memory,
                        std::shared_ptr<CognitiveFusionEngine> cognition = nullptr);
    ~PerceptionActionLoop();
    
    // Lifecycle management
    bool initialize();
    void shutdown();
    bool start();
    void stop();
    void pause();
    void resume();
    
    // Interface management
    void addSensoryInterface(std::shared_ptr<SensoryInterface> interface);
    void addMotorInterface(std::shared_ptr<MotorInterface> interface);
    void removeSensoryInterface(const std::string& name);
    void removeMotorInterface(const std::string& name);
    
    // Loop configuration
    void setLoopInterval(std::chrono::milliseconds interval) { loopInterval_ = interval; }
    void setPerceptionProcessingCallback(std::function<void(std::vector<std::shared_ptr<SensoryData>>)> callback);
    void setActionDecisionCallback(std::function<std::vector<std::shared_ptr<MotorAction>>(
        const State&, const std::vector<std::shared_ptr<SensoryData>>&)> callback);
    
    // Status and metrics
    bool isRunning() const { return running_; }
    bool isPaused() const { return paused_; }
    size_t getCycleCount() const { return cycleCount_; }
    std::chrono::milliseconds getAverageLoopTime() const;
    double getPerceptionLatency() const { return perceptionLatency_; }
    double getActionLatency() const { return actionLatency_; }
    
    // Manual processing (for testing/debugging)
    void processSingleCycle();
    std::vector<std::shared_ptr<SensoryData>> gatherSensoryData();
    std::vector<std::shared_ptr<MotorAction>> processPerception(
        const std::vector<std::shared_ptr<SensoryData>>& sensoryData);
    void executeActions(const std::vector<std::shared_ptr<MotorAction>>& actions);
    
private:
    void mainLoop();
    void updateState(const std::vector<std::shared_ptr<SensoryData>>& sensoryData);
    void logCycleMetrics();
    
    std::shared_ptr<State> state_;
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<CognitiveFusionEngine> cognition_;
    
    std::unordered_map<std::string, std::shared_ptr<SensoryInterface>> sensoryInterfaces_;
    std::unordered_map<std::string, std::shared_ptr<MotorInterface>> motorInterfaces_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::unique_ptr<std::thread> loopThread_;
    std::chrono::milliseconds loopInterval_{100}; // 10 Hz default
    
    // Callbacks
    std::function<void(std::vector<std::shared_ptr<SensoryData>>)> perceptionCallback_;
    std::function<std::vector<std::shared_ptr<MotorAction>>(
        const State&, const std::vector<std::shared_ptr<SensoryData>>&)> actionDecisionCallback_;
    
    // Metrics
    std::atomic<size_t> cycleCount_{0};
    std::vector<std::chrono::milliseconds> loopTimes_;
    std::atomic<double> perceptionLatency_{0.0};
    std::atomic<double> actionLatency_{0.0};
    
    mutable std::mutex interfacesMutex_;
    mutable std::mutex metricsMutex_;
};

/**
 * Embodiment Manager - High-level coordination of embodiment system
 */
class EmbodimentManager {
public:
    EmbodimentManager();
    ~EmbodimentManager();
    
    // System management
    bool initialize();
    void shutdown();
    bool start();
    void stop();
    
    // Component management
    void setAgentLoop(std::shared_ptr<AgentLoop> agentLoop) { agentLoop_ = agentLoop; }
    void setState(std::shared_ptr<State> state) { state_ = state; }
    void setMemory(std::shared_ptr<AgentMemoryManager> memory) { memory_ = memory; }
    void setCognition(std::shared_ptr<CognitiveFusionEngine> cognition) { cognition_ = cognition; }
    
    // Perception-Action Loop
    std::shared_ptr<PerceptionActionLoop> getPerceptionActionLoop() const { return perceptionActionLoop_; }
    void configurePerceptionActionLoop(std::chrono::milliseconds interval = std::chrono::milliseconds(100));
    
    // Interface factories and registration
    void registerSensoryInterface(std::shared_ptr<SensoryInterface> interface);
    void registerMotorInterface(std::shared_ptr<MotorInterface> interface);
    void createDefaultInterfaces(); // Create standard interfaces (console, file, etc.)
    
    // System coherence validation
    struct CoherenceReport {
        bool overallCoherent = false;
        std::vector<std::string> issues;
        std::vector<std::string> warnings;
        std::unordered_map<std::string, double> metrics;
        std::chrono::system_clock::time_point timestamp;
    };
    
    CoherenceReport validateSystemCoherence();
    void enableContinuousValidation(bool enable, std::chrono::seconds interval = std::chrono::seconds(10));
    
    // Integration testing
    bool testSensoryIntegration();
    bool testMotorIntegration();
    bool testPerceptionActionLoop();
    bool testSystemIntegration();
    
    // Status and diagnostics
    bool isRunning() const { return running_; }
    std::unordered_map<std::string, std::string> getSystemStatus() const;
    std::unordered_map<std::string, double> getPerformanceMetrics() const;
    
private:
    void coherenceValidationLoop();
    void updateSystemMetrics();
    
    std::shared_ptr<AgentLoop> agentLoop_;
    std::shared_ptr<State> state_;
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<CognitiveFusionEngine> cognition_;
    std::shared_ptr<PerceptionActionLoop> perceptionActionLoop_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> continuousValidation_{false};
    std::unique_ptr<std::thread> validationThread_;
    std::chrono::seconds validationInterval_{10};
    
    mutable std::mutex systemMutex_;
    
    // System metrics
    CoherenceReport lastCoherenceReport_;
    std::unordered_map<std::string, double> performanceMetrics_;
};

// Default interface implementations for common use cases

/**
 * Console Text Interface - Basic text input/output via console
 */
class ConsoleTextInterface : public SensoryInterface, public MotorInterface {
public:
    ConsoleTextInterface();
    virtual ~ConsoleTextInterface() = default;
    
    // SensoryInterface implementation
    std::string getName() const override { return "ConsoleTextInput"; }
    SensoryDataType getType() const override { return SensoryDataType::TEXTUAL; }
    bool initialize() override;
    void shutdown() override;
    bool isActive() const override { return active_; }
    
    std::shared_ptr<SensoryData> readData() override;
    std::vector<std::shared_ptr<SensoryData>> readDataBuffer(size_t maxItems = 10) override;
    bool hasData() const override;
    
    void setConfiguration(const std::unordered_map<std::string, std::string>& config) override;
    std::unordered_map<std::string, std::string> getConfiguration() const override;
    void setDataCallback(std::function<void(std::shared_ptr<SensoryData>)> callback) override;
    void enableRealTimeProcessing(bool enable) override;
    
    // MotorInterface implementation  
    MotorActionType getType() const { return MotorActionType::COMMUNICATION; }
    bool executeAction(std::shared_ptr<MotorAction> action) override;
    bool canExecute(std::shared_ptr<MotorAction> action) const override;
    void stopAction(const std::string& actionId = "") override;
    void stopAllActions() override;
    
    bool isActionComplete(const std::string& actionId) const override;
    std::vector<std::string> getActiveActions() const override;
    double getActionProgress(const std::string& actionId) const override;
    
private:
    void inputThread();
    
    std::atomic<bool> active_{false};
    std::unique_ptr<std::thread> inputThread_;
    std::vector<std::string> inputBuffer_;
    std::unordered_map<std::string, std::string> config_;
    std::function<void(std::shared_ptr<SensoryData>)> dataCallback_;
    bool realTimeProcessing_ = false;
    
    mutable std::mutex bufferMutex_;
    mutable std::mutex configMutex_;
};

/**
 * File-based Sensory Interface - Read sensory data from files
 */
class FileSensoryInterface : public SensoryInterface {
public:
    FileSensoryInterface(SensoryDataType type, const std::string& filePath);
    virtual ~FileSensoryInterface() = default;
    
    std::string getName() const override { return "FileSensory_" + filePath_; }
    SensoryDataType getType() const override { return type_; }
    bool initialize() override;
    void shutdown() override;
    bool isActive() const override { return active_; }
    
    std::shared_ptr<SensoryData> readData() override;
    std::vector<std::shared_ptr<SensoryData>> readDataBuffer(size_t maxItems = 10) override;
    bool hasData() const override;
    
    void setConfiguration(const std::unordered_map<std::string, std::string>& config) override;
    std::unordered_map<std::string, std::string> getConfiguration() const override;
    void setDataCallback(std::function<void(std::shared_ptr<SensoryData>)> callback) override;
    void enableRealTimeProcessing(bool enable) override;
    
private:
    SensoryDataType type_;
    std::string filePath_;
    std::atomic<bool> active_{false};
    std::ifstream fileStream_;
    std::unordered_map<std::string, std::string> config_;
    std::function<void(std::shared_ptr<SensoryData>)> dataCallback_;
    bool realTimeProcessing_ = false;
    
    mutable std::mutex configMutex_;
};

/**
 * Mock Motor Interface - For testing and development
 */
class MockMotorInterface : public MotorInterface {
public:
    MockMotorInterface(MotorActionType type);
    virtual ~MockMotorInterface() = default;
    
    std::string getName() const override { return "MockMotor_" + std::to_string((int)type_); }
    MotorActionType getType() const override { return type_; }
    bool initialize() override;
    void shutdown() override;
    bool isActive() const override { return active_; }
    
    bool executeAction(std::shared_ptr<MotorAction> action) override;
    bool canExecute(std::shared_ptr<MotorAction> action) const override;
    void stopAction(const std::string& actionId = "") override;
    void stopAllActions() override;
    
    bool isActionComplete(const std::string& actionId) const override;
    std::vector<std::string> getActiveActions() const override;
    double getActionProgress(const std::string& actionId) const override;
    
    void setConfiguration(const std::unordered_map<std::string, std::string>& config) override;
    std::unordered_map<std::string, std::string> getConfiguration() const override;
    
    // Mock-specific methods
    std::vector<std::shared_ptr<MotorAction>> getExecutedActions() const;
    void clearExecutedActions();
    
private:
    MotorActionType type_;
    std::atomic<bool> active_{false};
    std::unordered_map<std::string, std::string> config_;
    std::vector<std::shared_ptr<MotorAction>> executedActions_;
    std::vector<std::string> activeActions_;
    
    mutable std::mutex actionsMutex_;
    mutable std::mutex configMutex_;
};

} // namespace elizaos