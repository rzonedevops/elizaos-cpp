#include "elizaos/the_org.hpp"
#include "elizaos/agentlogger.hpp"
#include <algorithm>
#include <random>
#include <sstream>
#include <regex>
#include <iomanip>
#include <fstream>
#include <future>
#include <set>

// Simple logging helper to replace Logger calls
#define LOG_INFO(component, message) do { \
    elizaos::AgentLogger logger; \
    logger.log(message, component, "Info", elizaos::LogLevel::INFO); \
} while(0)

#define LOG_WARNING(component, message) do { \
    elizaos::AgentLogger logger; \
    logger.log(message, component, "Warning", elizaos::LogLevel::WARNING); \
} while(0)

#define LOG_ERROR(component, message) do { \
    elizaos::AgentLogger logger; \
    logger.log(message, component, "Error", elizaos::LogLevel::ERROR); \
} while(0)

namespace elizaos {

// ============================================================================
// TheOrgAgent Base Implementation
// ============================================================================

TheOrgAgent::TheOrgAgent(const AgentConfig& config, AgentRole role)
    : config_(config), role_(role), state_(config) {
    AgentLogger logger;
    logger.log("Initializing agent: " + config.agentName + " with role: " + the_org_utils::agentRoleToString(role), 
               "TheOrgAgent", "Agent Init", LogLevel::INFO);
}

std::shared_ptr<Memory> TheOrgAgent::createMemory(const std::string& content, MemoryType type) {
    std::lock_guard<std::mutex> lock(memoryMutex_);
    auto memory = std::make_shared<Memory>(generateUUID(), content, config_.agentId, config_.agentId);
    
    // Set metadata based on type
    switch (type) {
        case MemoryType::MESSAGE: {
            MessageMetadata metadata;
            metadata.timestamp = std::chrono::system_clock::now();
            memory->setMetadata(metadata);
            break;
        }
        case MemoryType::DOCUMENT: {
            DocumentMetadata metadata;
            metadata.timestamp = std::chrono::system_clock::now();
            memory->setMetadata(metadata);
            break;
        }
        default: {
            CustomMetadata metadata;
            metadata.timestamp = std::chrono::system_clock::now();
            memory->setMetadata(metadata);
            break;
        }
    }
    
    return memory;
}

void TheOrgAgent::addMemory(std::shared_ptr<Memory> memory) {
    std::lock_guard<std::mutex> lock(memoryMutex_);
    memoryStore_.push_back(memory);
    
    // Keep only recent memories (last 1000)
    if (memoryStore_.size() > 1000) {
        memoryStore_.erase(memoryStore_.begin());
    }
}

std::vector<std::shared_ptr<Memory>> TheOrgAgent::searchMemories(const std::string& query, size_t maxResults) {
    std::lock_guard<std::mutex> lock(memoryMutex_);
    std::vector<std::shared_ptr<Memory>> results;
    
    // Simple text-based search (in production would use embeddings)
    for (const auto& memory : memoryStore_) {
        if (memory->getContent().find(query) != std::string::npos) {
            results.push_back(memory);
            if (results.size() >= maxResults) break;
        }
    }
    
    return results;
}

void TheOrgAgent::addPlatform(const PlatformConfig& platform) {
    std::lock_guard<std::mutex> lock(platformMutex_);
    platforms_[platform.type] = platform;
    
    AgentLogger logger;
    logger.log("Added platform: " + the_org_utils::platformTypeToString(platform.type), 
               "TheOrgAgent", "Platform", LogLevel::INFO);
}

void TheOrgAgent::removePlatform(PlatformType type) {
    std::lock_guard<std::mutex> lock(platformMutex_);
    platforms_.erase(type);
    
    
    LOG_INFO("TheOrgAgent", "Removed platform: " + the_org_utils::platformTypeToString(type));
}

bool TheOrgAgent::sendMessage(PlatformType platform, const std::string& channelId, const std::string& message) {
    std::lock_guard<std::mutex> lock(platformMutex_);
    
    auto it = platforms_.find(platform);
    if (it == platforms_.end()) {
        
        LOG_ERROR("TheOrgAgent", "Platform not configured: " + the_org_utils::platformTypeToString(platform));
        return false;
    }
    
    // Format message for platform
    std::string formattedMessage = formatResponse(message, platform);
    
    // In production, this would integrate with actual platform APIs
    
    LOG_INFO("TheOrgAgent", "Sending message to " + the_org_utils::platformTypeToString(platform) + 
                                " channel " + channelId + ": " + formattedMessage);
    
    // Create memory of sent message
    auto memory = createMemory("Sent: " + formattedMessage, MemoryType::MESSAGE);
    addMemory(memory);
    
    return true;
}

std::vector<std::string> TheOrgAgent::getRecentMessages([[maybe_unused]] PlatformType platform, const std::string& channelId, size_t count) {
    // In production, this would fetch from platform APIs
    std::vector<std::string> messages;
    
    // Simulate some recent messages
    for (size_t i = 0; i < count && i < 10; ++i) {
        messages.push_back("Sample message " + std::to_string(i + 1) + " from " + channelId);
    }
    
    return messages;
}

void TheOrgAgent::sendToAgent(const UUID& agentId, const std::string& message, const std::string& type) {
    // This would be handled by TheOrgManager in practice
    
    LOG_INFO("TheOrgAgent", "Sending inter-agent message to " + agentId + " (type: " + type + "): " + message);
}

std::queue<std::string> TheOrgAgent::getIncomingMessages() {
    std::lock_guard<std::mutex> lock(messageMutex_);
    return incomingMessages_;
}

void TheOrgAgent::processMessage(const std::string& message, const std::string& senderId) {
    std::lock_guard<std::mutex> lock(messageMutex_);
    incomingMessages_.push("From " + senderId + ": " + message);
    
    
    LOG_INFO("TheOrgAgent", "Received message from " + senderId + ": " + message);
}

UUID TheOrgAgent::createTask(const std::string& name, [[maybe_unused]] const std::string& description, [[maybe_unused]] int priority) {
    UUID taskId = generateUUID();
    
    
    LOG_INFO("TheOrgAgent", "Created task: " + name + " (ID: " + taskId + ")");
    
    return taskId;
}

bool TheOrgAgent::completeTask(const UUID& taskId) {
    
    LOG_INFO("TheOrgAgent", "Completed task: " + taskId);
    return true;
}

std::vector<std::shared_ptr<Task>> TheOrgAgent::getPendingTasks() {
    // Would integrate with TaskManager in practice
    return {};
}

void TheOrgAgent::updateConfig(const std::unordered_map<std::string, std::string>& settings) {
    std::lock_guard<std::mutex> lock(settingsMutex_);
    for (const auto& [key, value] : settings) {
        settings_[key] = value;
    }
}

std::string TheOrgAgent::getConfigValue(const std::string& key) const {
    std::lock_guard<std::mutex> lock(settingsMutex_);
    auto it = settings_.find(key);
    return (it != settings_.end()) ? it->second : "";
}

bool TheOrgAgent::validateMessage(const std::string& message) const {
    return !message.empty() && message.length() <= 2000; // Basic validation
}

std::string TheOrgAgent::formatResponse(const std::string& response, PlatformType platform) const {
    return the_org_utils::sanitizeForPlatform(response, platform);
}

// ============================================================================
// CommunityManagerAgent Implementation
// ============================================================================

CommunityManagerAgent::CommunityManagerAgent(const AgentConfig& config)
    : TheOrgAgent(config, AgentRole::COMMUNITY_MANAGER) {
    currentMetrics_ = CommunityMetrics{};
    currentMetrics_.lastUpdated = std::chrono::system_clock::now();
}

void CommunityManagerAgent::initialize() {
    
    LOG_INFO("CommunityManager", "Initializing Eli5 Community Manager Agent");
    
    // Set default moderation rules
    addModerationRule("spam", ModerationAction::WARNING, "Spam content detected");
    addModerationRule("toxic", ModerationAction::TIMEOUT, "Toxic behavior");
    addModerationRule("harassment", ModerationAction::BAN, "Harassment is not tolerated");
}

void CommunityManagerAgent::start() {
    running_ = true;
    processingThread_ = std::thread(&CommunityManagerAgent::processLoop, this);
    
    
    LOG_INFO("CommunityManager", "Started Eli5 Community Manager Agent");
}

void CommunityManagerAgent::stop() {
    running_ = false;
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
    
    
    LOG_INFO("CommunityManager", "Stopped Eli5 Community Manager Agent");
}

void CommunityManagerAgent::pause() {
    paused_ = true;
    
    LOG_INFO("CommunityManager", "Paused Eli5 Community Manager Agent");
}

void CommunityManagerAgent::resume() {
    paused_ = false;
    
    LOG_INFO("CommunityManager", "Resumed Eli5 Community Manager Agent");
}

bool CommunityManagerAgent::isRunning() const {
    return running_;
}

void CommunityManagerAgent::enableNewUserGreeting(const std::string& channelId, const std::string& greetingMessage) {
    greetingEnabled_ = true;
    greetingChannelId_ = channelId;
    customGreetingMessage_ = greetingMessage;
    
    
    LOG_INFO("CommunityManager", "Enabled new user greeting in channel: " + channelId);
}

void CommunityManagerAgent::disableNewUserGreeting() {
    greetingEnabled_ = false;
    greetingChannelId_.clear();
    customGreetingMessage_.clear();
    
    
    LOG_INFO("CommunityManager", "Disabled new user greeting");
}

bool CommunityManagerAgent::shouldGreetNewUser(const std::string& userId) const {
    return greetingEnabled_ && !userId.empty();
}

std::string CommunityManagerAgent::generateGreeting(const std::string& userName, const std::string& serverName) const {
    if (!customGreetingMessage_.empty()) {
        // Replace placeholders
        std::string greeting = customGreetingMessage_;
        size_t pos = greeting.find("{user}");
        if (pos != std::string::npos) {
            greeting.replace(pos, 6, userName);
        }
        pos = greeting.find("{server}");
        if (pos != std::string::npos) {
            greeting.replace(pos, 8, serverName);
        }
        return greeting;
    }
    
    // Default greeting
    return "Welcome to the community, " + userName + "! 👋 Feel free to introduce yourself and let us know if you have any questions.";
}

void CommunityManagerAgent::addModerationRule(const std::string& rule, ModerationAction action, const std::string& reason) {
    std::lock_guard<std::mutex> lock(rulesMutex_);
    moderationRules_[rule] = {action, reason};
    
    
    LOG_INFO("CommunityManager", "Added moderation rule: " + rule);
}

void CommunityManagerAgent::removeModerationRule(const std::string& rule) {
    std::lock_guard<std::mutex> lock(rulesMutex_);
    moderationRules_.erase(rule);
    
    
    LOG_INFO("CommunityManager", "Removed moderation rule: " + rule);
}

bool CommunityManagerAgent::evaluateMessage(const std::string& message, const std::string& userId, [[maybe_unused]] const std::string& channelId) {
    std::lock_guard<std::mutex> lock(rulesMutex_);
    
    // Convert to lowercase for comparison
    std::string lowerMessage = message;
    std::transform(lowerMessage.begin(), lowerMessage.end(), lowerMessage.begin(), ::tolower);
    
    for (const auto& [rule, actionAndReason] : moderationRules_) {
        if (lowerMessage.find(rule) != std::string::npos) {
            
            LOG_WARNING("CommunityManager", "Moderation rule triggered: " + rule + " by user: " + userId);
            
            applyModerationAction(userId, actionAndReason.first, actionAndReason.second);
            return false; // Message violates rules
        }
    }
    
    return true; // Message is acceptable
}

void CommunityManagerAgent::applyModerationAction(const std::string& userId, ModerationAction action, 
                                                 const std::string& reason, std::optional<std::chrono::seconds> duration) {
    ModerationEvent event;
    event.id = generateUUID();
    event.userId = userId;
    event.moderatorId = config_.agentId;
    event.action = action;
    event.reason = reason;
    event.duration = duration;
    event.timestamp = std::chrono::system_clock::now();
    
    moderationHistory_.push_back(event);
    
    
    std::string actionStr;
    switch (action) {
        case ModerationAction::WARNING: actionStr = "WARNING"; break;
        case ModerationAction::TIMEOUT: actionStr = "TIMEOUT"; break;
        case ModerationAction::KICK: actionStr = "KICK"; break;
        case ModerationAction::BAN: actionStr = "BAN"; break;
        default: actionStr = "UNKNOWN"; break;
    }
    
    LOG_WARNING("CommunityManager", "Applied " + actionStr + " to user " + userId + ": " + reason);
}

CommunityMetrics CommunityManagerAgent::generateCommunityMetrics() const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    return currentMetrics_;
}

