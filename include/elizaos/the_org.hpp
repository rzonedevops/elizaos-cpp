#pragma once

#include "core.hpp"
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
#include <queue>
#include <future>

namespace elizaos {

// Forward declarations
class TheOrgAgent;
class CommunityManagerAgent;
class DeveloperRelationsAgent;
class CommunityLiaisonAgent;
class ProjectManagerAgent;
class SocialMediaManagerAgent;
class TheOrgManager;

/**
 * Platform integration types
 */
enum class PlatformType {
    DISCORD,
    TELEGRAM,
    TWITTER,
    SLACK,
    FACEBOOK,
    LINKEDIN,
    GITHUB
};

struct PlatformConfig {
    PlatformType type;
    std::string applicationId;
    std::string apiToken;
    std::string webhookUrl;
    std::unordered_map<std::string, std::string> additionalSettings;
};

/**
 * Agent role types for specialized behaviors
 */
enum class AgentRole {
    COMMUNITY_MANAGER,     // Eli5 - Welcomes users, moderates discussions
    DEVELOPER_RELATIONS,   // Eddy - Documentation support, technical assistance
    COMMUNITY_LIAISON,     // Ruby - Cross-org knowledge sharing, identifies synergies
    PROJECT_MANAGER,       // Jimmy - Coordinates projects, tracks progress
    SOCIAL_MEDIA_MANAGER  // Laura - Content creation and social media publishing
};

/**
 * Organization configuration and management
 */
struct OrganizationConfig {
    UUID id;
    std::string name;
    std::string description;
    std::vector<PlatformConfig> platforms;
    std::vector<std::string> subscribedTopics;
    std::vector<std::string> reportSubscriptions;
    std::unordered_map<std::string, std::string> customSettings;
};

/**
 * Project management structures
 */
enum class ProjectStatus {
    PLANNING,
    ACTIVE,
    ON_HOLD,
    COMPLETED,
    CANCELLED
};

struct TeamMemberAvailability {
    std::vector<std::string> workDays;  // Monday, Tuesday, etc.
    struct {
        std::string start;  // HH:MM format
        std::string end;    // HH:MM format
    } workHours;
    std::string timeZone;
    int hoursPerWeek;
    enum class EmploymentStatus {
        FULL_TIME,
        PART_TIME,
        FREELANCE,
        NONE
    } employmentStatus;
};

struct TeamMember {
    UUID id;
    std::string name;
    std::string role;
    TeamMemberAvailability availability;
    std::vector<std::string> skills;
    std::unordered_map<std::string, std::string> contactInfo;
};

struct Project {
    UUID id;
    std::string name;
    std::string description;
    ProjectStatus status;
    std::vector<UUID> teamMemberIds;
    std::vector<UUID> taskIds;
    Timestamp createdAt;
    Timestamp updatedAt;
    std::optional<Timestamp> deadline;
    std::unordered_map<std::string, std::string> metadata;
};

struct DailyUpdate {
    UUID id;
    UUID teamMemberId;
    UUID projectId;
    std::string date;  // ISO date format
    std::string summary;
    std::vector<std::string> accomplishments;
    std::vector<std::string> blockers;
    std::vector<std::string> plannedWork;
    Timestamp submittedAt;
};

/**
 * Social media content management
 */
enum class ContentType {
    TEXT_POST,
    IMAGE_POST,
    VIDEO_POST,
    LINK_SHARE,
    POLL,
    STORY,
    THREAD
};

enum class ContentStatus {
    DRAFT,
    SCHEDULED,
    PUBLISHED,
    FAILED
};

struct SocialMediaContent {
    UUID id;
    ContentType type;
    ContentStatus status;
    std::string title;
    std::string content;
    std::vector<std::string> mediaUrls;
    std::vector<std::string> hashtags;
    std::vector<PlatformType> targetPlatforms;
    std::optional<Timestamp> scheduledTime;
    Timestamp createdAt;
    Timestamp updatedAt;
    std::unordered_map<std::string, std::string> platformSpecificData;
};

/**
 * Cross-organizational intelligence structures
 */
enum class ReportType {
    DAILY,
    WEEKLY,
    TOPIC_SPECIFIC,
    MONTHLY,
    QUARTERLY
};

struct ParallelTopic {
    std::string topic;
    std::vector<UUID> organizationIds;
    std::vector<std::string> recentDiscussions;
    std::string potentialSynergies;
    double relevanceScore;
};

struct OrganizationUpdate {
    UUID orgId;
    std::string orgName;
    std::vector<std::string> activeTopics;
    std::vector<std::string> recentHighlights;
    std::vector<std::string> keyDiscussions;
    double activityLevel;
};

struct CrossOrgReport {
    UUID id;
    ReportType type;
    std::string date;
    struct {
        std::string overview;
        std::vector<ParallelTopic> parallelTopics;
        std::vector<OrganizationUpdate> organizationUpdates;
        std::vector<std::string> collaborationOpportunities;
        std::vector<std::string> knowledgeGaps;
    } content;
    Timestamp generatedAt;
    std::vector<UUID> recipientOrgIds;
};

/**
 * Community management structures
 */
enum class ModerationAction {
    WARNING,
    TIMEOUT,
    KICK,
    BAN,
    ROLE_ASSIGNMENT,
    CHANNEL_RESTRICTION
};

struct ModerationEvent {
    UUID id;
    UUID userId;
    UUID moderatorId;
    ModerationAction action;
    std::string reason;
    std::optional<std::chrono::seconds> duration;
    Timestamp timestamp;
    std::string channelId;
    std::string serverId;
};

struct CommunityMetrics {
    size_t totalMembers;
    size_t activeMembers;
    size_t newMembersToday;
    size_t messagesPerDay;
    double engagementRate;
    std::vector<std::string> topTopics;
    std::vector<std::string> mostActiveChannels;
    Timestamp lastUpdated;
};

/**
 * Base agent class for the_org system
 */
class TheOrgAgent {
public:
    TheOrgAgent(const AgentConfig& config, AgentRole role);
    virtual ~TheOrgAgent() = default;

