#pragma once

#include "elizaos/core.hpp"
#include "elizaos/discord_summarizer.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <functional>
#include <regex>
#include <mutex>
#include <atomic>

namespace elizaos {

/**
 * Discord scrubbing and content management extension
 * Provides content filtering, moderation, and cleanup capabilities
 */

// Content filtering rules
enum class FilterAction {
    NONE,
    WARN,
    DELETE,
    TIMEOUT,
    KICK,
    BAN
};

struct ContentFilter {
    std::string name;
    std::string description;
    std::regex pattern;
    FilterAction action;
    int severity;               // 1-10 scale
    bool enabled;
    std::string reason;
    
    ContentFilter() : action(FilterAction::NONE), severity(1), enabled(true) {}
    ContentFilter(const std::string& n, const std::string& patternStr, FilterAction a, int sev = 1)
        : name(n), pattern(patternStr), action(a), severity(sev), enabled(true) {}
};

// Moderation action record
struct ModerationAction {
    std::string id;
    std::string userId;
    std::string moderatorId;
    std::string channelId;
    std::string messageId;
    FilterAction action;
    std::string reason;
    std::chrono::system_clock::time_point timestamp;
    bool appealed;
    std::string appealReason;
    
    ModerationAction() : action(FilterAction::NONE), appealed(false) {
        timestamp = std::chrono::system_clock::now();
    }
};

// User reputation system
struct UserReputation {
    std::string userId;
    int reputationScore;        // Can be negative
    int warningCount;
    int timeoutCount;
    int kickCount;
    int banCount;
    std::chrono::system_clock::time_point lastIncident;
    std::vector<std::string> violations;
    bool isTrusted;
    
    UserReputation() : reputationScore(100), warningCount(0), timeoutCount(0), 
                       kickCount(0), banCount(0), isTrusted(false) {}
    UserReputation(const std::string& uid) : UserReputation() { userId = uid; }
};

// Content cleanup configuration
struct CleanupConfig {
    bool deleteSpam;
    bool deleteBot;
    bool deleteDuplicates;
    bool deleteEmpty;
    bool deleteOldMessages;
    std::chrono::hours maxAge;
    int maxDuplicateCount;
    std::vector<std::string> preserveChannels;
    
    CleanupConfig() : deleteSpam(true), deleteBot(false), deleteDuplicates(true),
                      deleteEmpty(true), deleteOldMessages(false), maxAge(24 * 30),
                      maxDuplicateCount(3) {}
};

// Content scanner for detecting violations
class ContentScanner {
public:
    ContentScanner();
    ~ContentScanner();
    
    // Filter management
    void addFilter(const ContentFilter& filter);
    void removeFilter(const std::string& name);
    void updateFilter(const std::string& name, const ContentFilter& filter);
    std::vector<ContentFilter> getFilters() const;
    
    // Content scanning
    struct ScanResult {
        bool violation;
        std::vector<std::string> triggeredFilters;
        FilterAction recommendedAction;
        int totalSeverity;
        std::string reason;
        
        ScanResult() : violation(false), recommendedAction(FilterAction::NONE), totalSeverity(0) {}
    };
    
    ScanResult scanMessage(const DiscordMessage& message);
    ScanResult scanContent(const std::string& content);
    std::vector<ScanResult> scanMessages(const std::vector<DiscordMessage>& messages);
    
    // Built-in filter categories
    void enableProfanityFilter(bool enable = true);
    void enableSpamFilter(bool enable = true);
    void enablePhishingFilter(bool enable = true);
    void enableInviteFilter(bool enable = true);
    void enableMentionSpamFilter(bool enable = true, int maxMentions = 5);
    
    // Custom pattern management
    void addProfanityWords(const std::vector<std::string>& words);
    void addAllowedDomains(const std::vector<std::string>& domains);
    void addBlockedDomains(const std::vector<std::string>& domains);
    
private:
    std::vector<ContentFilter> filters_;
    std::unordered_set<std::string> profanityWords_;
    std::unordered_set<std::string> allowedDomains_;
    std::unordered_set<std::string> blockedDomains_;
    
    bool profanityFilterEnabled_;
    bool spamFilterEnabled_;
    bool phishingFilterEnabled_;
    bool inviteFilterEnabled_;
    bool mentionSpamEnabled_;
    int maxMentions_;
    
    mutable std::mutex scannerMutex_;
    
    // Built-in detection methods
    bool detectProfanity(const std::string& content);
    bool detectSpam(const DiscordMessage& message);
    bool detectPhishing(const std::string& content);
    bool detectInviteLinks(const std::string& content);
    bool detectMentionSpam(const DiscordMessage& message);
    
