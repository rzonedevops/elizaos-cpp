#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <optional>
#include <variant>
#include <mutex>

namespace elizaos {

// Forward declarations
class Agent;
class Memory;
class Action;
class State;
class Task;
class TaskManager;
class HypergraphNode;
class HypergraphEdge;

// Basic types
using UUID = std::string;
using Timestamp = std::chrono::system_clock::time_point;
using EmbeddingVector = std::vector<float>;

// Memory system enums and types
enum class MemoryType {
    DOCUMENT,
    FRAGMENT, 
    MESSAGE,
    DESCRIPTION,
    CUSTOM
};

enum class MemoryScope {
    SHARED,
    PRIVATE,
    ROOM
};

/**
 * Memory metadata structures for enhanced memory representation
 */
struct BaseMetadata {
    MemoryType type;
    std::optional<std::string> source;
    std::optional<UUID> sourceId;
    std::optional<MemoryScope> scope;
    std::optional<Timestamp> timestamp;
    std::vector<std::string> tags;
};

struct DocumentMetadata : BaseMetadata {
    DocumentMetadata() { type = MemoryType::DOCUMENT; }
};

struct FragmentMetadata : BaseMetadata {
    UUID documentId;
    size_t position;
    FragmentMetadata() { type = MemoryType::FRAGMENT; }
};

struct MessageMetadata : BaseMetadata {
    MessageMetadata() { type = MemoryType::MESSAGE; }
};

struct DescriptionMetadata : BaseMetadata {
    DescriptionMetadata() { type = MemoryType::DESCRIPTION; }
};

struct CustomMetadata : BaseMetadata {
    std::unordered_map<std::string, std::string> customData;
    CustomMetadata() { type = MemoryType::CUSTOM; }
};

using MemoryMetadata = std::variant<DocumentMetadata, FragmentMetadata, MessageMetadata, DescriptionMetadata, CustomMetadata>;

/**
 * Hypergraph structures for knowledge representation
 */
class HypergraphNode {
public:
    HypergraphNode(const UUID& id, const std::string& label);
    
    const UUID& getId() const { return id_; }
    const std::string& getLabel() const { return label_; }
    const std::unordered_map<std::string, std::string>& getAttributes() const { return attributes_; }
    
    void setAttribute(const std::string& key, const std::string& value);
    std::optional<std::string> getAttribute(const std::string& key) const;
    
private:
    UUID id_;
    std::string label_;
    std::unordered_map<std::string, std::string> attributes_;
};

class HypergraphEdge {
public:
    HypergraphEdge(const UUID& id, const std::string& label, const std::vector<UUID>& nodeIds);
    
    const UUID& getId() const { return id_; }
    const std::string& getLabel() const { return label_; }
    const std::vector<UUID>& getNodeIds() const { return nodeIds_; }
    double getWeight() const { return weight_; }
    
    void setWeight(double weight) { weight_ = weight; }
    
private:
    UUID id_;
    std::string label_;
    std::vector<UUID> nodeIds_;
    double weight_ = 1.0;
};

/**
 * Core data structures representing agent entities and state
 * Based on the TypeScript State interface from eliza/packages/core
 */
struct AgentConfig {
    UUID agentId;
    std::string agentName;
    std::string bio;
    std::string lore;
    std::string adjective;
};

struct Actor {
    UUID id;
    std::string name;
    std::string details;
};

struct Goal {
    UUID id;
    std::string description;
    std::string status;
    Timestamp createdAt;
    Timestamp updatedAt;
};

/**
 * Enhanced Memory class with embedding vectors and metadata support
 * Supports hypergraph encoding and rich metadata
 */
class Memory {
public:
    Memory(const UUID& id, const std::string& content, const UUID& entityId, const UUID& agentId);
    Memory(const UUID& id, const std::string& content, const UUID& entityId, const UUID& agentId, 
           const MemoryMetadata& metadata);
    
    // Basic accessors
    const UUID& getId() const { return id_; }
    const std::string& getContent() const { return content_; }
    const UUID& getEntityId() const { return entityId_; }
    const UUID& getAgentId() const { return agentId_; }
    const UUID& getRoomId() const { return roomId_; }
    void setRoomId(const UUID& roomId) { roomId_ = roomId; }
    Timestamp getCreatedAt() const { return createdAt_; }
    