    // Core agent interface
    virtual void initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual bool isRunning() const = 0;

    // Agent identity and role
    const UUID& getId() const { return config_.agentId; }
    const std::string& getName() const { return config_.agentName; }
    AgentRole getRole() const { return role_; }
    
    // State and memory management
    virtual State& getState() { return state_; }
    virtual const State& getState() const { return state_; }
    virtual std::shared_ptr<Memory> createMemory(const std::string& content, MemoryType type = MemoryType::MESSAGE);
    virtual void addMemory(std::shared_ptr<Memory> memory);
    virtual std::vector<std::shared_ptr<Memory>> searchMemories(const std::string& query, size_t maxResults = 10);

    // Platform integration
    virtual void addPlatform(const PlatformConfig& platform);
    virtual void removePlatform(PlatformType type);
    virtual bool sendMessage(PlatformType platform, const std::string& channelId, const std::string& message);
    virtual std::vector<std::string> getRecentMessages(PlatformType platform, const std::string& channelId, size_t count = 50);

    // Inter-agent communication
    virtual void sendToAgent(const UUID& agentId, const std::string& message, const std::string& type = "message");
    virtual std::queue<std::string> getIncomingMessages();
    virtual void processMessage(const std::string& message, const std::string& senderId);

    // Task management integration
    virtual UUID createTask(const std::string& name, const std::string& description, int priority = 0);
    virtual bool completeTask(const UUID& taskId);
    virtual std::vector<std::shared_ptr<Task>> getPendingTasks();

    // Configuration and settings
    virtual void updateConfig(const std::unordered_map<std::string, std::string>& settings);
    virtual std::string getConfigValue(const std::string& key) const;

protected:
    AgentConfig config_;
    AgentRole role_;
    State state_;
    std::vector<std::shared_ptr<Memory>> memoryStore_;
    std::unordered_map<PlatformType, PlatformConfig> platforms_;
    std::queue<std::string> incomingMessages_;
    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::unordered_map<std::string, std::string> settings_;
    
    mutable std::mutex memoryMutex_;
    mutable std::mutex platformMutex_;
    mutable std::mutex messageMutex_;
    mutable std::mutex settingsMutex_;
    
