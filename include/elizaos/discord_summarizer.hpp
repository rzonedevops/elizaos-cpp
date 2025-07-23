#pragma once

#include "elizaos/core.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <future>
#include <functional>
#include <mutex>

namespace elizaos {

/**
 * Discord message summarization and analysis module
 * Provides Discord integration, message processing, and content analysis
 */

// Discord message representation
struct DiscordMessage {
    std::string id;
    std::string channelId;
    std::string guildId;
    std::string authorId;
    std::string authorName;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    std::vector<std::string> attachments;
    std::vector<std::string> embeds;
    std::vector<std::string> reactions;
    bool isBot;
    
    DiscordMessage() : isBot(false) {}
    DiscordMessage(const std::string& msgId, const std::string& chanId, 
                   const std::string& author, const std::string& text)
        : id(msgId), channelId(chanId), authorName(author), content(text), isBot(false) {
        timestamp = std::chrono::system_clock::now();
    }
};

// Discord channel information
struct DiscordChannel {
    std::string id;
    std::string name;
    std::string type;
    std::string guildId;
    std::string topic;
    bool isNsfw;
    
    DiscordChannel() : isNsfw(false) {}
    DiscordChannel(const std::string& chanId, const std::string& chanName, const std::string& guildId)
        : id(chanId), name(chanName), guildId(guildId), isNsfw(false) {}
};

// Discord guild (server) information
struct DiscordGuild {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> channels;
    std::vector<std::string> members;
    
    DiscordGuild() = default;
    DiscordGuild(const std::string& guildId, const std::string& guildName)
        : id(guildId), name(guildName) {}
};

// Message analysis results
struct MessageAnalysis {
    std::string messageId;
    double sentiment;           // -1.0 (negative) to 1.0 (positive)
    std::vector<std::string> topics;
    std::vector<std::string> keywords;
    std::unordered_map<std::string, double> categories;  // category -> confidence
    int toxicityLevel;         // 0-10 scale
    bool containsSpam;
    std::string language;
    
    MessageAnalysis() : sentiment(0.0), toxicityLevel(0), containsSpam(false) {}
};

// Channel summary statistics
struct ChannelSummary {
    std::string channelId;
    std::string channelName;
    std::chrono::system_clock::time_point periodStart;
    std::chrono::system_clock::time_point periodEnd;
    
    int totalMessages;
    int uniqueUsers;
    std::vector<std::string> topUsers;
    std::vector<std::string> mainTopics;
    double averageSentiment;
    std::string mostActiveTime;
    
    ChannelSummary() : totalMessages(0), uniqueUsers(0), averageSentiment(0.0) {}
};

// Discord API client interface
class DiscordClient {
public:
    DiscordClient();
    virtual ~DiscordClient();
    
    // Connection management
    virtual bool connect(const std::string& token);
    virtual bool disconnect();
    virtual bool isConnected() const;
    
    // Message operations
    virtual std::vector<DiscordMessage> getMessages(const std::string& channelId, int limit = 100);
    virtual std::vector<DiscordMessage> getMessagesSince(const std::string& channelId, 
                                                         const std::chrono::system_clock::time_point& since);
    virtual bool sendMessage(const std::string& channelId, const std::string& content);
    virtual bool deleteMessage(const std::string& channelId, const std::string& messageId);
    
    // Channel operations
    virtual std::vector<DiscordChannel> getChannels(const std::string& guildId);
    virtual DiscordChannel getChannel(const std::string& channelId);
    
    // Guild operations
    virtual std::vector<DiscordGuild> getGuilds();
    virtual DiscordGuild getGuild(const std::string& guildId);
    
    // Event handling
    virtual void setMessageHandler(std::function<void(const DiscordMessage&)> handler);
    virtual void setChannelHandler(std::function<void(const DiscordChannel&)> handler);
    
protected:
    bool connected_;
    std::string token_;
    std::function<void(const DiscordMessage&)> messageHandler_;
    std::function<void(const DiscordChannel&)> channelHandler_;
    mutable std::mutex clientMutex_;
};

// Message analysis engine
class MessageAnalyzer {
public:
    MessageAnalyzer();
    ~MessageAnalyzer();
    
    // Analysis operations
    MessageAnalysis analyzeMessage(const DiscordMessage& message);
    std::vector<MessageAnalysis> analyzeMessages(const std::vector<DiscordMessage>& messages);
    
    // Sentiment analysis
    double calculateSentiment(const std::string& content);
    std::string classifySentiment(double sentimentScore);
    
    // Topic extraction
    std::vector<std::string> extractTopics(const std::string& content);
    std::vector<std::string> extractKeywords(const std::string& content);
    
    // Content classification
    std::unordered_map<std::string, double> classifyContent(const std::string& content);
    int assessToxicity(const std::string& content);
    bool detectSpam(const DiscordMessage& message);
    
    // Language detection
    std::string detectLanguage(const std::string& content);
    
    // Configuration
    void setToxicityThreshold(int threshold);
    void setSentimentModel(const std::string& modelPath);
    void addTopicCategory(const std::string& category, const std::vector<std::string>& keywords);
    
private:
    int toxicityThreshold_;
    std::string sentimentModelPath_;
    std::unordered_map<std::string, std::vector<std::string>> topicCategories_;
    mutable std::mutex analyzerMutex_;
    