    // Helper methods
    std::vector<std::string> extractUrls(const std::string& content);
    int countMentions(const std::string& content);
};

// Automated moderation system
class AutoModerator {
public:
    AutoModerator();
    ~AutoModerator();
    
    // Moderation operations
    bool processMessage(const DiscordMessage& message);
    bool processEdit(const DiscordMessage& oldMessage, const DiscordMessage& newMessage);
    bool reviewUser(const std::string& userId);
    
    // Action execution
    bool executeAction(const ModerationAction& action);
    bool warnUser(const std::string& userId, const std::string& reason, const std::string& channelId = "");
    bool timeoutUser(const std::string& userId, int minutes, const std::string& reason);
    bool kickUser(const std::string& userId, const std::string& reason);
    bool banUser(const std::string& userId, const std::string& reason, int deleteMessageDays = 0);
    bool deleteMessage(const std::string& channelId, const std::string& messageId, const std::string& reason = "");
    
    // Reputation management
    void updateUserReputation(const std::string& userId, int change, const std::string& reason);
    UserReputation getUserReputation(const std::string& userId);
    void setTrustedUser(const std::string& userId, bool trusted);
    
    // Configuration
    void setStrictMode(bool strict);
    void setAutoEscalation(bool enable);
    void setReputationThreshold(int threshold);
    void setActionCooldown(int seconds);
    
    // Action history
    std::vector<ModerationAction> getUserActions(const std::string& userId);
    std::vector<ModerationAction> getChannelActions(const std::string& channelId);
    std::vector<ModerationAction> getRecentActions(int hours = 24);
    
    // Appeals system
    bool submitAppeal(const std::string& actionId, const std::string& reason);
    bool reviewAppeal(const std::string& actionId, bool approved, const std::string& moderatorId);
    std::vector<ModerationAction> getPendingAppeals();
    
private:
    ContentScanner scanner_;
    std::unordered_map<std::string, UserReputation> userReputations_;
    std::unordered_map<std::string, ModerationAction> actionHistory_;
    
    bool strictMode_;
    bool autoEscalation_;
    int reputationThreshold_;
    int actionCooldownSeconds_;
    
    mutable std::mutex moderatorMutex_;
    
    // Internal moderation logic
    FilterAction determineAction(const ContentScanner::ScanResult& scanResult, const UserReputation& reputation);
    bool shouldEscalate(const UserReputation& reputation);
    bool isOnCooldown(const std::string& userId);
    void logAction(const ModerationAction& action);
};

// Content cleanup and maintenance
class ContentCleaner {
public:
    ContentCleaner();
    ~ContentCleaner();
    
    // Cleanup operations
    struct CleanupResult {
        int messagesDeleted;
        int duplicatesRemoved;
        int spamRemoved;
        int emptyRemoved;
        int oldRemoved;
        std::vector<std::string> errors;
        
        CleanupResult() : messagesDeleted(0), duplicatesRemoved(0), spamRemoved(0),
                         emptyRemoved(0), oldRemoved(0) {}
    };
    
    CleanupResult cleanChannel(const std::string& channelId, const CleanupConfig& config);
    CleanupResult cleanGuild(const std::string& guildId, const CleanupConfig& config);
    std::vector<CleanupResult> cleanAllChannels(const CleanupConfig& config);
    
    // Scheduled cleanup
    void scheduleCleanup(const std::string& channelId, const CleanupConfig& config, 
                        const std::chrono::hours& interval);
    void cancelScheduledCleanup(const std::string& channelId);
    std::vector<std::string> getScheduledCleanups() const;
    
    // Duplicate detection
    std::vector<std::vector<DiscordMessage>> findDuplicateMessages(const std::string& channelId);
    bool areDuplicates(const DiscordMessage& msg1, const DiscordMessage& msg2, double threshold = 0.8);
    
    // Bulk operations
    bool bulkDeleteMessages(const std::string& channelId, const std::vector<std::string>& messageIds);
    bool archiveChannel(const std::string& channelId, const std::string& archivePath);
    bool restoreFromArchive(const std::string& channelId, const std::string& archivePath);
    
private:
    std::unordered_map<std::string, CleanupConfig> scheduledCleanups_;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> nextCleanupTimes_;
    std::vector<std::thread> cleanupThreads_;
    std::atomic<bool> cleanupRunning_;
    
    mutable std::mutex cleanerMutex_;
    
    // Cleanup implementation
    std::vector<DiscordMessage> findMessagesToDelete(const std::string& channelId, const CleanupConfig& config);
    bool isSpamMessage(const DiscordMessage& message);
    bool isEmptyMessage(const DiscordMessage& message);
    bool isOldMessage(const DiscordMessage& message, const std::chrono::hours& maxAge);
    double calculateMessageSimilarity(const DiscordMessage& msg1, const DiscordMessage& msg2);
    
