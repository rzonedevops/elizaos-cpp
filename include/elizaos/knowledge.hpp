#pragma once

#include "elizaos/core.hpp"
#include "elizaos/agentmemory.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentaction.hpp"  // For JsonValue definition
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <any>

namespace elizaos {

// Knowledge entry types
enum class KnowledgeType {
    FACT,
    RULE,
    CONCEPT,
    RELATIONSHIP,
    PROCEDURE,
    EXPERIENCE
};

// Knowledge confidence levels
enum class ConfidenceLevel {
    VERY_LOW = 1,
    LOW = 2,
    MEDIUM = 3,
    HIGH = 4,
    VERY_HIGH = 5
};

// Knowledge source types
enum class KnowledgeSource {
    LEARNED,
    PROGRAMMED,
    INFERRED,
    OBSERVED,
    COMMUNICATED
};

// Forward declarations
class KnowledgeBase;
class KnowledgeEntry;
class KnowledgeQuery;

// Knowledge entry structure
class KnowledgeEntry {
public:
    std::string id;
    std::string content;
    KnowledgeType type;
    ConfidenceLevel confidence;
    KnowledgeSource source;
    std::vector<std::string> tags;
    std::unordered_map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::vector<std::string> related_entries;
    
    KnowledgeEntry(const std::string& content, KnowledgeType type = KnowledgeType::FACT);
    
    // Serialization methods
    JsonValue toJson() const;
    static KnowledgeEntry fromJson(const JsonValue& json);
    
    // Utility methods
    void addTag(const std::string& tag);
    void addRelation(const std::string& entryId);
    void updateConfidence(ConfidenceLevel newConfidence);
    bool hasTag(const std::string& tag) const;
};

// Knowledge query parameters
class KnowledgeQuery {
public:
    std::string text;
    std::vector<KnowledgeType> types;
    std::vector<std::string> tags;
    ConfidenceLevel minConfidence = ConfidenceLevel::VERY_LOW;
    int maxResults = 10;
    bool includeRelated = false;
    
    KnowledgeQuery(const std::string& queryText);
};

// Knowledge inference engine
class KnowledgeInferenceEngine {
public:
    KnowledgeInferenceEngine();
    
    // Inference methods
    std::vector<KnowledgeEntry> inferFromFacts(const std::vector<KnowledgeEntry>& facts);
    std::vector<KnowledgeEntry> findRelatedConcepts(const KnowledgeEntry& entry);
    KnowledgeEntry combineEvidence(const std::vector<KnowledgeEntry>& evidence);
    
    // Rule-based inference
    void addInferenceRule(const std::string& ruleName, 
                         std::function<std::vector<KnowledgeEntry>(const std::vector<KnowledgeEntry>&)> rule);
    void removeInferenceRule(const std::string& ruleName);
    
private:
    std::unordered_map<std::string, std::function<std::vector<KnowledgeEntry>(const std::vector<KnowledgeEntry>&)>> rules_;
    std::mutex rulesMutex_;
};

// Main knowledge base class
class KnowledgeBase {
public:
    KnowledgeBase();
    ~KnowledgeBase();
    
    // Core knowledge management
    std::string addKnowledge(const KnowledgeEntry& entry);
    bool updateKnowledge(const std::string& id, const KnowledgeEntry& entry);
    bool removeKnowledge(const std::string& id);
    std::optional<KnowledgeEntry> getKnowledge(const std::string& id);
    
    // Query methods
    std::vector<KnowledgeEntry> query(const KnowledgeQuery& query);
    std::vector<KnowledgeEntry> searchByText(const std::string& text, int maxResults = 10);
    std::vector<KnowledgeEntry> searchByTags(const std::vector<std::string>& tags, int maxResults = 10);
    std::vector<KnowledgeEntry> getRelatedKnowledge(const std::string& entryId, int maxResults = 5);
    
    // Knowledge organization
    std::vector<std::string> getAllTags() const;
    std::unordered_map<KnowledgeType, int> getKnowledgeTypeStats() const;
    std::vector<KnowledgeEntry> getKnowledgeByType(KnowledgeType type);
    
    // Knowledge validation and maintenance
    void validateKnowledge();
    void pruneOldKnowledge(std::chrono::hours maxAge);
    void consolidateKnowledge();
    
    // Inference integration
    std::vector<KnowledgeEntry> performInference(const KnowledgeQuery& query);
    void setInferenceEngine(std::shared_ptr<KnowledgeInferenceEngine> engine);
    
    // Import/Export
    bool exportToFile(const std::string& filename) const;
    bool importFromFile(const std::string& filename);
    JsonValue exportToJson() const;
    bool importFromJson(const JsonValue& data);
    
    // Statistics and debugging
    size_t getKnowledgeCount() const;
    std::string getStatistics() const;
    void clear();
    
private:
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<AgentLogger> logger_;
    std::shared_ptr<KnowledgeInferenceEngine> inferenceEngine_;
    mutable std::mutex knowledgeMutex_;
    
    // Internal helper methods
    std::string generateKnowledgeId();
    void saveKnowledgeToMemory(const KnowledgeEntry& entry);
    std::optional<KnowledgeEntry> loadKnowledgeFromMemory(const std::string& id);
    std::vector<KnowledgeEntry> searchMemoryByContent(const std::string& content, int maxResults);
    std::vector<KnowledgeEntry> getAllKnowledgeFromMemory() const;
    
    // Knowledge validation helpers
    bool isValidKnowledgeEntry(const KnowledgeEntry& entry) const;
    void updateKnowledgeMetrics(const KnowledgeEntry& entry);
};

// Global knowledge base instance
extern std::shared_ptr<KnowledgeBase> globalKnowledgeBase;

// Utility functions
std::string knowledgeTypeToString(KnowledgeType type);
KnowledgeType stringToKnowledgeType(const std::string& typeStr);
std::string confidenceLevelToString(ConfidenceLevel level);
ConfidenceLevel stringToConfidenceLevel(const std::string& levelStr);
std::string knowledgeSourceToString(KnowledgeSource source);
KnowledgeSource stringToKnowledgeSource(const std::string& sourceStr);

} // namespace elizaos