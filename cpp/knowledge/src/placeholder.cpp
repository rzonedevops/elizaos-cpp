#include "elizaos/knowledge.hpp"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <random>
#include <set>
#include <iomanip>
#include <any>

namespace elizaos {

// Global knowledge base instance
std::shared_ptr<KnowledgeBase> globalKnowledgeBase = std::make_shared<KnowledgeBase>();

// Simple UUID generator for knowledge entries
std::string generateKnowledgeUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string uuid = "knowledge-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
    for (auto& c : uuid) {
        if (c == 'x') {
            c = "0123456789abcdef"[dis(gen)];
        }
    }
    return uuid;
}

// =====================================================
// KnowledgeEntry Implementation
// =====================================================

KnowledgeEntry::KnowledgeEntry(const std::string& content, KnowledgeType type) 
    : content(content), type(type), confidence(ConfidenceLevel::MEDIUM), 
      source(KnowledgeSource::PROGRAMMED) {
    id = generateKnowledgeUUID();
    created_at = std::chrono::system_clock::now();
    updated_at = created_at;
}

JsonValue KnowledgeEntry::toJson() const {
    JsonValue json;
    json["id"] = std::string(id);
    json["content"] = std::string(content);
    json["type"] = std::string(knowledgeTypeToString(type));
    json["confidence"] = std::string(confidenceLevelToString(confidence));
    json["source"] = std::string(knowledgeSourceToString(source));
    
    // Simple timestamp conversion
    json["created_at"] = std::string(std::to_string(std::chrono::system_clock::to_time_t(created_at)));
    json["updated_at"] = std::string(std::to_string(std::chrono::system_clock::to_time_t(updated_at)));
    
    return json;
}

KnowledgeEntry KnowledgeEntry::fromJson(const JsonValue& json) {
    KnowledgeEntry entry("", KnowledgeType::FACT);
    
    try {
        auto getString = [&](const std::string& key) -> std::string {
            auto it = json.find(key);
            if (it != json.end()) {
                return std::any_cast<std::string>(it->second);
            }
            return "";
        };
        
        entry.id = getString("id");
        entry.content = getString("content");
        entry.type = stringToKnowledgeType(getString("type"));
        entry.confidence = stringToConfidenceLevel(getString("confidence"));
        entry.source = stringToKnowledgeSource(getString("source"));
        
        // Parse timestamps
        auto created_time_t = std::stoll(getString("created_at"));
        auto updated_time_t = std::stoll(getString("updated_at"));
        entry.created_at = std::chrono::system_clock::from_time_t(created_time_t);
        entry.updated_at = std::chrono::system_clock::from_time_t(updated_time_t);
    } catch (const std::exception&) {
        // Handle parsing errors gracefully
    }
    
    return entry;
}

void KnowledgeEntry::addTag(const std::string& tag) {
    if (std::find(tags.begin(), tags.end(), tag) == tags.end()) {
        tags.push_back(tag);
        updated_at = std::chrono::system_clock::now();
    }
}

void KnowledgeEntry::addRelation(const std::string& entryId) {
    if (std::find(related_entries.begin(), related_entries.end(), entryId) == related_entries.end()) {
        related_entries.push_back(entryId);
        updated_at = std::chrono::system_clock::now();
    }
}

void KnowledgeEntry::updateConfidence(ConfidenceLevel newConfidence) {
    confidence = newConfidence;
    updated_at = std::chrono::system_clock::now();
}