    // Enhanced features
    const std::optional<EmbeddingVector>& getEmbedding() const { return embedding_; }
    void setEmbedding(const EmbeddingVector& embedding) { embedding_ = embedding; }
    
    const MemoryMetadata& getMetadata() const { return metadata_; }
    void setMetadata(const MemoryMetadata& metadata) { metadata_ = metadata; }
    
    bool isUnique() const { return unique_; }
    void setUnique(bool unique) { unique_ = unique; }
    
    double getSimilarity() const { return similarity_; }
    void setSimilarity(double similarity) { similarity_ = similarity; }
    
    // Hypergraph connections
    void addHypergraphNode(const UUID& nodeId) { hypergraphNodes_.push_back(nodeId); }
    void addHypergraphEdge(const UUID& edgeId) { hypergraphEdges_.push_back(edgeId); }
    const std::vector<UUID>& getHypergraphNodes() const { return hypergraphNodes_; }
    const std::vector<UUID>& getHypergraphEdges() const { return hypergraphEdges_; }
    
private:
    UUID id_;
    std::string content_;
    UUID entityId_;
    UUID agentId_;
    UUID roomId_;
    Timestamp createdAt_;
    
    // Enhanced features
    std::optional<EmbeddingVector> embedding_;
    MemoryMetadata metadata_;
    bool unique_ = false;
    double similarity_ = 0.0;
    
    // Hypergraph connections
    std::vector<UUID> hypergraphNodes_;
    std::vector<UUID> hypergraphEdges_;
};

/**
 * Task orchestration primitives for agent workflow management
 */
enum class TaskStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    CANCELLED
};

struct TaskOptions {
    std::unordered_map<std::string, std::string> data;
};

class Task {
public:
    Task(const UUID& id, const std::string& name, const std::string& description);
    
    const UUID& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const UUID& getRoomId() const { return roomId_; }
    const UUID& getWorldId() const { return worldId_; }
    
    TaskStatus getStatus() const { return status_; }
    void setStatus(TaskStatus status) { status_ = status; }
    
    const std::vector<std::string>& getTags() const { return tags_; }
    void addTag(const std::string& tag) { tags_.push_back(tag); }
    
    const TaskOptions& getOptions() const { return options_; }
    void setOptions(const TaskOptions& options) { options_ = options; }
    
    Timestamp getCreatedAt() const { return createdAt_; }
    Timestamp getUpdatedAt() const { return updatedAt_; }
    void updateTimestamp() { updatedAt_ = std::chrono::system_clock::now(); }
    
    // Task scheduling properties
    std::optional<Timestamp> getScheduledTime() const { return scheduledTime_; }
    void setScheduledTime(const Timestamp& time) { scheduledTime_ = time; }
    
    int getPriority() const { return priority_; }
    void setPriority(int priority) { priority_ = priority; }
    
private:
    UUID id_;
    std::string name_;
    std::string description_;
    UUID roomId_;
    UUID worldId_;
    TaskStatus status_ = TaskStatus::PENDING;
    std::vector<std::string> tags_;
    TaskOptions options_;
    Timestamp createdAt_;
    Timestamp updatedAt_;
    std::optional<Timestamp> scheduledTime_;
    int priority_ = 0;
};

/**
 * Task execution interface
 */
class TaskWorker {
public:
    virtual ~TaskWorker() = default;
    virtual std::string getName() const = 0;
    virtual bool validate(const Task& task, const State& state, std::shared_ptr<Memory> message) const = 0;
    virtual bool execute(Task& task, State& state, const TaskOptions& options) = 0;
};

/**
 * Task orchestration manager
 */
class TaskManager {
public:
    TaskManager();
    ~TaskManager();
    
    // Task management
    UUID createTask(const std::string& name, const std::string& description, 
                   const UUID& roomId = "", const UUID& worldId = "");
    bool scheduleTask(const UUID& taskId, const Timestamp& scheduledTime);
    bool cancelTask(const UUID& taskId);
    std::shared_ptr<Task> getTask(const UUID& taskId);
    std::vector<std::shared_ptr<Task>> getPendingTasks();
    std::vector<std::shared_ptr<Task>> getTasksByTag(const std::string& tag);
    