    // Internal helper methods
    virtual void processLoop() = 0;
    virtual bool validateMessage(const std::string& message) const;
    virtual std::string formatResponse(const std::string& response, PlatformType platform) const;
};

/**
 * Eli5 - Community Manager Agent
 * Welcomes new users, moderates discussions, manages community health
 */
class CommunityManagerAgent : public TheOrgAgent {
public:
    CommunityManagerAgent(const AgentConfig& config);
    
    // TheOrgAgent interface implementation
    void initialize() override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override;

    // Community management specific functionality
    void enableNewUserGreeting(const std::string& channelId, const std::string& greetingMessage = "");
    void disableNewUserGreeting();
    bool shouldGreetNewUser(const std::string& userId) const;
    std::string generateGreeting(const std::string& userName, const std::string& serverName = "") const;
    
    // Moderation capabilities
    void addModerationRule(const std::string& rule, ModerationAction action, const std::string& reason = "");
    void removeModerationRule(const std::string& rule);
    bool evaluateMessage(const std::string& message, const std::string& userId, const std::string& channelId);
    void applyModerationAction(const std::string& userId, ModerationAction action, const std::string& reason, 
                              std::optional<std::chrono::seconds> duration = std::nullopt);
    
    // Community metrics and health
    CommunityMetrics generateCommunityMetrics() const;
    void trackUserActivity(const std::string& userId, const std::string& activity);
    std::vector<std::string> identifyActiveUsers(std::chrono::hours timeWindow = std::chrono::hours(24)) const;
    std::vector<std::string> getTopTopics(std::chrono::hours timeWindow = std::chrono::hours(24)) const;
    
    // Conflict resolution
    void initiateConflictResolution(const std::vector<std::string>& userIds, const std::string& channelId);
    void escalateIssue(const std::string& description, const std::vector<std::string>& involvedUsers);
    
    // Community events and engagement
    void scheduleEvent(const std::string& eventName, const std::string& description, Timestamp scheduledTime);
    void announceEvent(const std::string& eventId, const std::vector<std::string>& channelIds);
    void trackEventParticipation(const std::string& eventId, const std::string& userId);

private:
    void processLoop() override;
    void processNewUserJoin(const std::string& userId, const std::string& serverId);
    void processMessageForModeration(const std::string& message, const std::string& userId, const std::string& channelId);
    void generateDailyReport();
    void updateCommunityMetrics();
    
    bool greetingEnabled_ = false;
    std::string greetingChannelId_;
    std::string customGreetingMessage_;
    std::unordered_map<std::string, std::pair<ModerationAction, std::string>> moderationRules_;
    std::vector<ModerationEvent> moderationHistory_;
    CommunityMetrics currentMetrics_;
    std::unordered_map<std::string, std::vector<Timestamp>> userActivity_;
    std::thread processingThread_;
    
    mutable std::mutex rulesMutex_;
    mutable std::mutex metricsMutex_;
    mutable std::mutex activityMutex_;
};

/**
 * Eddy - Developer Relations Agent
 * Provides documentation support, code examples, and technical assistance
 */
class DeveloperRelationsAgent : public TheOrgAgent {
public:
    DeveloperRelationsAgent(const AgentConfig& config);
    
    // TheOrgAgent interface implementation
    void initialize() override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override;

    // Documentation and support
    void indexDocumentation(const std::string& docPath, const std::string& version = "latest");
    std::vector<std::string> searchDocumentation(const std::string& query) const;
    std::string generateCodeExample(const std::string& topicConcept, const std::string& language = "cpp") const;
    std::string provideAPIReference(const std::string& apiName) const;
    
    // Technical assistance
    std::string diagnoseIssue(const std::string& errorMessage, const std::string& context = "") const;
    std::vector<std::string> suggestSolutions(const std::string& problemDescription) const;
    std::string generateTutorial(const std::string& topic, const std::string& difficulty = "beginner") const;
    
    // Code review and quality
    std::string reviewCode(const std::string& code, const std::string& language = "cpp") const;
    std::vector<std::string> identifyBestPractices(const std::string& code, const std::string& language = "cpp") const;
    std::string suggestRefactoring(const std::string& code, const std::string& language = "cpp") const;
    