void CommunityManagerAgent::trackUserActivity(const std::string& userId, [[maybe_unused]] const std::string& activity) {
    std::lock_guard<std::mutex> lock(activityMutex_);
    userActivity_[userId].push_back(std::chrono::system_clock::now());
    
    // Keep only recent activity (last 7 days)
    auto cutoff = std::chrono::system_clock::now() - std::chrono::hours(168);
    auto& activities = userActivity_[userId];
    activities.erase(std::remove_if(activities.begin(), activities.end(),
                                   [cutoff](const Timestamp& ts) { return ts < cutoff; }),
                     activities.end());
}

std::vector<std::string> CommunityManagerAgent::identifyActiveUsers(std::chrono::hours timeWindow) const {
    std::lock_guard<std::mutex> lock(activityMutex_);
    std::vector<std::string> activeUsers;
    
    auto cutoff = std::chrono::system_clock::now() - timeWindow;
    
    for (const auto& [userId, activities] : userActivity_) {
        for (const auto& timestamp : activities) {
            if (timestamp >= cutoff) {
                activeUsers.push_back(userId);
                break;
            }
        }
    }
    
    return activeUsers;
}

std::vector<std::string> CommunityManagerAgent::getTopTopics([[maybe_unused]] std::chrono::hours timeWindow) const {
    // In production, this would analyze message content
    return {"elizaos development", "agent framework", "community building", "AI agents", "typescript integration"};
}