    // Worker management
    void registerWorker(std::shared_ptr<TaskWorker> worker);
    void unregisterWorker(const std::string& workerName);
    
    // Execution control
    void start();
    void stop();
    void pause();
    void resume();
    bool isRunning() const { return running_; }
    
    // Configuration
    void setTickInterval(std::chrono::milliseconds interval) { tickInterval_ = interval; }
    
private:
    void executionLoop();
    void processPendingTasks();
    bool executeTask(std::shared_ptr<Task> task);
    
    std::unordered_map<UUID, std::shared_ptr<Task>> tasks_;
    std::unordered_map<std::string, std::shared_ptr<TaskWorker>> workers_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::thread executionThread_;
    std::chrono::milliseconds tickInterval_{1000}; // 1 second default
    
    mutable std::mutex tasksMutex_;
    mutable std::mutex workersMutex_;
};
/**
 * State represents the complete context for agent decision making
 * Based on the State interface from the TypeScript implementation
 */
class State {
public:
    State(const AgentConfig& config);
    
    // Agent identity
    const UUID& getAgentId() const { return config_.agentId; }
    const std::string& getAgentName() const { return config_.agentName; }
    const std::string& getBio() const { return config_.bio; }
    const std::string& getLore() const { return config_.lore; }
    
    // Context management
    void addActor(const Actor& actor);
    void addGoal(const Goal& goal);
    void addRecentMessage(std::shared_ptr<Memory> memory);
    
    const std::vector<Actor>& getActors() const { return actors_; }
    const std::vector<Goal>& getGoals() const { return goals_; }
    const std::vector<std::shared_ptr<Memory>>& getRecentMessages() const { return recentMessages_; }
    
private:
    AgentConfig config_;
    std::vector<Actor> actors_;
    std::vector<Goal> goals_;
    std::vector<std::shared_ptr<Memory>> recentMessages_;
};

/**
 * Symbolic/Connectionist Fusion Framework
 * Interfaces for hybrid reasoning combining symbolic and neural approaches
 */

// Truth value representation for probabilistic reasoning
struct TruthValue {
    double strength;     // Probability or degree of belief [0,1]
    double confidence;   // Confidence in the measurement [0,1]
    
    TruthValue(double s = 0.0, double c = 0.0) : strength(s), confidence(c) {}
    
    // PLN-style truth value operations
    TruthValue conjunction(const TruthValue& other) const;
    TruthValue disjunction(const TruthValue& other) const;
    TruthValue negation() const;
    TruthValue implication(const TruthValue& other) const;
    
    // Utility functions
    double getExpectedValue() const { return strength * confidence; }
    bool isValid() const { return strength >= 0.0 && strength <= 1.0 && confidence >= 0.0 && confidence <= 1.0; }
};

// Inference rule representation
struct InferenceRule {
    std::string name;
    std::string pattern;           // Pattern to match (e.g., "A -> B")
    std::string conclusion;        // Conclusion pattern (e.g., "B")
    TruthValue truth;             // Truth value of the rule
    double weight;                // Weight/importance of the rule
    
    InferenceRule(const std::string& n, const std::string& p, const std::string& c, 
                  const TruthValue& t = TruthValue(1.0, 1.0), double w = 1.0)
        : name(n), pattern(p), conclusion(c), truth(t), weight(w) {}
};

// Inference result with uncertainty
struct InferenceResult {
    std::string conclusion;
    TruthValue truth;
    std::vector<std::string> reasoningChain;  // Steps taken to reach conclusion
    double confidence;
    
    InferenceResult(const std::string& c = "", const TruthValue& t = TruthValue(),
                   double conf = 0.0)
        : conclusion(c), truth(t), confidence(conf) {}
};

// Symbolic reasoning interface
class SymbolicReasoner {
public:
    virtual ~SymbolicReasoner() = default;
    virtual std::string getName() const = 0;
    virtual std::vector<std::string> reason(const State& state, const std::string& query) = 0;
    virtual bool validateRule(const std::string& rule) const = 0;
    virtual void addRule(const std::string& rule) = 0;
    