    // Knowledge base management
    void addTechnicalKnowledge(const std::string& topic, const std::string& content, const std::vector<std::string>& tags = {});
    void updateKnowledgeBase(const std::string& topic, const std::string& updatedContent);
    std::string retrieveKnowledge(const std::string& topic) const;
    std::vector<std::string> getRelatedTopics(const std::string& topic) const;
    
    // Developer onboarding
    std::string generateOnboardingGuide(const std::string& project, const std::string& role = "developer") const;
    std::vector<std::string> createLearningPath(const std::string& goal, const std::string& currentLevel = "beginner") const;
    void trackDeveloperProgress(const UUID& developerId, const std::string& milestone);
    
    // Community engagement
    void hostTechnicalSession(const std::string& topic, const std::string& channelId, Timestamp scheduledTime);
    void answerTechnicalQuestion(const std::string& question, const std::string& channelId, const std::string& userId);
    void shareWeeklyTechUpdates(const std::vector<std::string>& channelIds);

private:
    void processLoop() override;
    void processQuestion(const std::string& question, const std::string& userId, const std::string& channelId);
    void updateTechnicalKnowledge();
    std::string formatCodeForPlatform(const std::string& code, PlatformType platform) const;
    bool isCodeRelated(const std::string& message) const;
    
    struct DocumentationEntry {
        std::string path;
        std::string content;
        std::string version;
        std::vector<std::string> tags;
        Timestamp lastUpdated;
    };
    
    struct KnowledgeEntry {
        std::string topic;
        std::string content;
        std::vector<std::string> tags;
        std::vector<std::string> relatedTopics;
        Timestamp lastUpdated;
        double relevanceScore;
    };
    
    std::vector<DocumentationEntry> documentationIndex_;
    std::unordered_map<std::string, KnowledgeEntry> knowledgeBase_;
    std::unordered_map<UUID, std::vector<std::string>> developerProgress_;
    std::thread processingThread_;
    
    mutable std::mutex docMutex_;
    mutable std::mutex knowledgeMutex_;
    mutable std::mutex progressMutex_;
};

/**
 * Ruby - Community Liaison Agent
 * Facilitates cross-community knowledge sharing and identifies synergies
 */
class CommunityLiaisonAgent : public TheOrgAgent {
public:
    CommunityLiaisonAgent(const AgentConfig& config);
    
    // TheOrgAgent interface implementation
    void initialize() override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override;

    // Organization monitoring
    void addOrganization(const OrganizationConfig& org);
    void removeOrganization(const UUID& orgId);
    void updateOrganizationTopics(const UUID& orgId, const std::vector<std::string>& topics);
    std::vector<OrganizationConfig> getMonitoredOrganizations() const;
    
    // Cross-organizational intelligence
    void trackDiscussion(const UUID& orgId, const std::string& topic, const std::string& summary, const std::string& channelId);
    std::vector<ParallelTopic> identifyParallelTopics(std::chrono::hours timeWindow = std::chrono::hours(168)) const; // 1 week
    std::vector<std::string> findCollaborationOpportunities(const std::vector<UUID>& orgIds) const;
    double calculateTopicRelevance(const std::string& topic, const std::vector<UUID>& orgIds) const;
    
    // Report generation
    CrossOrgReport generateDailyReport(const std::vector<UUID>& recipientOrgIds = {}) const;
    CrossOrgReport generateWeeklyReport(const std::vector<UUID>& recipientOrgIds = {}) const;
    CrossOrgReport generateTopicSpecificReport(const std::string& topic, const std::vector<UUID>& recipientOrgIds = {}) const;
    void distributeReport(const CrossOrgReport& report);
    
    // Knowledge sharing
    void shareKnowledge(const UUID& sourceOrgId, const UUID& targetOrgId, const std::string& topic, const std::string& content);
    void facilitateIntroduction(const UUID& org1Id, const UUID& org2Id, const std::string& sharedInterest);
    std::vector<std::string> suggestKnowledgeExchange(const UUID& orgId) const;
    
    // Topic analysis and trending
    std::vector<std::string> getTrendingTopics(std::chrono::hours timeWindow = std::chrono::hours(24)) const;
    std::unordered_map<std::string, double> analyzeTopicSentiment(const std::string& topic) const;
    std::vector<std::string> predictEmergingTopics() const;
    