bool KnowledgeEntry::hasTag(const std::string& tag) const {
    return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

// =====================================================
// KnowledgeQuery Implementation
// =====================================================

KnowledgeQuery::KnowledgeQuery(const std::string& queryText) : text(queryText) {}

// =====================================================
// KnowledgeInferenceEngine Implementation
// =====================================================

KnowledgeInferenceEngine::KnowledgeInferenceEngine() {
    // Add some basic inference rules
    addInferenceRule("transitivity", [](const std::vector<KnowledgeEntry>& facts) {
        std::vector<KnowledgeEntry> inferred;
        // Basic transitivity: if A relates to B and B relates to C, then A relates to C
        // This is a simplified implementation
        for (const auto& fact1 : facts) {
            for (const auto& fact2 : facts) {
                if (fact1.type == KnowledgeType::RELATIONSHIP && 
                    fact2.type == KnowledgeType::RELATIONSHIP) {
                    // Find common elements and create transitivity
                    for (const auto& related1 : fact1.related_entries) {
                        if (std::find(fact2.related_entries.begin(), fact2.related_entries.end(), related1) != fact2.related_entries.end()) {
                            KnowledgeEntry transitiveEntry("Transitive relationship inferred", KnowledgeType::RELATIONSHIP);
                            transitiveEntry.source = KnowledgeSource::INFERRED;
                            transitiveEntry.confidence = ConfidenceLevel::LOW;
                            transitiveEntry.addTag("inferred");
                            transitiveEntry.addTag("transitivity");
                            inferred.push_back(transitiveEntry);
                        }
                    }
                }
            }
        }
        return inferred;
    });
}

std::vector<KnowledgeEntry> KnowledgeInferenceEngine::inferFromFacts(const std::vector<KnowledgeEntry>& facts) {
    std::vector<KnowledgeEntry> allInferred;
    
    std::lock_guard<std::mutex> lock(rulesMutex_);
    for (const auto& rulePair : rules_) {
        auto inferred = rulePair.second(facts);
        allInferred.insert(allInferred.end(), inferred.begin(), inferred.end());
    }
    
    return allInferred;
}

std::vector<KnowledgeEntry> KnowledgeInferenceEngine::findRelatedConcepts(const KnowledgeEntry& entry) {
    std::vector<KnowledgeEntry> related;
    
    // Find concepts that share tags or content similarity
    // This is a simplified implementation that would benefit from semantic analysis
    KnowledgeEntry conceptEntry("Related concept to: " + entry.content, KnowledgeType::CONCEPT);
    conceptEntry.source = KnowledgeSource::INFERRED;
    conceptEntry.confidence = ConfidenceLevel::MEDIUM;
    conceptEntry.addTag("related");
    conceptEntry.addRelation(entry.id);
    
    related.push_back(conceptEntry);
    return related;
}

KnowledgeEntry KnowledgeInferenceEngine::combineEvidence(const std::vector<KnowledgeEntry>& evidence) {
    if (evidence.empty()) {
        return KnowledgeEntry("No evidence to combine", KnowledgeType::FACT);
    }
    
    KnowledgeEntry combined("Combined evidence", KnowledgeType::FACT);
    combined.source = KnowledgeSource::INFERRED;
    
    // Calculate average confidence
    int totalConfidence = 0;
    for (const auto& entry : evidence) {
        totalConfidence += static_cast<int>(entry.confidence);
        combined.addRelation(entry.id);
    }
    
    int avgConfidence = totalConfidence / evidence.size();
    combined.confidence = static_cast<ConfidenceLevel>(std::clamp(avgConfidence, 1, 5));
    
    combined.addTag("combined_evidence");
    return combined;
}

void KnowledgeInferenceEngine::addInferenceRule(const std::string& ruleName, 
                                               std::function<std::vector<KnowledgeEntry>(const std::vector<KnowledgeEntry>&)> rule) {
    std::lock_guard<std::mutex> lock(rulesMutex_);
    rules_[ruleName] = rule;
}

void KnowledgeInferenceEngine::removeInferenceRule(const std::string& ruleName) {
    std::lock_guard<std::mutex> lock(rulesMutex_);
    rules_.erase(ruleName);
}

// =====================================================
// KnowledgeBase Implementation
// =====================================================

KnowledgeBase::KnowledgeBase() {
    memory_ = std::make_shared<AgentMemoryManager>();
    logger_ = std::make_shared<AgentLogger>();
    inferenceEngine_ = std::make_shared<KnowledgeInferenceEngine>();
    
    logger_->log("Knowledge base initialized", "info", "knowledge");
}

KnowledgeBase::~KnowledgeBase() = default;

std::string KnowledgeBase::generateKnowledgeId() {
    return generateKnowledgeUUID();
}

std::string KnowledgeBase::addKnowledge(const KnowledgeEntry& entry) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    if (!isValidKnowledgeEntry(entry)) {
        logger_->log("Invalid knowledge entry rejected", "warning", "knowledge");
        return "";
    }
    
    KnowledgeEntry newEntry = entry;
    if (newEntry.id.empty()) {
        newEntry.id = generateKnowledgeId();
    }
    
    saveKnowledgeToMemory(newEntry);
    updateKnowledgeMetrics(newEntry);
    
    logger_->log("Added knowledge: " + newEntry.content.substr(0, 50) + "...", "info", "knowledge");
    return newEntry.id;
}