    // Extended PLN-style reasoning interface
    virtual std::vector<InferenceResult> reasonWithUncertainty(const State& state, const std::string& query) = 0;
    virtual void addInferenceRule(const InferenceRule& rule) = 0;
    virtual std::vector<InferenceRule> getApplicableRules(const std::string& query) const = 0;
    virtual TruthValue evaluateQuery(const State& state, const std::string& query) = 0;
};

// Connectionist (neural) processing interface  
class ConnectionistProcessor {
public:
    virtual ~ConnectionistProcessor() = default;
    virtual std::string getName() const = 0;
    virtual EmbeddingVector generateEmbedding(const std::string& input) = 0;
    virtual double computeSimilarity(const EmbeddingVector& a, const EmbeddingVector& b) = 0;
    virtual std::vector<std::string> generateResponse(const EmbeddingVector& context) = 0;
};

// Variable binding for pattern matching
struct VariableBinding {
    std::string variable;
    std::string value;
    TruthValue confidence;
    
    VariableBinding(const std::string& var, const std::string& val, const TruthValue& conf = TruthValue(1.0, 1.0))
        : variable(var), value(val), confidence(conf) {}
};

// Pattern matching result with bindings
struct PatternMatch {
    bool isMatch;
    std::vector<VariableBinding> bindings;
    double confidence;
    std::string matchedPattern;
    
    PatternMatch(bool match = false, double conf = 0.0, const std::string& pattern = "")
        : isMatch(match), confidence(conf), matchedPattern(pattern) {}
};

// AtomSpace traversal pattern
struct AtomSpacePattern {
    std::string type;                           // Node or edge type to match
    std::vector<std::string> variables;         // Variables to bind
    std::vector<AtomSpacePattern> subpatterns;  // Nested patterns
    std::string constraint;                     // Additional constraints
    
    AtomSpacePattern(const std::string& t = "", const std::vector<std::string>& vars = {})
        : type(t), variables(vars) {}
};

// Pattern matching interface
class PatternMatcher {
public:
    virtual ~PatternMatcher() = default;
    virtual std::string getName() const = 0;
    virtual double matchPattern(const std::string& input, const std::string& pattern) = 0;
    virtual std::vector<std::string> extractPatterns(const std::string& input) = 0;
    
    // Extended AtomSpace pattern matching
    virtual PatternMatch matchAtomSpacePattern(const AtomSpacePattern& pattern, 
                                               const std::vector<std::shared_ptr<HypergraphNode>>& nodes,
                                               const std::vector<std::shared_ptr<HypergraphEdge>>& edges) = 0;
    virtual std::vector<PatternMatch> findAllMatches(const AtomSpacePattern& pattern,
                                                     const std::vector<std::shared_ptr<HypergraphNode>>& nodes,
                                                     const std::vector<std::shared_ptr<HypergraphEdge>>& edges) = 0;
    virtual std::vector<std::shared_ptr<HypergraphNode>> traverseAtomSpace(const AtomSpacePattern& pattern,
                                                                           const std::shared_ptr<HypergraphNode>& startNode) = 0;
};

// PLN-like inference engine for probabilistic reasoning
class PLNInferenceEngine {
public:
    PLNInferenceEngine();
    ~PLNInferenceEngine();
    
    // Rule management
    void addRule(const InferenceRule& rule);
    void removeRule(const std::string& ruleName);
    std::vector<InferenceRule> getApplicableRules(const std::string& query) const;
    
    // Inference operations
    std::vector<InferenceResult> forwardChain(const State& state, const std::string& query, int maxDepth = 5);
    std::vector<InferenceResult> backwardChain(const State& state, const std::string& goal, int maxDepth = 5);
    InferenceResult bestInference(const State& state, const std::string& query);
    
    // Truth value operations
    TruthValue combineTruthValues(const TruthValue& tv1, const TruthValue& tv2, const std::string& operation);
    TruthValue propagateConfidence(const TruthValue& premise, const TruthValue& rule);
    
    // AtomSpace integration
    void setAtomSpace(const std::vector<std::shared_ptr<HypergraphNode>>& nodes,
                     const std::vector<std::shared_ptr<HypergraphEdge>>& edges);
    std::vector<std::shared_ptr<HypergraphNode>> queryAtomSpace(const std::string& query);
    
private:
    std::vector<InferenceRule> rules_;
    std::vector<std::shared_ptr<HypergraphNode>> atomSpaceNodes_;
    std::vector<std::shared_ptr<HypergraphEdge>> atomSpaceEdges_;
    
