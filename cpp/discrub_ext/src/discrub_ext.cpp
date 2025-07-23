#include "elizaos/discrub_ext.hpp"
#include "elizaos/agentlogger.hpp"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <random>
#include <chrono>
#include <cmath>

namespace elizaos {

// Global extension instance
std::shared_ptr<DiscrubExtension> globalDiscrubExtension = std::make_shared<DiscrubExtension>();

// ContentScanner implementation
ContentScanner::ContentScanner() 
    : profanityFilterEnabled_(true), spamFilterEnabled_(true), phishingFilterEnabled_(true),
      inviteFilterEnabled_(true), mentionSpamEnabled_(true), maxMentions_(5) {
    
    // Initialize with basic profanity words (mild examples)
    profanityWords_ = {"spam", "scam", "fake", "hack"};
    
    // Initialize allowed/blocked domains
    allowedDomains_ = {"discord.com", "github.com", "google.com"};
    blockedDomains_ = {"suspicious-site.com", "malware.net"};
    
    // Add default filters
    addFilter(ContentFilter("profanity", "\\b(damn|hell|crap)\\b", FilterAction::WARN, 3));
    addFilter(ContentFilter("spam_repetition", "(.{1})\\1{5,}", FilterAction::DELETE, 5));
    addFilter(ContentFilter("excessive_caps", "[A-Z]{10,}", FilterAction::WARN, 2));
    addFilter(ContentFilter("invite_links", "discord\\.gg/\\w+", FilterAction::DELETE, 4));
}

ContentScanner::~ContentScanner() {}

void ContentScanner::addFilter(const ContentFilter& filter) {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    
    // Remove existing filter with same name
    filters_.erase(std::remove_if(filters_.begin(), filters_.end(),
        [&filter](const ContentFilter& f) { return f.name == filter.name; }), filters_.end());
    
    filters_.push_back(filter);
    logInfo("Added content filter: " + filter.name, "discrub_ext");
}

void ContentScanner::removeFilter(const std::string& name) {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    
    filters_.erase(std::remove_if(filters_.begin(), filters_.end(),
        [&name](const ContentFilter& f) { return f.name == name; }), filters_.end());
    
    logInfo("Removed content filter: " + name, "discrub_ext");
}

void ContentScanner::updateFilter(const std::string& name, const ContentFilter& filter) {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    
    auto it = std::find_if(filters_.begin(), filters_.end(),
        [&name](const ContentFilter& f) { return f.name == name; });
    
    if (it != filters_.end()) {
        *it = filter;
        logInfo("Updated content filter: " + name, "discrub_ext");
    }
}

std::vector<ContentFilter> ContentScanner::getFilters() const {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    return filters_;
}

ContentScanner::ScanResult ContentScanner::scanMessage(const DiscordMessage& message) {
    return scanContent(message.content);
}

ContentScanner::ScanResult ContentScanner::scanContent(const std::string& content) {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    
    ScanResult result;
    result.violation = false;
    result.totalSeverity = 0;
    result.recommendedAction = FilterAction::NONE;
    
    // Apply regex-based filters
    for (const auto& filter : filters_) {
        if (!filter.enabled) continue;
        
        try {
            if (std::regex_search(content, filter.pattern)) {
                result.violation = true;
                result.triggeredFilters.push_back(filter.name);
                result.totalSeverity += filter.severity;
                
                // Use highest severity action
                if (static_cast<int>(filter.action) > static_cast<int>(result.recommendedAction)) {
                    result.recommendedAction = filter.action;
                }
                
                if (!filter.reason.empty()) {
                    if (!result.reason.empty()) result.reason += "; ";
                    result.reason += filter.reason;
                }
            }
        } catch (const std::regex_error& e) {
            logError("Regex error in filter " + filter.name + ": " + e.what(), "discrub_ext");
        }
    }
    
    // Apply built-in detection methods
    if (profanityFilterEnabled_ && detectProfanity(content)) {
        result.violation = true;
        result.triggeredFilters.push_back("built-in-profanity");
        result.totalSeverity += 3;
        if (result.recommendedAction < FilterAction::WARN) {
            result.recommendedAction = FilterAction::WARN;
        }
    }
    
    if (phishingFilterEnabled_ && detectPhishing(content)) {
        result.violation = true;
        result.triggeredFilters.push_back("built-in-phishing");
        result.totalSeverity += 8;
        result.recommendedAction = FilterAction::DELETE;
    }
    
    if (inviteFilterEnabled_ && detectInviteLinks(content)) {
        result.violation = true;
        result.triggeredFilters.push_back("built-in-invite");
        result.totalSeverity += 4;
        if (result.recommendedAction < FilterAction::DELETE) {
            result.recommendedAction = FilterAction::DELETE;
        }
    }
    
    // Build reason string
    if (result.violation && result.reason.empty()) {
        result.reason = "Content policy violation detected";
    }
    
    return result;
}

std::vector<ContentScanner::ScanResult> ContentScanner::scanMessages(const std::vector<DiscordMessage>& messages) {
    std::vector<ScanResult> results;
    
    for (const auto& message : messages) {
        results.push_back(scanMessage(message));
    }
    
    logInfo("Scanned " + std::to_string(messages.size()) + " messages", "discrub_ext");
    
    return results;
}

void ContentScanner::enableProfanityFilter(bool enable) {
    profanityFilterEnabled_ = enable;
    logInfo(std::string("Profanity filter ") + (enable ? "enabled" : "disabled"), "discrub_ext");
}

void ContentScanner::enableSpamFilter(bool enable) {
    spamFilterEnabled_ = enable;
    logInfo(std::string("Spam filter ") + (enable ? "enabled" : "disabled"), "discrub_ext");
}

void ContentScanner::enablePhishingFilter(bool enable) {
    phishingFilterEnabled_ = enable;
    logInfo(std::string("Phishing filter ") + (enable ? "enabled" : "disabled"), "discrub_ext");
}

void ContentScanner::enableInviteFilter(bool enable) {
    inviteFilterEnabled_ = enable;
    logInfo(std::string("Invite filter ") + (enable ? "enabled" : "disabled"), "discrub_ext");
}

void ContentScanner::enableMentionSpamFilter(bool enable, int maxMentions) {
    mentionSpamEnabled_ = enable;
    maxMentions_ = maxMentions;
    logInfo(std::string("Mention spam filter ") + (enable ? "enabled" : "disabled") + 
        " (max: " + std::to_string(maxMentions) + ")", "discrub_ext");
}

void ContentScanner::addProfanityWords(const std::vector<std::string>& words) {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    for (const auto& word : words) {
        profanityWords_.insert(word);
    }
    logInfo("Added " + std::to_string(words.size()) + " profanity words", "discrub_ext");
}

void ContentScanner::addAllowedDomains(const std::vector<std::string>& domains) {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    for (const auto& domain : domains) {
        allowedDomains_.insert(domain);
    }
    logInfo("Added " + std::to_string(domains.size()) + " allowed domains", "discrub_ext");
}

void ContentScanner::addBlockedDomains(const std::vector<std::string>& domains) {
    std::lock_guard<std::mutex> lock(scannerMutex_);
    for (const auto& domain : domains) {
        blockedDomains_.insert(domain);
    }
    logInfo("Added " + std::to_string(domains.size()) + " blocked domains", "discrub_ext");
}

bool ContentScanner::detectProfanity(const std::string& content) {
    std::string lowerContent = content;
    std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
    
    for (const auto& word : profanityWords_) {
        if (lowerContent.find(word) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

bool ContentScanner::detectSpam(const DiscordMessage& message) {
    const std::string& content = message.content;
    
    // Check for excessive repetition
    size_t maxRepeat = 0;
    for (size_t i = 0; i < content.length(); ++i) {
        size_t count = 1;
        while (i + count < content.length() && content[i] == content[i + count]) {
            count++;
        }
        maxRepeat = std::max(maxRepeat, count);
    }
    
    if (maxRepeat > 5) {
        return true;
    }
    
    // Check for excessive length
    if (content.length() > 2000) {
        return true;
    }
    
    // Check for mention spam
    if (mentionSpamEnabled_ && detectMentionSpam(message)) {
        return true;
    }
    
    return false;
}

bool ContentScanner::detectPhishing(const std::string& content) {
    // Look for suspicious patterns
    std::vector<std::string> suspiciousPatterns = {
        "click here to claim",
        "free nitro",
        "discord gift",
        "steam gift",
        "limited time",
        "verify your account"
    };
    
    std::string lowerContent = content;
    std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
    
    for (const auto& pattern : suspiciousPatterns) {
        if (lowerContent.find(pattern) != std::string::npos) {
            // Check if it contains URLs
            auto urls = extractUrls(content);
            if (!urls.empty()) {
                return true;
            }
        }
    }
    
    // Check for blocked domains
    auto urls = extractUrls(content);
    for (const auto& url : urls) {
        for (const auto& blockedDomain : blockedDomains_) {
            if (url.find(blockedDomain) != std::string::npos) {
                return true;
            }
        }
    }
    
    return false;
}

bool ContentScanner::detectInviteLinks(const std::string& content) {
    std::regex invitePattern(R"(discord\.gg/\w+|discordapp\.com/invite/\w+)");
    return std::regex_search(content, invitePattern);
}

bool ContentScanner::detectMentionSpam(const DiscordMessage& message) {
    int mentionCount = countMentions(message.content);
    return mentionCount > maxMentions_;
}

std::vector<std::string> ContentScanner::extractUrls(const std::string& content) {
    std::vector<std::string> urls;
    std::regex urlPattern(R"(https?://[^\s]+)");
    
    std::sregex_iterator iter(content.begin(), content.end(), urlPattern);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        urls.push_back(iter->str());
    }
    
    return urls;
}

int ContentScanner::countMentions(const std::string& content) {
    std::regex mentionPattern(R"(<@!?\d+>|@everyone|@here)");
    
    int count = 0;
    std::sregex_iterator iter(content.begin(), content.end(), mentionPattern);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        count++;
    }
    
    return count;
}

// AutoModerator implementation
AutoModerator::AutoModerator() 
    : strictMode_(false), autoEscalation_(true), reputationThreshold_(50), actionCooldownSeconds_(300) {}
AutoModerator::~AutoModerator() {}

// ContentCleaner implementation
ContentCleaner::ContentCleaner() : cleanupRunning_(false) {}
ContentCleaner::~ContentCleaner() {}

// ModerationAnalytics implementation
ModerationAnalytics::ModerationAnalytics() {}
ModerationAnalytics::~ModerationAnalytics() {}

// DiscrubExtension main implementation - simplified for compilation
DiscrubExtension::DiscrubExtension() : monitoring_(false) {}

DiscrubExtension::~DiscrubExtension() {
    stopMonitoring();
}

bool DiscrubExtension::initializeWithDiscord(std::shared_ptr<DiscordClient> client) {
    discordClient_ = client;
    
    if (discordClient_) {
        logInfo("Discrub Extension initialized with Discord client", "discrub_ext");
        return true;
    }
    
    logError("Failed to initialize - no Discord client provided", "discrub_ext");
    return false;
}

void DiscrubExtension::startMonitoring(const std::vector<std::string>& channelIds) {
    std::lock_guard<std::mutex> lock(configMutex_);
    
    if (monitoring_) {
        logWarning("Monitoring already active", "discrub_ext");
        return;
    }
    
    monitoredChannels_ = channelIds;
    monitoring_ = true;
    
    logInfo("Started monitoring " + std::to_string(channelIds.size()) + " channels", "discrub_ext");
}

void DiscrubExtension::stopMonitoring() {
    {
        std::lock_guard<std::mutex> lock(configMutex_);
        monitoring_ = false;
    }
    
    logInfo("Stopped monitoring", "discrub_ext");
}

bool DiscrubExtension::isMonitoring() const {
    return monitoring_.load();
}

void DiscrubExtension::processIncomingMessage(const DiscordMessage& message) {
    // Scan the message
    auto scanResult = scanner_.scanMessage(message);
    
    if (scanResult.violation) {
        logWarning("Violation detected in message " + message.id + ": " + scanResult.reason, "discrub_ext");
    }
}

void DiscrubExtension::processMessageEdit(const DiscordMessage& /* oldMessage */, const DiscordMessage& newMessage) {
    // Process the edited message
    processIncomingMessage(newMessage);
}

void DiscrubExtension::processMessageDelete(const std::string& channelId, const std::string& messageId) {
    logInfo("Processed message deletion: " + messageId + " in channel " + channelId, "discrub_ext");
}

void DiscrubExtension::loadConfiguration(const std::string& configPath) {
    logInfo("Loading configuration from: " + configPath, "discrub_ext");
}

void DiscrubExtension::saveConfiguration(const std::string& configPath) {
    logInfo("Saving configuration to: " + configPath, "discrub_ext");
}

void DiscrubExtension::setDefaultModerationSettings() {
    scanner_.enableProfanityFilter(true);
    scanner_.enableSpamFilter(true);
    scanner_.enablePhishingFilter(true);
    scanner_.enableInviteFilter(true);
    scanner_.enableMentionSpamFilter(true, 5);
    
    logInfo("Applied default moderation settings", "discrub_ext");
}

void DiscrubExtension::monitoringLoop() {
    logInfo("Monitoring loop started", "discrub_ext");
    
    while (monitoring_) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // Monitoring implementation would go here
    }
    
    logInfo("Monitoring loop ended", "discrub_ext");
}

} // namespace elizaos