    // Scheduled cleanup thread
    void cleanupLoop();
};

// Analytics and reporting
class ModerationAnalytics {
public:
    ModerationAnalytics();
    ~ModerationAnalytics();
    
    // Report generation
    struct ModerationReport {
        std::chrono::system_clock::time_point periodStart;
        std::chrono::system_clock::time_point periodEnd;
        
        int totalActions;
        int warningsIssued;
        int timeoutsIssued;
        int kicksIssued;
        int bansIssued;
        int messagesDeleted;
        
        std::vector<std::string> topViolators;
        std::vector<std::string> commonViolations;
        std::unordered_map<std::string, int> violationsByChannel;
        double averageResponseTime;
        
        ModerationReport() : totalActions(0), warningsIssued(0), timeoutsIssued(0),
                           kicksIssued(0), bansIssued(0), messagesDeleted(0), averageResponseTime(0.0) {}
    };
    
    ModerationReport generateReport(const std::chrono::system_clock::time_point& startTime,
                                   const std::chrono::system_clock::time_point& endTime);
    ModerationReport generateDailyReport();
    ModerationReport generateWeeklyReport();
    ModerationReport generateMonthlyReport();
    
    // Trend analysis
    std::vector<double> getViolationTrends(int days = 30);
    std::vector<std::string> getTopViolationTypes(int limit = 10);
    std::unordered_map<std::string, double> getChannelRiskScores();
    
    // Export capabilities
    std::string exportReportAsJson(const ModerationReport& report);
    std::string exportReportAsHtml(const ModerationReport& report);
    bool exportReportToFile(const ModerationReport& report, const std::string& filePath);
    
private:
    mutable std::mutex analyticsMutex_;
    
    // Data aggregation
    std::vector<ModerationAction> getActionsInPeriod(const std::chrono::system_clock::time_point& start,
                                                     const std::chrono::system_clock::time_point& end);
    std::vector<std::string> findTopViolators(const std::vector<ModerationAction>& actions, int limit = 5);
    std::vector<std::string> findCommonViolations(const std::vector<ModerationAction>& actions, int limit = 5);
};

// Main discrub extension
class DiscrubExtension {
public:
    DiscrubExtension();
    ~DiscrubExtension();
    
    // Component access
    ContentScanner& getScanner() { return scanner_; }
    AutoModerator& getModerator() { return moderator_; }
    ContentCleaner& getCleaner() { return cleaner_; }
    ModerationAnalytics& getAnalytics() { return analytics_; }
    
    // High-level operations
    bool initializeWithDiscord(std::shared_ptr<DiscordClient> client);
    void startMonitoring(const std::vector<std::string>& channelIds);
    void stopMonitoring();
    bool isMonitoring() const;
    
    // Real-time processing
    void processIncomingMessage(const DiscordMessage& message);
    void processMessageEdit(const DiscordMessage& oldMessage, const DiscordMessage& newMessage);
    void processMessageDelete(const std::string& channelId, const std::string& messageId);
    
    // Batch operations
    std::future<ContentCleaner::CleanupResult> scheduleBatchCleanup(const std::string& channelId, 
                                                                    const CleanupConfig& config);
    std::future<ModerationAnalytics::ModerationReport> generateReport(
        const std::chrono::system_clock::time_point& startTime,
        const std::chrono::system_clock::time_point& endTime);
    
    // Configuration management
    void loadConfiguration(const std::string& configPath);
    void saveConfiguration(const std::string& configPath);
    void setDefaultModerationSettings();
    
    // Event handlers
    void setViolationHandler(std::function<void(const DiscordMessage&, const ContentScanner::ScanResult&)> handler);
    void setActionHandler(std::function<void(const ModerationAction&)> handler);
    void setCleanupHandler(std::function<void(const ContentCleaner::CleanupResult&)> handler);
    
private:
    ContentScanner scanner_;
    AutoModerator moderator_;
    ContentCleaner cleaner_;
    ModerationAnalytics analytics_;
    
    std::shared_ptr<DiscordClient> discordClient_;
    std::vector<std::string> monitoredChannels_;
    std::atomic<bool> monitoring_;
    std::thread monitoringThread_;
    
    // Event handlers
    std::function<void(const DiscordMessage&, const ContentScanner::ScanResult&)> violationHandler_;
    std::function<void(const ModerationAction&)> actionHandler_;
    std::function<void(const ContentCleaner::CleanupResult&)> cleanupHandler_;
    
    std::unordered_map<std::string, std::string> config_;
    mutable std::mutex configMutex_;
    
    // Internal processing
    void monitoringLoop();
    void handleViolation(const DiscordMessage& message, const ContentScanner::ScanResult& result);
};

// Global extension instance
extern std::shared_ptr<DiscrubExtension> globalDiscrubExtension;

} // namespace elizaos