    // Internal inference helpers
    bool unify(const std::string& pattern, const std::string& target, std::vector<VariableBinding>& bindings);
    std::string substituteVariables(const std::string& pattern, const std::vector<VariableBinding>& bindings);
    TruthValue evaluatePattern(const std::string& pattern, const State& state);
    
    mutable std::mutex rulesMutex_;
    mutable std::mutex atomSpaceMutex_;
};

// Fusion engine that combines symbolic and connectionist approaches
class CognitiveFusionEngine {
public:
    CognitiveFusionEngine();
    ~CognitiveFusionEngine();
    
    // Component registration
    void registerSymbolicReasoner(std::shared_ptr<SymbolicReasoner> reasoner);
    void registerConnectionistProcessor(std::shared_ptr<ConnectionistProcessor> processor);
    void registerPatternMatcher(std::shared_ptr<PatternMatcher> matcher);
    
    // PLN integration
    void registerPLNEngine(std::shared_ptr<PLNInferenceEngine> engine);
    std::shared_ptr<PLNInferenceEngine> getPLNEngine() const { return plnEngine_; }
    
    // Hybrid reasoning
    struct ReasoningResult {
        std::vector<std::string> symbolicResults;
        std::vector<std::string> connectionistResults;
        std::vector<std::string> fusedResults;
        double confidence;
        
        // Extended with PLN results
        std::vector<InferenceResult> plnResults;
        std::vector<PatternMatch> patternMatches;
        TruthValue overallTruth;
    };
    
    ReasoningResult processQuery(const State& state, const std::string& query);
    ReasoningResult processQueryWithUncertainty(const State& state, const std::string& query);
    
    // Memory integration
    void integrateMemory(std::shared_ptr<Memory> memory);
    std::vector<std::shared_ptr<Memory>> retrieveRelevantMemories(const std::string& query, size_t maxResults = 10);
    
    // AtomSpace operations
    void buildAtomSpaceFromMemories();
    std::vector<std::shared_ptr<HypergraphNode>> getAtomSpaceNodes() const;
    std::vector<std::shared_ptr<HypergraphEdge>> getAtomSpaceEdges() const;
    
private:
    std::vector<std::shared_ptr<SymbolicReasoner>> symbolicReasoners_;
    std::vector<std::shared_ptr<ConnectionistProcessor>> connectionistProcessors_;
    std::vector<std::shared_ptr<PatternMatcher>> patternMatchers_;
    
    // PLN integration
    std::shared_ptr<PLNInferenceEngine> plnEngine_;
    
    // Memory storage for fusion
    std::vector<std::shared_ptr<Memory>> memoryStore_;
    
    // AtomSpace representation
    std::vector<std::shared_ptr<HypergraphNode>> atomSpaceNodes_;
    std::vector<std::shared_ptr<HypergraphEdge>> atomSpaceEdges_;
    
    mutable std::mutex reasonersMutex_;
    mutable std::mutex processorsMutex_;
    mutable std::mutex matchersMutex_;
    mutable std::mutex memoryMutex_;
    mutable std::mutex atomSpaceMutex_;
    
    // Internal fusion helpers
    TruthValue fuseResults(const std::vector<InferenceResult>& results);
    double calculateOverallConfidence(const ReasoningResult& result);
};

/**
 * Action interface for agent behaviors
 * Corresponds to the Action system in the TypeScript implementation
 */
class Action {
public:
    virtual ~Action() = default;
    virtual std::string getName() const = 0;
    virtual bool validate(const State& state, std::shared_ptr<Memory> message) const = 0;
    virtual bool execute(State& state, std::shared_ptr<Memory> message) = 0;
};

/**
 * Provider interface for state composition
 * Corresponds to the Provider interface in the TypeScript implementation
 */
class Provider {
public:
    virtual ~Provider() = default;
    virtual std::string getName() const = 0;
    virtual std::unordered_map<std::string, std::string> get(const State& state, std::shared_ptr<Memory> message) = 0;
};

// Utility functions
std::string generateUUID();

} // namespace elizaos