    // Relationship mapping
    void mapOrganizationRelationship(const UUID& org1Id, const UUID& org2Id, const std::string& relationshipType);
    std::vector<UUID> getRelatedOrganizations(const UUID& orgId) const;
    double calculateOrganizationSimilarity(const UUID& org1Id, const UUID& org2Id) const;

private:
    void processLoop() override;
    void monitorOrganizations();
    void analyzeCrossOrgPatterns();
    void generatePeriodicReports();
    std::string formatReportForPlatform(const CrossOrgReport& report, PlatformType platform) const;
    void updateTopicTrends();
    
    struct DiscussionEntry {
        UUID orgId;
        std::string topic;
        std::string summary;
        std::string channelId;
        Timestamp timestamp;
        std::vector<std::string> participants;
        double engagementLevel;
    };
    
    struct TopicTrend {
        std::string topic;
        std::vector<UUID> activeOrganizations;
        double trendScore;
        std::chrono::hours duration;
        Timestamp firstSeen;
        Timestamp lastSeen;
    };
    
    std::unordered_map<UUID, OrganizationConfig> organizations_;
    std::vector<DiscussionEntry> discussionHistory_;
    std::vector<TopicTrend> topicTrends_;
    std::unordered_map<std::string, std::unordered_map<UUID, double>> topicOrgRelevance_;
    std::thread processingThread_;
    
    mutable std::mutex orgMutex_;
    mutable std::mutex discussionMutex_;
    mutable std::mutex trendMutex_;
};

/**
 * Jimmy - Project Manager Agent
 * Coordinates projects, tracks progress, and manages team check-ins
 */
class ProjectManagerAgent : public TheOrgAgent {
public:
    ProjectManagerAgent(const AgentConfig& config);
    
    // TheOrgAgent interface implementation
    void initialize() override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override;

    // Project management
    UUID createProject(const std::string& name, const std::string& description, const std::vector<UUID>& teamMemberIds = {});
    void updateProject(const UUID& projectId, const Project& updatedProject);
    void addTeamMemberToProject(const UUID& projectId, const UUID& teamMemberId);
    void removeTeamMemberFromProject(const UUID& projectId, const UUID& teamMemberId);
    std::vector<Project> getActiveProjects() const;
    std::optional<Project> getProject(const UUID& projectId) const;
    
    // Team member management
    UUID addTeamMember(const TeamMember& member);
    void updateTeamMember(const UUID& memberId, const TeamMember& updatedMember);
    void removeTeamMember(const UUID& memberId);
    std::optional<TeamMember> getTeamMember(const UUID& memberId) const;
    std::vector<TeamMember> getAllTeamMembers() const;
    
    // Daily check-ins and updates
    void scheduleDailyCheckins(const UUID& projectId);
    void sendCheckinReminder(const UUID& teamMemberId, const UUID& projectId);
    void recordDailyUpdate(const DailyUpdate& update);
    std::vector<DailyUpdate> getDailyUpdates(const UUID& projectId, const std::string& date = "") const;
    std::vector<DailyUpdate> getMemberUpdates(const UUID& teamMemberId, std::chrono::hours timeWindow = std::chrono::hours(168)) const;
    
    // Reporting and analytics
    std::string generateProjectStatusReport(const UUID& projectId) const;
    std::string generateTeamProductivityReport(const std::vector<UUID>& teamMemberIds, std::chrono::hours timeWindow = std::chrono::hours(168)) const;
    std::string generateWeeklyReport(const std::vector<UUID>& projectIds = {}) const;
    void distributeReport(const std::string& report, const std::vector<std::string>& channelIds);
    
    // Task integration
    void linkTaskToProject(const UUID& projectId, const UUID& taskId);
    void assignTaskToMember(const UUID& taskId, const UUID& teamMemberId);
    std::vector<UUID> getProjectTasks(const UUID& projectId) const;
    std::vector<UUID> getMemberTasks(const UUID& teamMemberId) const;
    