bool KnowledgeBase::updateKnowledge(const std::string& id, const KnowledgeEntry& entry) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    auto existing = loadKnowledgeFromMemory(id);
    if (!existing) {
        logger_->log("Knowledge entry not found for update: " + id, "warning", "knowledge");
        return false;
    }
    
    KnowledgeEntry updatedEntry = entry;
    updatedEntry.id = id;
    updatedEntry.updated_at = std::chrono::system_clock::now();
    
    saveKnowledgeToMemory(updatedEntry);
    logger_->log("Updated knowledge: " + id, "info", "knowledge");
    return true;
}

bool KnowledgeBase::removeKnowledge(const std::string& id) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    // Try to find and remove from memory
    UUID memoryId(id);
    bool removed = memory_->deleteMemory(memoryId);
    
    if (removed) {
        logger_->log("Removed knowledge: " + id, "info", "knowledge");
    } else {
        logger_->log("Failed to remove knowledge: " + id, "warning", "knowledge");
    }
    
    return removed;
}

std::optional<KnowledgeEntry> KnowledgeBase::getKnowledge(const std::string& id) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    return loadKnowledgeFromMemory(id);
}

std::vector<KnowledgeEntry> KnowledgeBase::query(const KnowledgeQuery& query) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    std::vector<KnowledgeEntry> results;
    
    // Start with text search
    auto textResults = searchMemoryByContent(query.text, query.maxResults * 2);
    
    // Filter by criteria
    for (const auto& entry : textResults) {
        bool matches = true;
        
        // Check confidence level
        if (entry.confidence < query.minConfidence) {
            matches = false;
        }
        
        // Check knowledge types
        if (!query.types.empty()) {
            bool typeMatches = std::find(query.types.begin(), query.types.end(), entry.type) != query.types.end();
            if (!typeMatches) {
                matches = false;
            }
        }
        
        // Check tags
        if (!query.tags.empty()) {
            bool tagMatches = false;
            for (const auto& tag : query.tags) {
                if (entry.hasTag(tag)) {
                    tagMatches = true;
                    break;
                }
            }
            if (!tagMatches) {
                matches = false;
            }
        }
        
        if (matches) {
            results.push_back(entry);
            if (results.size() >= static_cast<size_t>(query.maxResults)) {
                break;
            }
        }
    }
    
    // Add related entries if requested
    if (query.includeRelated && !results.empty()) {
        std::set<std::string> addedIds;
        for (const auto& entry : results) {
            addedIds.insert(entry.id);
        }
        
        std::vector<KnowledgeEntry> relatedResults;
        for (const auto& entry : results) {
            auto related = getRelatedKnowledge(entry.id, 3);
            for (const auto& relatedEntry : related) {
                if (addedIds.find(relatedEntry.id) == addedIds.end()) {
                    relatedResults.push_back(relatedEntry);
                    addedIds.insert(relatedEntry.id);
                }
            }
        }
        
        results.insert(results.end(), relatedResults.begin(), relatedResults.end());
    }
    
    logger_->log("Knowledge query returned " + std::to_string(results.size()) + " results", "info", "knowledge");
    return results;
}

