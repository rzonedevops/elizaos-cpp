#pragma once

#include "elizaos/core.hpp"
#include "elizaos/agentmemory.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentaction.hpp"
#include "elizaos/knowledge.hpp"
#include "elizaos/characters.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <functional>

namespace elizaos {

// Forward declarations
class ElizaCore;
class ConversationContext;
class ResponsePattern;
class ResponseGenerator;
class EmotionalStateTracker;

// Conversation turn tracking
struct ConversationTurn {
    std::string id;
    std::string input;
    std::string response;
    std::chrono::system_clock::time_point timestamp;
    std::string emotionalState;
    std::unordered_map<std::string, std::string> metadata;
    float confidence = 0.0f;
    
    ConversationTurn(const std::string& input, const std::string& response);
};

// Conversation context management
class ConversationContext {
public:
    std::string sessionId;
    std::string userId;
    std::string characterId;
    std::vector<ConversationTurn> history;
    std::unordered_map<std::string, std::string> sessionData;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point lastActivity;
    
    ConversationContext() = default;  // Add default constructor
    ConversationContext(const std::string& sessionId, const std::string& userId = "");
    
    void addTurn(const ConversationTurn& turn);
    std::vector<ConversationTurn> getRecentHistory(int count = 5) const;
    std::string getContextSummary() const;
    void setSessionData(const std::string& key, const std::string& value);
    std::string getSessionData(const std::string& key) const;
    void updateLastActivity();
    
    JsonValue toJson() const;
    static ConversationContext fromJson(const JsonValue& json);
};

// Response pattern for pattern matching and generation
class ResponsePattern {
public:
    std::string id;
    std::string pattern;              // Regex pattern to match input
    std::vector<std::string> responses; // Possible responses
    std::string category;             // Pattern category (greeting, question, etc.)
    float priority = 1.0f;           // Pattern matching priority
    std::vector<std::string> conditions; // Additional conditions
    std::unordered_map<std::string, std::string> metadata;
    
    ResponsePattern(const std::string& pattern, const std::vector<std::string>& responses,
                   const std::string& category = "general");
    
    bool matches(const std::string& input) const;
    std::string generateResponse(const std::unordered_map<std::string, std::string>& captures = {}) const;
    std::vector<std::string> extractCaptures(const std::string& input) const;
    
    JsonValue toJson() const;
    static ResponsePattern fromJson(const JsonValue& json);
};

// Emotional state tracking for conversation
class EmotionalStateTracker {
public:
    float happiness = 0.5f;
    float sadness = 0.1f;
    float anger = 0.1f;
    float fear = 0.1f;
    float surprise = 0.2f;
    float disgust = 0.1f;
    float excitement = 0.3f;
    float calmness = 0.6f;
    
    EmotionalStateTracker() = default;
    
    void updateFromInput(const std::string& input);
    void updateFromInteraction(const std::string& outcome);
    void decay(float factor = 0.95f); // Emotional decay over time
    std::string getDominantEmotion() const;
    float getEmotionalIntensity() const;
    void adjustEmotion(const std::string& emotion, float adjustment);
    
    JsonValue toJson() const;
    static EmotionalStateTracker fromJson(const JsonValue& json);
    
private:
    void normalizeEmotions();
    std::vector<std::string> detectEmotionalWords(const std::string& input) const;
};

// Response generation engine
class ResponseGenerator {
public:
    ResponseGenerator();
    ~ResponseGenerator() = default;
    
    // Response generation methods
    std::string generateResponse(const std::string& input, 
                               const ConversationContext& context,
                               const CharacterProfile* character = nullptr);
    
    void addPattern(const ResponsePattern& pattern);
    void removePattern(const std::string& patternId);
    std::vector<ResponsePattern> getMatchingPatterns(const std::string& input) const;
    
    // Knowledge integration
    void setKnowledgeBase(std::shared_ptr<KnowledgeBase> kb);
    std::string generateKnowledgeBasedResponse(const std::string& input) const;
    
    // Character-based response generation
    std::string generateCharacterResponse(const std::string& input,
                                        const CharacterProfile& character,
                                        const ConversationContext& context) const;
    
    // Template response generation
    std::string processResponseTemplate(const std::string& template_,
                                      const std::unordered_map<std::string, std::string>& variables) const;
    