    // Time tracking and availability
    bool isTeamMemberAvailable(const UUID& teamMemberId, Timestamp time) const;
    std::vector<Timestamp> findTeamMeetingTime(const std::vector<UUID>& teamMemberIds, std::chrono::minutes duration) const;
    void trackWorkHours(const UUID& teamMemberId, const UUID& projectId, std::chrono::minutes duration);
    
    // Blocker and risk management
    void reportBlocker(const UUID& projectId, const UUID& teamMemberId, const std::string& description);
    void resolveBlocker(const UUID& blockerId, const std::string& resolution);
    std::vector<std::string> getActiveBlockers(const UUID& projectId) const;
    void assessProjectRisk(const UUID& projectId);

private:
    void processLoop() override;
    void sendDailyCheckins();
    void processCheckinResponses();
    void generateAutomaticReports();
    void monitorProjectHealth();
    std::string formatMemberAvailability(const TeamMemberAvailability& availability) const;
    bool isInWorkingHours(const UUID& teamMemberId, Timestamp time) const;
    
    struct Blocker {
        UUID id;
        UUID projectId;
        UUID reportedBy;
        std::string description;
        Timestamp reportedAt;
        std::optional<std::string> resolution;
        std::optional<Timestamp> resolvedAt;
        bool isActive;
    };
    
    struct ProjectMetrics {
        UUID projectId;
        double completionPercentage;
        std::chrono::hours totalTimeSpent;
        size_t activeBlockers;
        double teamProductivity;
        Timestamp lastUpdate;
    };
    
    std::unordered_map<UUID, Project> projects_;
    std::unordered_map<UUID, TeamMember> teamMembers_;
    std::vector<DailyUpdate> dailyUpdates_;
    std::vector<Blocker> blockers_;
    std::unordered_map<UUID, ProjectMetrics> projectMetrics_;
    std::unordered_map<UUID, std::vector<std::pair<UUID, std::chrono::minutes>>> workHours_; // teamMemberId -> [(projectId, hours)]
    std::thread processingThread_;
    
    mutable std::mutex projectMutex_;
    mutable std::mutex teamMutex_;
    mutable std::mutex updateMutex_;
    mutable std::mutex blockerMutex_;
    mutable std::mutex metricsMutex_;
};

/**
 * Laura - Social Media Manager Agent
 * Crafts and publishes content across social media platforms
 */
class SocialMediaManagerAgent : public TheOrgAgent {
public:
    SocialMediaManagerAgent(const AgentConfig& config);
    
    // TheOrgAgent interface implementation
    void initialize() override;
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override;

    // Content creation and management
    UUID createContent(ContentType type, const std::string& title, const std::string& content, 
                      const std::vector<PlatformType>& targetPlatforms);
    void updateContent(const UUID& contentId, const SocialMediaContent& updatedContent);
    void deleteContent(const UUID& contentId);
    std::optional<SocialMediaContent> getContent(const UUID& contentId) const;
    std::vector<SocialMediaContent> getContentByStatus(ContentStatus status) const;
    
    // Content scheduling and publishing
    void scheduleContent(const UUID& contentId, Timestamp publishTime);
    void publishContent(const UUID& contentId);
    void publishContentToPlatform(const UUID& contentId, PlatformType platform);
    std::vector<UUID> getScheduledContent(std::chrono::hours timeWindow = std::chrono::hours(24)) const;
    
    // Content generation and AI assistance
    std::string generateContent(const std::string& topic, ContentType type, PlatformType platform, 
                               const std::string& tone = "professional") const;
    std::vector<std::string> suggestHashtags(const std::string& content, PlatformType platform) const;
    std::string optimizeContentForPlatform(const std::string& content, PlatformType platform) const;
    std::string generateCaption(const std::string& imageDescription, PlatformType platform) const;
    
    // Content calendar and strategy
    void createContentCalendar(const std::vector<std::string>& topics, std::chrono::hours planningWindow = std::chrono::hours(168));
    std::vector<std::string> suggestContentTopics(PlatformType platform) const;
    void setPostingSchedule(PlatformType platform, const std::vector<std::string>& postingTimes);
    std::string analyzeContentPerformance(std::chrono::hours timeWindow = std::chrono::hours(168)) const;
    