std::vector<KnowledgeEntry> KnowledgeBase::searchByText(const std::string& text, int maxResults) {
    KnowledgeQuery query(text);
    query.maxResults = maxResults;
    return this->query(query);
}

std::vector<KnowledgeEntry> KnowledgeBase::searchByTags(const std::vector<std::string>& tags, int maxResults) {
    KnowledgeQuery query("");
    query.tags = tags;
    query.maxResults = maxResults;
    return this->query(query);
}

std::vector<KnowledgeEntry> KnowledgeBase::getRelatedKnowledge(const std::string& entryId, int maxResults) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    auto entry = loadKnowledgeFromMemory(entryId);
    if (!entry) {
        return {};
    }
    
    std::vector<KnowledgeEntry> related;
    
    // Get directly related entries
    for (const auto& relatedId : entry->related_entries) {
        auto relatedEntry = loadKnowledgeFromMemory(relatedId);
        if (relatedEntry) {
            related.push_back(*relatedEntry);
        }
    }
    
    // Get entries with similar tags
    auto allEntries = getAllKnowledgeFromMemory();
    for (const auto& otherEntry : allEntries) {
        if (otherEntry.id == entryId) continue;
        
        // Check for tag overlap
        int commonTags = 0;
        for (const auto& tag : entry->tags) {
            if (otherEntry.hasTag(tag)) {
                commonTags++;
            }
        }
        
        if (commonTags > 0) {
            related.push_back(otherEntry);
        }
        
        if (related.size() >= static_cast<size_t>(maxResults)) {
            break;
        }
    }
    
    return related;
}

std::vector<std::string> KnowledgeBase::getAllTags() const {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    std::set<std::string> uniqueTags;
    auto allEntries = getAllKnowledgeFromMemory();
    
    for (const auto& entry : allEntries) {
        for (const auto& tag : entry.tags) {
            uniqueTags.insert(tag);
        }
    }
    
    return std::vector<std::string>(uniqueTags.begin(), uniqueTags.end());
}

std::unordered_map<KnowledgeType, int> KnowledgeBase::getKnowledgeTypeStats() const {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    std::unordered_map<KnowledgeType, int> stats;
    auto allEntries = getAllKnowledgeFromMemory();
    
    for (const auto& entry : allEntries) {
        stats[entry.type]++;
    }
    
    return stats;
}

std::vector<KnowledgeEntry> KnowledgeBase::getKnowledgeByType(KnowledgeType type) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    std::vector<KnowledgeEntry> results;
    auto allEntries = getAllKnowledgeFromMemory();
    
    for (const auto& entry : allEntries) {
        if (entry.type == type) {
            results.push_back(entry);
        }
    }
    
    return results;
}

void KnowledgeBase::validateKnowledge() {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    auto allEntries = getAllKnowledgeFromMemory();
    int validCount = 0;
    int invalidCount = 0;
    
    for (const auto& entry : allEntries) {
        if (isValidKnowledgeEntry(entry)) {
            validCount++;
        } else {
            invalidCount++;
            logger_->log("Invalid knowledge entry found: " + entry.id, "warning", "knowledge");
        }
    }
    
    logger_->log("Knowledge validation complete. Valid: " + std::to_string(validCount) + 
                ", Invalid: " + std::to_string(invalidCount), "info", "knowledge");
}

void KnowledgeBase::pruneOldKnowledge(std::chrono::hours maxAge) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    auto cutoffTime = std::chrono::system_clock::now() - maxAge;
    auto allEntries = getAllKnowledgeFromMemory();
    int prunedCount = 0;
    
    for (const auto& entry : allEntries) {
        if (entry.updated_at < cutoffTime && entry.confidence <= ConfidenceLevel::LOW) {
            if (removeKnowledge(entry.id)) {
                prunedCount++;
            }
        }
    }
    
    logger_->log("Pruned " + std::to_string(prunedCount) + " old knowledge entries", "info", "knowledge");
}