void CommunityManagerAgent::scheduleEvent(const std::string& eventName, [[maybe_unused]] const std::string& description, Timestamp scheduledTime) {
    
    LOG_INFO("CommunityManager", "Scheduled community event: " + eventName + " at " + 
                 the_org_utils::formatTimestamp(scheduledTime));
}

void CommunityManagerAgent::announceEvent([[maybe_unused]] const std::string& eventId, const std::vector<std::string>& channelIds) {
    for (const auto& channelId : channelIds) {
        sendMessage(PlatformType::DISCORD, channelId, "📅 Upcoming community event! Check the details in the events channel.");
    }
}

void CommunityManagerAgent::processLoop() {
    
    LOG_INFO("CommunityManager", "Started processing loop");
    
    while (running_) {
        if (!paused_) {
            // Process incoming messages
            auto messages = getIncomingMessages();
            while (!messages.empty()) {
                // Process message for greeting, moderation, etc.
                messages.pop();
            }
            
            // Update metrics periodically
            updateCommunityMetrics();
            
            // Generate daily report if needed
            auto now = std::chrono::system_clock::now();
            static auto lastReport = now;
            if (now - lastReport > std::chrono::hours(24)) {
                generateDailyReport();
                lastReport = now;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void CommunityManagerAgent::generateDailyReport() {
    auto metrics = generateCommunityMetrics();
    auto activeUsers = identifyActiveUsers(std::chrono::hours(24));
    
    std::stringstream report;
    report << "📊 **Daily Community Report**\n\n";
    report << "**Active Members:** " << activeUsers.size() << "\n";
    report << "**Total Members:** " << metrics.totalMembers << "\n";
    report << "**New Members Today:** " << metrics.newMembersToday << "\n";
    report << "**Messages Per Day:** " << metrics.messagesPerDay << "\n";
    report << "**Engagement Rate:** " << std::fixed << std::setprecision(2) << (metrics.engagementRate * 100) << "%\n\n";
    
    auto topTopics = getTopTopics(std::chrono::hours(24));
    if (!topTopics.empty()) {
        report << "**Top Discussion Topics:**\n";
        for (size_t i = 0; i < std::min(topTopics.size(), size_t(5)); ++i) {
            report << (i + 1) << ". " << topTopics[i] << "\n";
        }
    }
    
    
    LOG_INFO("CommunityManager", "Generated daily report: " + report.str());
}

void CommunityManagerAgent::updateCommunityMetrics() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    // Simulate metrics update (in production would pull from platform APIs)
    currentMetrics_.activeMembers = identifyActiveUsers(std::chrono::hours(24)).size();
    currentMetrics_.messagesPerDay = 150; // Simulated
    currentMetrics_.engagementRate = 0.15; // Simulated
    currentMetrics_.lastUpdated = std::chrono::system_clock::now();
}

// ============================================================================
// DeveloperRelationsAgent Implementation
// ============================================================================

DeveloperRelationsAgent::DeveloperRelationsAgent(const AgentConfig& config)
    : TheOrgAgent(config, AgentRole::DEVELOPER_RELATIONS) {
}

void DeveloperRelationsAgent::initialize() {
    
    LOG_INFO("DeveloperRelations", "Initializing Eddy Developer Relations Agent");
    
    // Initialize knowledge base with basic topics
    addTechnicalKnowledge("elizaos-core", "Core agent framework with State, Memory, and Action systems", {"core", "framework", "agents"});
    addTechnicalKnowledge("typescript-integration", "ElizaOS supports TypeScript plugins and character definitions", {"typescript", "plugins", "integration"});
    addTechnicalKnowledge("agent-memory", "Persistent memory system with embedding-based retrieval", {"memory", "embeddings", "persistence"});
}

void DeveloperRelationsAgent::start() {
    running_ = true;
    processingThread_ = std::thread(&DeveloperRelationsAgent::processLoop, this);
    
    
    LOG_INFO("DeveloperRelations", "Started Eddy Developer Relations Agent");
}

void DeveloperRelationsAgent::stop() {
    running_ = false;
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
    
    
    LOG_INFO("DeveloperRelations", "Stopped Eddy Developer Relations Agent");
}

void DeveloperRelationsAgent::pause() {
    paused_ = true;
}

void DeveloperRelationsAgent::resume() {
    paused_ = false;
}

bool DeveloperRelationsAgent::isRunning() const {
    return running_;
}

void DeveloperRelationsAgent::indexDocumentation(const std::string& docPath, const std::string& version) {
    std::lock_guard<std::mutex> lock(docMutex_);
    
    DocumentationEntry entry;
    entry.path = docPath;
    entry.version = version;
    entry.lastUpdated = std::chrono::system_clock::now();
    
    // In production, would read and parse actual documentation files
    entry.content = "Documentation content for " + docPath;
    entry.tags = {"documentation", "reference"};
    
    documentationIndex_.push_back(entry);
    
    
    LOG_INFO("DeveloperRelations", "Indexed documentation: " + docPath + " (version: " + version + ")");
}

std::vector<std::string> DeveloperRelationsAgent::searchDocumentation(const std::string& query) const {
    std::lock_guard<std::mutex> lock(docMutex_);
    std::vector<std::string> results;
    
    for (const auto& doc : documentationIndex_) {
        if (doc.content.find(query) != std::string::npos || doc.path.find(query) != std::string::npos) {
            results.push_back(doc.path + " (v" + doc.version + ")");
        }
    }
    
    return results;
}

std::string DeveloperRelationsAgent::generateCodeExample(const std::string& topicConcept, const std::string& language) const {
    if (language == "cpp") {
        if (topicConcept == "agent-creation") {
            return R"(
// Creating a new ElizaOS agent in C++
#include "elizaos/core.hpp"

elizaos::AgentConfig config;
config.agentId = elizaos::generateUUID();
config.agentName = "MyAgent";
config.bio = "A helpful assistant agent";

elizaos::State state(config);
auto memory = std::make_shared<elizaos::Memory>(
    elizaos::generateUUID(), 
    "Initial memory", 
    config.agentId, 
    config.agentId
);
state.addRecentMessage(memory);
)";
        } else if (topicConcept == "memory-management") {
            return R"(
// Working with agent memory
auto memory = agent.createMemory("User asked about documentation", elizaos::MemoryType::MESSAGE);
agent.addMemory(memory);

// Search memories
auto relevantMemories = agent.searchMemories("documentation", 5);
for (const auto& mem : relevantMemories) {
    std::cout << "Found: " << mem->getContent() << std::endl;
}
)";
        }
    }
    
    return "// Code example for " + topicConcept + " in " + language + " not available yet";
}

void DeveloperRelationsAgent::addTechnicalKnowledge(const std::string& topic, const std::string& content, const std::vector<std::string>& tags) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    KnowledgeEntry entry;
    entry.topic = topic;
    entry.content = content;
    entry.tags = tags;
    entry.lastUpdated = std::chrono::system_clock::now();
    entry.relevanceScore = 1.0;
    
    knowledgeBase_[topic] = entry;
    
    
    LOG_INFO("DeveloperRelations", "Added technical knowledge: " + topic);
}