    // Internal analysis helpers
    std::vector<std::string> tokenizeText(const std::string& text);
    double scoreKeywordMatch(const std::string& text, const std::vector<std::string>& keywords);
    bool containsProfanity(const std::string& content);
};

// Channel summarization engine
class ChannelSummarizer {
public:
    ChannelSummarizer();
    ~ChannelSummarizer();
    
    // Summarization operations
    ChannelSummary summarizeChannel(const std::string& channelId, 
                                   const std::chrono::system_clock::time_point& startTime,
                                   const std::chrono::system_clock::time_point& endTime);
    ChannelSummary summarizeChannelDaily(const std::string& channelId);
    ChannelSummary summarizeChannelWeekly(const std::string& channelId);
    
    // Bulk operations
    std::vector<ChannelSummary> summarizeAllChannels(const std::string& guildId,
                                                     const std::chrono::system_clock::time_point& startTime,
                                                     const std::chrono::system_clock::time_point& endTime);
    
    // Report generation
    std::string generateTextReport(const ChannelSummary& summary);
    std::string generateJsonReport(const ChannelSummary& summary);
    std::string generateHtmlReport(const ChannelSummary& summary);
    
    // Configuration
    void setTopUsersLimit(int limit);
    void setTopTopicsLimit(int limit);
    void setMinimumMessages(int minimum);
    
private:
    int topUsersLimit_;
    int topTopicsLimit_;
    int minimumMessages_;
    mutable std::mutex summarizerMutex_;
    
    // Analysis helpers
    std::vector<std::string> findTopUsers(const std::vector<DiscordMessage>& messages, int limit);
    std::vector<std::string> findMainTopics(const std::vector<MessageAnalysis>& analyses, int limit);
    double calculateAverageSentiment(const std::vector<MessageAnalysis>& analyses);
    std::string findMostActiveTime(const std::vector<DiscordMessage>& messages);
};

// Data persistence and caching
class DiscordDataManager {
public:
    DiscordDataManager();
    ~DiscordDataManager();
    
    // Message storage
    bool storeMessage(const DiscordMessage& message);
    bool storeMessages(const std::vector<DiscordMessage>& messages);
    std::vector<DiscordMessage> retrieveMessages(const std::string& channelId, int limit = 100);
    
    // Analysis storage
    bool storeAnalysis(const MessageAnalysis& analysis);
    bool storeAnalyses(const std::vector<MessageAnalysis>& analyses);
    MessageAnalysis retrieveAnalysis(const std::string& messageId);
    
    // Summary storage
    bool storeSummary(const ChannelSummary& summary);
    std::vector<ChannelSummary> retrieveSummaries(const std::string& channelId);
    
    // Cache management
    void setCacheSize(size_t maxEntries);
    void clearCache();
    void enablePersistence(const std::string& dataPath);
    
private:
    std::unordered_map<std::string, DiscordMessage> messageCache_;
    std::unordered_map<std::string, MessageAnalysis> analysisCache_;
    std::unordered_map<std::string, std::vector<ChannelSummary>> summaryCache_;
    
    size_t maxCacheSize_;
    std::string persistencePath_;
    bool persistenceEnabled_;
    mutable std::mutex dataMutex_;
    
    // Persistence helpers
    bool saveToFile(const std::string& filePath, const std::string& data);
    std::string loadFromFile(const std::string& filePath);
};

// Main Discord summarizer
class DiscordSummarizer {
public:
    DiscordSummarizer();
    ~DiscordSummarizer();
    
    // Component access
    DiscordClient& getClient() { return *client_; }
    MessageAnalyzer& getAnalyzer() { return analyzer_; }
    ChannelSummarizer& getSummarizer() { return summarizer_; }
    DiscordDataManager& getDataManager() { return dataManager_; }
    
    // High-level operations
    bool initializeWithToken(const std::string& token);
    std::future<ChannelSummary> generateChannelSummary(const std::string& channelId,
                                                       const std::chrono::system_clock::time_point& startTime,
                                                       const std::chrono::system_clock::time_point& endTime);
    std::future<std::vector<ChannelSummary>> generateGuildSummary(const std::string& guildId,
                                                                  const std::chrono::system_clock::time_point& startTime,
                                                                  const std::chrono::system_clock::time_point& endTime);
    
    // Automated monitoring
    void startMonitoring(const std::vector<std::string>& channelIds);
    void stopMonitoring();
    bool isMonitoring() const;
    
    // Configuration
    void loadConfiguration(const std::string& configPath);
    void saveConfiguration(const std::string& configPath);
    
private:
    std::unique_ptr<DiscordClient> client_;
    MessageAnalyzer analyzer_;
    ChannelSummarizer summarizer_;
    DiscordDataManager dataManager_;
    
    std::vector<std::string> monitoredChannels_;
    std::atomic<bool> monitoring_;
    std::thread monitoringThread_;
    
    std::unordered_map<std::string, std::string> config_;
    mutable std::mutex configMutex_;
    
    // Monitoring implementation
    void monitoringLoop();
    void processNewMessage(const DiscordMessage& message);
};

// Global summarizer instance
extern std::shared_ptr<DiscordSummarizer> globalDiscordSummarizer;

} // namespace elizaos