    void loadDefaultPatterns();
    void clear();
    
private:
    std::vector<ResponsePattern> patterns_;
    std::shared_ptr<KnowledgeBase> knowledgeBase_;
    std::mutex patternsMutex_;
    bool knowledgeIntegrationEnabled_ = true;  // Add missing flag
    
    std::string selectBestResponse(const std::vector<ResponsePattern>& patterns,
                                 const std::string& input) const;
    std::unordered_map<std::string, std::string> extractVariables(const std::string& input) const;
};

// Main Eliza conversational AI engine
class ElizaCore {
public:
    ElizaCore();
    ~ElizaCore();
    
    // Core conversation methods
    std::string processInput(const std::string& input,
                           const std::string& sessionId = "",
                           const std::string& userId = "");
    
    std::string processInputWithCharacter(const std::string& input,
                                        const std::string& characterId,
                                        const std::string& sessionId = "",
                                        const std::string& userId = "");
    
    // Session management
    std::string createSession(const std::string& userId = "", 
                            const std::string& characterId = "");
    bool endSession(const std::string& sessionId);
    std::optional<ConversationContext> getSession(const std::string& sessionId);
    std::vector<ConversationContext> getAllSessions() const;
    void cleanupOldSessions(std::chrono::hours maxAge = std::chrono::hours(24));
    
    // Character integration
    void setCharacter(const std::string& sessionId, const std::string& characterId);
    std::optional<CharacterProfile> getSessionCharacter(const std::string& sessionId);
    
    // Knowledge integration
    void setKnowledgeBase(std::shared_ptr<KnowledgeBase> kb);
    void setCharacterManager(std::shared_ptr<CharacterManager> cm);
    
    // Learning and adaptation
    void learnFromConversation(const std::string& sessionId);
    void updateResponsePatterns(const std::string& input, const std::string& feedback);
    
    // Configuration
    void setResponseGenerator(std::shared_ptr<ResponseGenerator> generator);
    void enableEmotionalTracking(bool enable = true);
    void enableKnowledgeIntegration(bool enable = true);
    void enableCharacterPersonality(bool enable = true);
    
    // Analytics and insights
    std::string getConversationAnalytics() const;
    std::vector<std::string> getFrequentTopics() const;
    std::unordered_map<std::string, int> getEmotionalStateStats() const;
    
    // Import/Export
    bool exportConversations(const std::string& filename) const;
    bool importConversations(const std::string& filename);
    void clearAllSessions();
    
    size_t getSessionCount() const;
    
private:
    std::unordered_map<std::string, ConversationContext> sessions_;
    std::shared_ptr<ResponseGenerator> responseGenerator_;
    std::shared_ptr<KnowledgeBase> knowledgeBase_;
    std::shared_ptr<CharacterManager> characterManager_;
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<AgentLogger> logger_;
    
    // Configuration flags
    bool emotionalTrackingEnabled_ = true;
    bool knowledgeIntegrationEnabled_ = true;
    bool characterPersonalityEnabled_ = true;
    
    mutable std::mutex sessionsMutex_;
    
    // Internal helper methods
    std::string generateSessionId();
    void saveSessionToMemory(const ConversationContext& session);
    std::optional<ConversationContext> loadSessionFromMemory(const std::string& sessionId);
    std::string preprocessInput(const std::string& input) const;
    std::string postprocessResponse(const std::string& response, 
                                  const ConversationContext& context) const;
    void updateEmotionalState(const std::string& sessionId, const std::string& input);
    void trackConversationMetrics(const ConversationContext& context);
};

// Global Eliza instance
extern std::shared_ptr<ElizaCore> globalElizaCore;

// Utility functions
std::string normalizeInput(const std::string& input);
std::vector<std::string> tokenizeInput(const std::string& input);
std::string extractSentiment(const std::string& input);
bool isQuestion(const std::string& input);
bool isGreeting(const std::string& input);
bool isGoodbye(const std::string& input);

// Predefined response patterns
namespace ElizaPatterns {
    std::vector<ResponsePattern> getGreetingPatterns();
    std::vector<ResponsePattern> getQuestionPatterns();
    std::vector<ResponsePattern> getEmotionalPatterns();
    std::vector<ResponsePattern> getReflectivePatterns();
    std::vector<ResponsePattern> getDefaultPatterns();
    std::vector<ResponsePattern> getAllPatterns();
}

} // namespace elizaos