void KnowledgeBase::consolidateKnowledge() {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    // Find and merge similar knowledge entries
    auto allEntries = getAllKnowledgeFromMemory();
    int consolidated = 0;
    
    for (size_t i = 0; i < allEntries.size(); ++i) {
        for (size_t j = i + 1; j < allEntries.size(); ++j) {
            const auto& entry1 = allEntries[i];
            const auto& entry2 = allEntries[j];
            
            // Simple similarity check based on content similarity
            if (entry1.type == entry2.type && 
                entry1.content.find(entry2.content.substr(0, 20)) != std::string::npos) {
                
                // Merge the entries by keeping the higher confidence one
                if (entry1.confidence >= entry2.confidence) {
                    // Keep entry1, remove entry2
                    if (removeKnowledge(entry2.id)) {
                        consolidated++;
                    }
                } else {
                    // Keep entry2, remove entry1
                    if (removeKnowledge(entry1.id)) {
                        consolidated++;
                    }
                }
                break;
            }
        }
    }
    
    logger_->log("Consolidated " + std::to_string(consolidated) + " knowledge entries", "info", "knowledge");
}

std::vector<KnowledgeEntry> KnowledgeBase::performInference(const KnowledgeQuery& query) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    // Get relevant facts for inference
    auto facts = this->query(query);
    
    // Perform inference
    auto inferred = inferenceEngine_->inferFromFacts(facts);
    
    // Add inferred knowledge to the base
    for (const auto& entry : inferred) {
        addKnowledge(entry);
    }
    
    logger_->log("Inference generated " + std::to_string(inferred.size()) + " new knowledge entries", 
                "info", "knowledge");
    
    return inferred;
}

void KnowledgeBase::setInferenceEngine(std::shared_ptr<KnowledgeInferenceEngine> engine) {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    inferenceEngine_ = engine;
    logger_->log("Knowledge inference engine updated", "info", "knowledge");
}

bool KnowledgeBase::exportToFile(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << "Knowledge Export - " << filename << std::endl;
        file << "Total entries: " << getKnowledgeCount() << std::endl;
        file << getStatistics() << std::endl;
        
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool KnowledgeBase::importFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        // In a real implementation, we'd parse the JSON properly
        logger_->log("Knowledge import from file: " + filename, "info", "knowledge");
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

JsonValue KnowledgeBase::exportToJson() const {
    JsonValue json;
    auto allEntries = getAllKnowledgeFromMemory();
    
    json["total_entries"] = std::string(std::to_string(allEntries.size()));
    json["export_timestamp"] = std::string(std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())));
    
    return json;
}