    // Engagement and interaction
    void monitorMentions(PlatformType platform);
    void respondToComment(const std::string& commentId, const std::string& response, PlatformType platform);
    void likePost(const std::string& postId, PlatformType platform);
    void sharePost(const std::string& postId, const std::string& comment, PlatformType platform);
    std::vector<std::string> getRecentMentions(PlatformType platform, std::chrono::hours timeWindow = std::chrono::hours(24)) const;
    
    // Analytics and insights
    struct SocialMediaMetrics {
        PlatformType platform;
        size_t followers;
        size_t totalPosts;
        double engagementRate;
        size_t impressions;
        size_t clicks;
        size_t shares;
        Timestamp lastUpdated;
    };
    
    SocialMediaMetrics getPlatformMetrics(PlatformType platform) const;
    std::string generateAnalyticsReport(std::chrono::hours timeWindow = std::chrono::hours(168)) const;
    std::vector<std::string> getBestPerformingContent(PlatformType platform, size_t count = 10) const;
    std::vector<std::string> getOptimalPostingTimes(PlatformType platform) const;
    
    // Campaign management
    UUID createCampaign(const std::string& name, const std::string& description, 
                       const std::vector<PlatformType>& platforms, Timestamp startDate, Timestamp endDate);
    void addContentToCampaign(const UUID& campaignId, const UUID& contentId);
    void launchCampaign(const UUID& campaignId);
    std::string analyzeCampaignPerformance(const UUID& campaignId) const;

private:
    void processLoop() override;
    void publishScheduledContent();
    void monitorEngagement();
    void updateMetrics();
    void generateContentSuggestions();
    std::string formatContentForPlatform(const SocialMediaContent& content, PlatformType platform) const;
    bool isOptimalPostingTime(PlatformType platform) const;
    
    struct Campaign {
        UUID id;
        std::string name;
        std::string description;
        std::vector<PlatformType> platforms;
        std::vector<UUID> contentIds;
        Timestamp startDate;
        Timestamp endDate;
        bool isActive;
        std::unordered_map<std::string, std::string> metrics;
    };
    
    struct ContentTemplate {
        std::string templateName;
        ContentType type;
        std::string template_;
        std::vector<PlatformType> supportedPlatforms;
        std::vector<std::string> placeholders;
    };
    
    std::unordered_map<UUID, SocialMediaContent> content_;
    std::unordered_map<UUID, Campaign> campaigns_;
    std::vector<ContentTemplate> templates_;
    std::unordered_map<PlatformType, SocialMediaMetrics> platformMetrics_;
    std::unordered_map<PlatformType, std::vector<std::string>> postingSchedules_;
    std::thread processingThread_;
    
    mutable std::mutex contentMutex_;
    mutable std::mutex campaignMutex_;
    mutable std::mutex metricsMutex_;
    mutable std::mutex scheduleMutex_;
};

/**
 * TheOrgManager - Central management system for coordinating all agents
 */
class TheOrgManager {
public:
    TheOrgManager();
    ~TheOrgManager();

    // Agent lifecycle management
    void initializeAllAgents(const std::vector<AgentConfig>& configs);
    void startAllAgents();
    void stopAllAgents();
    void pauseAllAgents();
    void resumeAllAgents();
    
    // Individual agent management
    void addAgent(std::shared_ptr<TheOrgAgent> agent);
    void removeAgent(const UUID& agentId);
    std::shared_ptr<TheOrgAgent> getAgent(const UUID& agentId) const;
    std::shared_ptr<TheOrgAgent> getAgentByRole(AgentRole role) const;
    std::vector<std::shared_ptr<TheOrgAgent>> getAllAgents() const;
    
    // Inter-agent communication
    void broadcastMessage(const std::string& message, const std::string& senderId, 
                         const std::vector<AgentRole>& targetRoles = {});
    void sendDirectMessage(const UUID& fromAgentId, const UUID& toAgentId, const std::string& message);
    void subscribeToEvents(const UUID& agentId, const std::vector<std::string>& eventTypes);
    void publishEvent(const std::string& eventType, const std::string& data, const UUID& sourceAgentId);
    
    // Global configuration and settings
    void loadConfiguration(const std::string& configPath);
    void saveConfiguration(const std::string& configPath) const;
    void updateGlobalSetting(const std::string& key, const std::string& value);
    std::string getGlobalSetting(const std::string& key) const;
    