std::string DeveloperRelationsAgent::retrieveKnowledge(const std::string& topic) const {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    auto it = knowledgeBase_.find(topic);
    if (it != knowledgeBase_.end()) {
        return it->second.content;
    }
    
    // Search for partial matches
    for (const auto& [key, entry] : knowledgeBase_) {
        if (key.find(topic) != std::string::npos || entry.content.find(topic) != std::string::npos) {
            return entry.content;
        }
    }
    
    return "Knowledge about '" + topic + "' not found. Would you like me to research this topic?";
}

void DeveloperRelationsAgent::processLoop() {
    
    LOG_INFO("DeveloperRelations", "Started processing loop");
    
    while (running_) {
        if (!paused_) {
            // Process incoming questions
            auto messages = getIncomingMessages();
            while (!messages.empty()) {
                std::string message = messages.front();
                messages.pop();
                
                if (isCodeRelated(message)) {
                    // Process as technical question
                    processQuestion(message, "unknown_user", "unknown_channel");
                }
            }
            
            // Update technical knowledge periodically
            updateTechnicalKnowledge();
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void DeveloperRelationsAgent::processQuestion(const std::string& question, const std::string& userId, const std::string& channelId) {
    
    LOG_INFO("DeveloperRelations", "Processing technical question from " + userId + ": " + question);
    
    // Analyze question and provide response
    std::string response;
    if (question.find("documentation") != std::string::npos) {
        auto docs = searchDocumentation("core");
        if (!docs.empty()) {
            response = "Here are relevant documentation resources:\n";
            for (const auto& doc : docs) {
                response += "• " + doc + "\n";
            }
        }
    } else if (question.find("example") != std::string::npos || question.find("code") != std::string::npos) {
        response = "Here's a code example:\n```cpp\n" + generateCodeExample("agent-creation", "cpp") + "\n```";
    } else {
        response = "I'd be happy to help! Could you provide more specific details about what you're trying to accomplish?";
    }
    
    sendMessage(PlatformType::DISCORD, channelId, response);
}

bool DeveloperRelationsAgent::isCodeRelated(const std::string& message) const {
    std::vector<std::string> codeKeywords = {"code", "programming", "function", "class", "error", "bug", "implementation", "api", "documentation", "tutorial", "example"};
    
    std::string lowerMessage = message;
    std::transform(lowerMessage.begin(), lowerMessage.end(), lowerMessage.begin(), ::tolower);
    
    for (const auto& keyword : codeKeywords) {
        if (lowerMessage.find(keyword) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

void DeveloperRelationsAgent::updateTechnicalKnowledge() {
    // Periodically update knowledge base (in production would sync with docs)
    static auto lastUpdate = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    
    if (now - lastUpdate > std::chrono::hours(24)) {
        
        LOG_INFO("DeveloperRelations", "Updating technical knowledge base");
        lastUpdate = now;
    }
}

// ============================================================================
// ProjectManagerAgent Implementation  
// ============================================================================

ProjectManagerAgent::ProjectManagerAgent(const AgentConfig& config)
    : TheOrgAgent(config, AgentRole::PROJECT_MANAGER) {
}

void ProjectManagerAgent::initialize() {
    
    LOG_INFO("ProjectManager", "Initializing Jimmy Project Manager Agent");
}

void ProjectManagerAgent::start() {
    running_ = true;
    processingThread_ = std::thread(&ProjectManagerAgent::processLoop, this);
    
    
    LOG_INFO("ProjectManager", "Started Jimmy Project Manager Agent");
}

void ProjectManagerAgent::stop() {
    running_ = false;
    if (processingThread_.joinable()) {
        processingThread_.join();
    }
    
    
    LOG_INFO("ProjectManager", "Stopped Jimmy Project Manager Agent");
}

void ProjectManagerAgent::pause() {
    paused_ = true;
}

void ProjectManagerAgent::resume() {
    paused_ = false;
}

bool ProjectManagerAgent::isRunning() const {
    return running_;
}

UUID ProjectManagerAgent::createProject(const std::string& name, const std::string& description, const std::vector<UUID>& teamMemberIds) {
    std::lock_guard<std::mutex> lock(projectMutex_);
    
    Project project;
    project.id = generateUUID();
    project.name = name;
    project.description = description;
    project.status = ProjectStatus::PLANNING;
    project.teamMemberIds = teamMemberIds;
    project.createdAt = std::chrono::system_clock::now();
    project.updatedAt = project.createdAt;
    
    projects_[project.id] = project;
    
    
    LOG_INFO("ProjectManager", "Created project: " + name + " (ID: " + project.id + ")");
    
    return project.id;
}

UUID ProjectManagerAgent::addTeamMember(const TeamMember& member) {
    std::lock_guard<std::mutex> lock(teamMutex_);
    
    TeamMember newMember = member;
    if (newMember.id.empty()) {
        newMember.id = generateUUID();
    }
    
    teamMembers_[newMember.id] = newMember;
    
    
    LOG_INFO("ProjectManager", "Added team member: " + member.name + " (ID: " + newMember.id + ")");
    
    return newMember.id;
}

void ProjectManagerAgent::recordDailyUpdate(const DailyUpdate& update) {
    std::lock_guard<std::mutex> lock(updateMutex_);
    
    DailyUpdate newUpdate = update;
    if (newUpdate.id.empty()) {
        newUpdate.id = generateUUID();
    }
    newUpdate.submittedAt = std::chrono::system_clock::now();
    
    dailyUpdates_.push_back(newUpdate);
    
    
    LOG_INFO("ProjectManager", "Recorded daily update for team member: " + update.teamMemberId);
}

std::string ProjectManagerAgent::generateProjectStatusReport(const UUID& projectId) const {
    std::lock_guard<std::mutex> lock(projectMutex_);
    
    auto it = projects_.find(projectId);
    if (it == projects_.end()) {
        return "Project not found: " + projectId;
    }
    
    const auto& project = it->second;
    
    std::stringstream report;
    report << "📋 **Project Status Report: " << project.name << "**\n\n";
    report << "**Status:** " << (project.status == ProjectStatus::ACTIVE ? "Active" : "Planning") << "\n";
    report << "**Description:** " << project.description << "\n";
    report << "**Team Size:** " << project.teamMemberIds.size() << " members\n";
    report << "**Created:** " << the_org_utils::formatTimestamp(project.createdAt) << "\n";
    report << "**Last Updated:** " << the_org_utils::formatTimestamp(project.updatedAt) << "\n\n";
    
    // Get recent updates for this project
    std::vector<DailyUpdate> projectUpdates;
    {
        std::lock_guard<std::mutex> updateLock(updateMutex_);
        for (const auto& update : dailyUpdates_) {
            if (update.projectId == projectId) {
                projectUpdates.push_back(update);
            }
        }
    }
    
    if (!projectUpdates.empty()) {
        report << "**Recent Updates:**\n";
        // Sort by date and show latest
        std::sort(projectUpdates.begin(), projectUpdates.end(), 
                 [](const DailyUpdate& a, const DailyUpdate& b) {
                     return a.submittedAt > b.submittedAt;
                 });
        
        for (size_t i = 0; i < std::min(projectUpdates.size(), size_t(5)); ++i) {
            const auto& update = projectUpdates[i];
            report << "• " << update.summary << " (" << update.date << ")\n";
        }
    }
    
    return report.str();
}

void ProjectManagerAgent::sendCheckinReminder(const UUID& teamMemberId, const UUID& projectId) {
    auto member = getTeamMember(teamMemberId);
    auto project = getProject(projectId);
    
    if (member && project) {
        std::string message = "🔔 Daily check-in reminder for project: " + project->name + 
                             "\nPlease provide your daily update when you have a moment!";
        
        // In production, would send via appropriate platform/channel
        
        LOG_INFO("ProjectManager", "Sent check-in reminder to " + member->name + " for project " + project->name);
    }
}

void ProjectManagerAgent::processLoop() {
    
    LOG_INFO("ProjectManager", "Started processing loop");
    
    while (running_) {
        if (!paused_) {
            // Send daily check-ins
            sendDailyCheckins();
            
            // Generate weekly reports
            auto now = std::chrono::system_clock::now();
            static auto lastWeeklyReport = now;
            if (now - lastWeeklyReport > std::chrono::hours(168)) { // 1 week
                auto report = generateWeeklyReport();
                
                LOG_INFO("ProjectManager", "Generated weekly report: " + report);
                lastWeeklyReport = now;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(3600)); // Check hourly
    }
}

void ProjectManagerAgent::sendDailyCheckins() {
    // Send check-in reminders to all active team members
    std::lock_guard<std::mutex> teamLock(teamMutex_);
    std::lock_guard<std::mutex> projectLock(projectMutex_);
    
    for (const auto& [projectId, project] : projects_) {
        if (project.status == ProjectStatus::ACTIVE) {
            for (const auto& memberId : project.teamMemberIds) {
                // Check if member should receive reminder based on their timezone/schedule
                sendCheckinReminder(memberId, projectId);
            }
        }
    }
}

std::string ProjectManagerAgent::generateWeeklyReport(const std::vector<UUID>& projectIds) const {
    std::stringstream report;
    report << "📊 **Weekly Project Report**\n\n";
    
    std::vector<UUID> targetProjects = projectIds.empty() ? 
        [this]() {
            std::vector<UUID> allProjects;
            std::lock_guard<std::mutex> lock(projectMutex_);
            for (const auto& [id, _] : projects_) {
                allProjects.push_back(id);
            }
            return allProjects;
        }() : projectIds;
    
    for (const auto& projectId : targetProjects) {
        auto project = getProject(projectId);
        if (project) {
            report << "**" << project->name << "**\n";
            report << "Status: " << (project->status == ProjectStatus::ACTIVE ? "Active" : "Planning") << "\n";
            report << "Team: " << project->teamMemberIds.size() << " members\n\n";
        }
    }
    
    return report.str();
}

std::optional<Project> ProjectManagerAgent::getProject(const UUID& projectId) const {
    std::lock_guard<std::mutex> lock(projectMutex_);
    auto it = projects_.find(projectId);
    return (it != projects_.end()) ? std::optional<Project>(it->second) : std::nullopt;
}

std::optional<TeamMember> ProjectManagerAgent::getTeamMember(const UUID& memberId) const {
    std::lock_guard<std::mutex> lock(teamMutex_);
    auto it = teamMembers_.find(memberId);
    return (it != teamMembers_.end()) ? std::optional<TeamMember>(it->second) : std::nullopt;
}

// ============================================================================
// TheOrgManager Implementation
// ============================================================================

TheOrgManager::TheOrgManager() {
    
    LOG_INFO("TheOrgManager", "Initializing TheOrg management system");
}

TheOrgManager::~TheOrgManager() {
    stopAllAgents();
}

void TheOrgManager::initializeAllAgents(const std::vector<AgentConfig>& configs) {
    
    LOG_INFO("TheOrgManager", "Initializing all agents with " + std::to_string(configs.size()) + " configurations");
    
    for (const auto& config : configs) {
        // Create agents based on their intended role (would be specified in config)
        // For now, create one of each type
        if (agents_.size() == 0) {
            auto cmAgent = std::make_shared<CommunityManagerAgent>(config);
            addAgent(cmAgent);
        } else if (agents_.size() == 1) {
            auto drAgent = std::make_shared<DeveloperRelationsAgent>(config);
            addAgent(drAgent);
        } else if (agents_.size() == 2) {
            auto pmAgent = std::make_shared<ProjectManagerAgent>(config);
            addAgent(pmAgent);
        }
    }
    
    // Initialize all agents
    for (const auto& [id, agent] : agents_) {
        agent->initialize();
    }
}

void TheOrgManager::startAllAgents() {
    std::lock_guard<std::mutex> lock(agentMutex_);
    
    for (const auto& [id, agent] : agents_) {
        agent->start();
    }
    
    running_ = true;
    coordinationThread_ = std::thread(&TheOrgManager::coordinationLoop, this);
    
    
    LOG_INFO("TheOrgManager", "Started all agents and coordination system");
}

void TheOrgManager::stopAllAgents() {
    running_ = false;
    
    if (coordinationThread_.joinable()) {
        coordinationThread_.join();
    }
    
    std::lock_guard<std::mutex> lock(agentMutex_);
    for (const auto& [id, agent] : agents_) {
        agent->stop();
    }
    
    
    LOG_INFO("TheOrgManager", "Stopped all agents and coordination system");
}

void TheOrgManager::addAgent(std::shared_ptr<TheOrgAgent> agent) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    agents_[agent->getId()] = agent;
    roleToAgentMap_[agent->getRole()] = agent->getId();
    
    
    LOG_INFO("TheOrgManager", "Added agent: " + agent->getName() + " (Role: " + the_org_utils::agentRoleToString(agent->getRole()) + ")");
}

std::shared_ptr<TheOrgAgent> TheOrgManager::getAgentByRole(AgentRole role) const {
    std::lock_guard<std::mutex> lock(agentMutex_);
    auto it = roleToAgentMap_.find(role);
    if (it != roleToAgentMap_.end()) {
        auto agentIt = agents_.find(it->second);
        if (agentIt != agents_.end()) {
            return agentIt->second;
        }
    }
    return nullptr;
}

void TheOrgManager::broadcastMessage(const std::string& message, const std::string& senderId, const std::vector<AgentRole>& targetRoles) {
    std::lock_guard<std::mutex> lock(agentMutex_);
    
    for (const auto& [id, agent] : agents_) {
        if (id == senderId) continue; // Don't send to self
        
        if (targetRoles.empty() || std::find(targetRoles.begin(), targetRoles.end(), agent->getRole()) != targetRoles.end()) {
            agent->processMessage(message, senderId);
        }
    }
    
    
    LOG_INFO("TheOrgManager", "Broadcasted message from " + senderId + " to " + std::to_string(targetRoles.size()) + " role types");
}

TheOrgManager::SystemMetrics TheOrgManager::getSystemMetrics() const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    return currentMetrics_;
}

void TheOrgManager::coordinationLoop() {
    
    LOG_INFO("TheOrgManager", "Started coordination loop");
    
    while (running_) {
        // Process inter-agent messages
        processInterAgentMessages();
        
        // Monitor agent health
        monitorAgentHealth();
        
        // Update system metrics
        updateSystemMetrics();
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void TheOrgManager::processInterAgentMessages() {
    // Handle cross-agent communication and coordination
    std::lock_guard<std::mutex> lock(agentMutex_);
    
    // Check for any inter-agent coordination needed
    for (const auto& [id, agent] : agents_) {
        if (!agent->isRunning()) {
            
            LOG_WARNING("TheOrgManager", "Agent " + agent->getName() + " is not running");
        }
    }
}

void TheOrgManager::monitorAgentHealth() {
    // Monitor agent performance and health
    std::lock_guard<std::mutex> lock(agentMutex_);
    
    for (const auto& [id, agent] : agents_) {
        // In production, would check response times, error rates, etc.
        if (agent->isRunning()) {
            // Agent is healthy
        }
    }
}

void TheOrgManager::updateSystemMetrics() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    currentMetrics_.totalAgents = agents_.size();
    currentMetrics_.activeAgents = 0;
    
    for (const auto& [id, agent] : agents_) {
        if (agent->isRunning()) {
            currentMetrics_.activeAgents++;
        }
    }
    
    currentMetrics_.systemLoad = 0.1; // Simulated
    currentMetrics_.averageResponseTime = std::chrono::milliseconds(100); // Simulated
    currentMetrics_.lastUpdated = std::chrono::system_clock::now();
}

// ============================================================================
// Utility Functions Implementation
// ============================================================================

namespace the_org_utils {

std::string formatTimestamp(Timestamp timestamp, const std::string& format) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), format.c_str());
    return ss.str();
}

std::string generateAgentId(AgentRole role) {
    std::string prefix;
    switch (role) {
        case AgentRole::COMMUNITY_MANAGER: prefix = "cm_"; break;
        case AgentRole::DEVELOPER_RELATIONS: prefix = "dr_"; break;
        case AgentRole::COMMUNITY_LIAISON: prefix = "cl_"; break;
        case AgentRole::PROJECT_MANAGER: prefix = "pm_"; break;
        case AgentRole::SOCIAL_MEDIA_MANAGER: prefix = "sm_"; break;
        default: prefix = "ag_"; break;
    }
    return prefix + generateUUID().substr(0, 8);
}

std::vector<std::string> parseHashtags(const std::string& content) {
    std::vector<std::string> hashtags;
    std::regex hashtagRegex(R"(#(\w+))");
    std::sregex_iterator iter(content.begin(), content.end(), hashtagRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        hashtags.push_back(iter->str());
    }
    
    return hashtags;
}

bool isWorkingDay(const std::string& day) {
    return day != "Saturday" && day != "Sunday";
}

std::string platformTypeToString(PlatformType type) {
    switch (type) {
        case PlatformType::DISCORD: return "Discord";
        case PlatformType::TELEGRAM: return "Telegram";
        case PlatformType::TWITTER: return "Twitter";
        case PlatformType::SLACK: return "Slack";
        case PlatformType::FACEBOOK: return "Facebook";
        case PlatformType::LINKEDIN: return "LinkedIn";
        case PlatformType::GITHUB: return "GitHub";
        default: return "Unknown";
    }
}

PlatformType stringToPlatformType(const std::string& str) {
    if (str == "Discord") return PlatformType::DISCORD;
    if (str == "Telegram") return PlatformType::TELEGRAM;
    if (str == "Twitter") return PlatformType::TWITTER;
    if (str == "Slack") return PlatformType::SLACK;
    if (str == "Facebook") return PlatformType::FACEBOOK;
    if (str == "LinkedIn") return PlatformType::LINKEDIN;
    if (str == "GitHub") return PlatformType::GITHUB;
    return PlatformType::DISCORD; // Default
}

std::string agentRoleToString(AgentRole role) {
    switch (role) {
        case AgentRole::COMMUNITY_MANAGER: return "Community Manager";
        case AgentRole::DEVELOPER_RELATIONS: return "Developer Relations";
        case AgentRole::COMMUNITY_LIAISON: return "Community Liaison";
        case AgentRole::PROJECT_MANAGER: return "Project Manager";
        case AgentRole::SOCIAL_MEDIA_MANAGER: return "Social Media Manager";
        default: return "Unknown Role";
    }
}

AgentRole stringToAgentRole(const std::string& str) {
    if (str == "Community Manager") return AgentRole::COMMUNITY_MANAGER;
    if (str == "Developer Relations") return AgentRole::DEVELOPER_RELATIONS;
    if (str == "Community Liaison") return AgentRole::COMMUNITY_LIAISON;
    if (str == "Project Manager") return AgentRole::PROJECT_MANAGER;
    if (str == "Social Media Manager") return AgentRole::SOCIAL_MEDIA_MANAGER;
    return AgentRole::COMMUNITY_MANAGER; // Default
}

double calculateSimilarity(const std::vector<std::string>& list1, const std::vector<std::string>& list2) {
    if (list1.empty() && list2.empty()) return 1.0;
    if (list1.empty() || list2.empty()) return 0.0;
    
    std::set<std::string> set1(list1.begin(), list1.end());
    std::set<std::string> set2(list2.begin(), list2.end());
    
    std::vector<std::string> intersection;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                         std::back_inserter(intersection));
    
    std::vector<std::string> unionSet;
    std::set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
                  std::back_inserter(unionSet));
    
    return static_cast<double>(intersection.size()) / unionSet.size();
}

std::string sanitizeForPlatform(const std::string& content, PlatformType platform) {
    std::string sanitized = content;
    
    // Basic sanitization
    switch (platform) {
        case PlatformType::TWITTER:
            if (sanitized.length() > 280) {
                sanitized = sanitized.substr(0, 277) + "...";
            }
            break;
        case PlatformType::DISCORD:
            if (sanitized.length() > 2000) {
                sanitized = sanitized.substr(0, 1997) + "...";
            }
            break;
        default:
            break;
    }
    
    return sanitized;
}

bool validateUrl(const std::string& url) {
    std::regex urlRegex(R"(https?://[^\s]+)");
    return std::regex_match(url, urlRegex);
}

std::string extractDomain(const std::string& url) {
    std::regex domainRegex(R"(https?://([^/]+))");
    std::smatch match;
    if (std::regex_search(url, match, domainRegex)) {
        return match[1].str();
    }
    return "";
}

std::vector<std::string> splitText(const std::string& text, size_t maxLength, const std::string& delimiter) {
    std::vector<std::string> parts;
    std::istringstream stream(text);
    std::string word;
    std::string currentPart;
    
    while (stream >> word) {
        if (currentPart.length() + word.length() + delimiter.length() <= maxLength) {
            if (!currentPart.empty()) {
                currentPart += delimiter;
            }
            currentPart += word;
        } else {
            if (!currentPart.empty()) {
                parts.push_back(currentPart);
            }
            currentPart = word;
        }
    }
    
    if (!currentPart.empty()) {
        parts.push_back(currentPart);
    }
    
    return parts;
}

std::string joinText(const std::vector<std::string>& parts, const std::string& separator) {
    if (parts.empty()) return "";
    
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += separator + parts[i];
    }
    return result;
}

} // namespace the_org_utils

} // namespace elizaos