bool KnowledgeBase::importFromJson(const JsonValue& data) {
    try {
        // Use the data parameter to avoid warning
        if (data.empty()) {
            logger_->log("Empty JSON data provided for import", "warning", "knowledge");
            return false;
        }
        
        logger_->log("Knowledge import from JSON data", "info", "knowledge");
        // Simple implementation - in practice would deserialize entries
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

size_t KnowledgeBase::getKnowledgeCount() const {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    return getAllKnowledgeFromMemory().size();
}

std::string KnowledgeBase::getStatistics() const {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    
    auto stats = getKnowledgeTypeStats();
    auto allTags = getAllTags();
    
    std::stringstream ss;
    ss << "Knowledge Base Statistics:" << std::endl;
    ss << "Total entries: " << getKnowledgeCount() << std::endl;
    ss << "Total tags: " << allTags.size() << std::endl;
    ss << "Knowledge types:" << std::endl;
    
    for (const auto& pair : stats) {
        ss << "  " << knowledgeTypeToString(pair.first) << ": " << pair.second << std::endl;
    }
    
    return ss.str();
}

void KnowledgeBase::clear() {
    std::lock_guard<std::mutex> lock(knowledgeMutex_);
    memory_->clear();
    logger_->log("Knowledge base cleared", "info", "knowledge");
}

// Private helper methods
void KnowledgeBase::saveKnowledgeToMemory(const KnowledgeEntry& entry) {
    UUID memoryId(entry.id);
    UUID entityId = generateKnowledgeUUID();
    UUID agentId = generateKnowledgeUUID();
    
    // Create CustomMetadata for the knowledge entry
    CustomMetadata customMeta;
    customMeta.customData["id"] = entry.id;
    customMeta.customData["type"] = knowledgeTypeToString(entry.type);
    customMeta.customData["confidence"] = confidenceLevelToString(entry.confidence);
    customMeta.customData["source"] = knowledgeSourceToString(entry.source);
    customMeta.customData["created_at"] = std::to_string(std::chrono::system_clock::to_time_t(entry.created_at));
    customMeta.customData["updated_at"] = std::to_string(std::chrono::system_clock::to_time_t(entry.updated_at));
    
    // Add tags as comma-separated string
    if (!entry.tags.empty()) {
        std::stringstream tagStream;
        for (size_t i = 0; i < entry.tags.size(); ++i) {
            if (i > 0) tagStream << ",";
            tagStream << entry.tags[i];
        }
        customMeta.customData["tags"] = tagStream.str();
    }
    
    MemoryMetadata metadata = customMeta;
    auto memory = std::make_shared<Memory>(memoryId, entry.content, entityId, agentId, metadata);
    
    memory_->createMemory(memory, "knowledge");
}

std::optional<KnowledgeEntry> KnowledgeBase::loadKnowledgeFromMemory(const std::string& id) {
    UUID memoryId(id);
    auto memory = memory_->getMemoryById(memoryId);
    
    if (!memory) {
        return std::nullopt;
    }
    
    KnowledgeEntry entry("", KnowledgeType::FACT);
    entry.id = id;
    entry.content = memory->getContent();
    
    // Parse metadata if it's CustomMetadata
    if (std::holds_alternative<CustomMetadata>(memory->getMetadata())) {
        const auto& customMeta = std::get<CustomMetadata>(memory->getMetadata());
        
        auto getValue = [&](const std::string& key) -> std::string {
            auto it = customMeta.customData.find(key);
            return it != customMeta.customData.end() ? it->second : "";
        };
        
        entry.type = stringToKnowledgeType(getValue("type"));
        entry.confidence = stringToConfidenceLevel(getValue("confidence"));
        entry.source = stringToKnowledgeSource(getValue("source"));
        
        // Parse tags from comma-separated string
        std::string tagsStr = getValue("tags");
        if (!tagsStr.empty()) {
            std::stringstream ss(tagsStr);
            std::string tag;
            while (std::getline(ss, tag, ',')) {
                entry.tags.push_back(tag);
            }
        }
        
        // Parse timestamps
        try {
            auto created_time_t = std::stoll(getValue("created_at"));
            auto updated_time_t = std::stoll(getValue("updated_at"));
            entry.created_at = std::chrono::system_clock::from_time_t(created_time_t);
            entry.updated_at = std::chrono::system_clock::from_time_t(updated_time_t);
        } catch (const std::exception&) {
            entry.created_at = std::chrono::system_clock::now();
            entry.updated_at = entry.created_at;
        }
    }
    
    return entry;
}

std::vector<KnowledgeEntry> KnowledgeBase::searchMemoryByContent(const std::string& content, int maxResults) {
    MemorySearchParams params;
    params.tableName = "knowledge";
    params.count = maxResults;
    
    auto memories = memory_->getMemories(params);
    std::vector<KnowledgeEntry> results;
    
    for (const auto& memory : memories) {
        if (memory->getContent().find(content) != std::string::npos) {
            auto entry = loadKnowledgeFromMemory(memory->getId());
            if (entry) {
                results.push_back(*entry);
            }
        }
    }
    
    return results;
}

std::vector<KnowledgeEntry> KnowledgeBase::getAllKnowledgeFromMemory() const {
    MemorySearchParams params;
    params.tableName = "knowledge";
    params.count = 1000; // Large number to get all
    
    auto memories = memory_->getMemories(params);
    std::vector<KnowledgeEntry> results;
    
    for (const auto& memory : memories) {
        auto entry = const_cast<KnowledgeBase*>(this)->loadKnowledgeFromMemory(memory->getId());
        if (entry) {
            results.push_back(*entry);
        }
    }
    
    return results;
}

bool KnowledgeBase::isValidKnowledgeEntry(const KnowledgeEntry& entry) const {
    return !entry.content.empty() && 
           entry.confidence >= ConfidenceLevel::VERY_LOW && 
           entry.confidence <= ConfidenceLevel::VERY_HIGH;
}

void KnowledgeBase::updateKnowledgeMetrics(const KnowledgeEntry& entry) {
    // Update internal metrics - this could track usage patterns, quality scores, etc.
    // For now, we just log the addition
    logger_->log("Knowledge metrics updated for entry type: " + 
                knowledgeTypeToString(entry.type), "debug", "knowledge");
}

// =====================================================
// Utility Functions
// =====================================================

std::string knowledgeTypeToString(KnowledgeType type) {
    switch (type) {
        case KnowledgeType::FACT: return "fact";
        case KnowledgeType::RULE: return "rule";
        case KnowledgeType::CONCEPT: return "concept";
        case KnowledgeType::RELATIONSHIP: return "relationship";
        case KnowledgeType::PROCEDURE: return "procedure";
        case KnowledgeType::EXPERIENCE: return "experience";
        default: return "fact";
    }
}

KnowledgeType stringToKnowledgeType(const std::string& typeStr) {
    if (typeStr == "rule") return KnowledgeType::RULE;
    if (typeStr == "concept") return KnowledgeType::CONCEPT;
    if (typeStr == "relationship") return KnowledgeType::RELATIONSHIP;
    if (typeStr == "procedure") return KnowledgeType::PROCEDURE;
    if (typeStr == "experience") return KnowledgeType::EXPERIENCE;
    return KnowledgeType::FACT;
}

std::string confidenceLevelToString(ConfidenceLevel level) {
    switch (level) {
        case ConfidenceLevel::VERY_LOW: return "very_low";
        case ConfidenceLevel::LOW: return "low";
        case ConfidenceLevel::MEDIUM: return "medium";
        case ConfidenceLevel::HIGH: return "high";
        case ConfidenceLevel::VERY_HIGH: return "very_high";
        default: return "medium";
    }
}

ConfidenceLevel stringToConfidenceLevel(const std::string& levelStr) {
    if (levelStr == "very_low") return ConfidenceLevel::VERY_LOW;
    if (levelStr == "low") return ConfidenceLevel::LOW;
    if (levelStr == "high") return ConfidenceLevel::HIGH;
    if (levelStr == "very_high") return ConfidenceLevel::VERY_HIGH;
    return ConfidenceLevel::MEDIUM;
}

std::string knowledgeSourceToString(KnowledgeSource source) {
    switch (source) {
        case KnowledgeSource::LEARNED: return "learned";
        case KnowledgeSource::PROGRAMMED: return "programmed";
        case KnowledgeSource::INFERRED: return "inferred";
        case KnowledgeSource::OBSERVED: return "observed";
        case KnowledgeSource::COMMUNICATED: return "communicated";
        default: return "programmed";
    }
}

KnowledgeSource stringToKnowledgeSource(const std::string& sourceStr) {
    if (sourceStr == "learned") return KnowledgeSource::LEARNED;
    if (sourceStr == "inferred") return KnowledgeSource::INFERRED;
    if (sourceStr == "observed") return KnowledgeSource::OBSERVED;
    if (sourceStr == "communicated") return KnowledgeSource::COMMUNICATED;
    return KnowledgeSource::PROGRAMMED;
}

} // namespace elizaos