    // Platform integration coordination
    void addGlobalPlatform(const PlatformConfig& platform);
    void removeGlobalPlatform(PlatformType type);
    void propagatePlatformToAgents(PlatformType type, const std::vector<AgentRole>& targetRoles);
    
    // Cross-agent task orchestration
    UUID createCrossAgentWorkflow(const std::string& name, const std::vector<AgentRole>& involvedRoles);
    void executeWorkflow(const UUID& workflowId, const std::unordered_map<std::string, std::string>& parameters);
    void monitorWorkflows();
    
    // System health and monitoring
    struct SystemMetrics {
        size_t totalAgents;
        size_t activeAgents;
        size_t totalTasks;
        size_t pendingTasks;
        double systemLoad;
        std::chrono::milliseconds averageResponseTime;
        Timestamp lastUpdated;
    };
    
    SystemMetrics getSystemMetrics() const;
    std::string generateHealthReport() const;
    void performHealthCheck();
    
    // Data persistence and backup
    void saveSystemState(const std::string& backupPath) const;
    void loadSystemState(const std::string& backupPath);
    void scheduleAutoBackup(std::chrono::minutes interval);
    
    // Event handling and logging
    void enableEventLogging(const std::string& logPath);
    void disableEventLogging();
    std::vector<std::string> getRecentEvents(std::chrono::hours timeWindow = std::chrono::hours(24)) const;
    void setLogLevel(const std::string& level);

private:
    void coordinationLoop();
    void processInterAgentMessages();
    void monitorAgentHealth();
    void executeScheduledTasks();
    void updateSystemMetrics();
    
    struct Workflow {
        UUID id;
        std::string name;
        std::vector<AgentRole> involvedRoles;
        std::vector<UUID> taskIds;
        std::unordered_map<std::string, std::string> parameters;
        bool isActive;
        Timestamp createdAt;
        Timestamp lastExecuted;
    };
    
    struct EventSubscription {
        UUID agentId;
        std::vector<std::string> eventTypes;
    };
    
    std::unordered_map<UUID, std::shared_ptr<TheOrgAgent>> agents_;
    std::unordered_map<AgentRole, UUID> roleToAgentMap_;
    std::unordered_map<PlatformType, PlatformConfig> globalPlatforms_;
    std::unordered_map<UUID, Workflow> workflows_;
    std::vector<EventSubscription> eventSubscriptions_;
    std::unordered_map<std::string, std::string> globalSettings_;
    
    std::atomic<bool> running_{false};
    std::thread coordinationThread_;
    SystemMetrics currentMetrics_;
    
    mutable std::mutex agentMutex_;
    mutable std::mutex platformMutex_;
    mutable std::mutex workflowMutex_;
    mutable std::mutex settingsMutex_;
    mutable std::mutex metricsMutex_;
    mutable std::mutex eventMutex_;
    
    // Event logging
    bool eventLoggingEnabled_ = false;
    std::string logPath_;
    std::string logLevel_ = "INFO";
    std::vector<std::string> eventLog_;
    mutable std::mutex logMutex_;
};

// Utility functions for the_org system
namespace the_org_utils {
    std::string formatTimestamp(Timestamp timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S");
    std::string generateAgentId(AgentRole role);
    std::vector<std::string> parseHashtags(const std::string& content);
    bool isWorkingDay(const std::string& day);
    Timestamp parseTimeString(const std::string& timeStr, const std::string& format = "%H:%M");
    std::string platformTypeToString(PlatformType type);
    PlatformType stringToPlatformType(const std::string& str);
    std::string agentRoleToString(AgentRole role);
    AgentRole stringToAgentRole(const std::string& str);
    double calculateSimilarity(const std::vector<std::string>& list1, const std::vector<std::string>& list2);
    std::string sanitizeForPlatform(const std::string& content, PlatformType platform);
    bool validateUrl(const std::string& url);
    std::string extractDomain(const std::string& url);
    std::vector<std::string> splitText(const std::string& text, size_t maxLength, const std::string& delimiter = " ");
    std::string joinText(const std::vector<std::string>& parts, const std::string& separator = " ");
}

} // namespace elizaos