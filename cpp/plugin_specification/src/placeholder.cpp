#include "elizaos/plugin_specification.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <dlfcn.h>

namespace elizaos {

// Global plugin manager instance
std::shared_ptr<PluginManager> globalPluginManager = std::make_shared<PluginManager>();

// =====================================================
// PluginVersion Implementation
// =====================================================

std::string PluginVersion::toString() const {
    std::ostringstream oss;
    oss << major << "." << minor << "." << patch;
    if (!prerelease.empty()) {
        oss << "-" << prerelease;
    }
    if (!build.empty()) {
        oss << "+" << build;
    }
    return oss.str();
}

bool PluginVersion::isCompatibleWith(const PluginVersion& other) const {
    // Simple semantic versioning compatibility
    if (major != other.major) {
        return false; // Major version must match
    }
    if (minor < other.minor) {
        return false; // Minor version must be >= required
    }
    return true;
}

PluginVersion PluginVersion::fromString(const std::string& versionStr) {
    PluginVersion version;
    
    // Parse version string (major.minor.patch[-prerelease][+build])
    std::string working = versionStr;
    
    // Extract build metadata
    size_t buildPos = working.find('+');
    if (buildPos != std::string::npos) {
        version.build = working.substr(buildPos + 1);
        working = working.substr(0, buildPos);
    }
    
    // Extract prerelease
    size_t prereleasePos = working.find('-');
    if (prereleasePos != std::string::npos) {
        version.prerelease = working.substr(prereleasePos + 1);
        working = working.substr(0, prereleasePos);
    }
    
    // Parse major.minor.patch
    std::istringstream iss(working);
    std::string part;
    
    if (std::getline(iss, part, '.')) {
        version.major = std::stoi(part);
    }
    if (std::getline(iss, part, '.')) {
        version.minor = std::stoi(part);
    }
    if (std::getline(iss, part)) {
        version.patch = std::stoi(part);
    }
    
    return version;
}

// =====================================================
// PluginDependency Implementation
// =====================================================

bool PluginDependency::isSatisfiedBy(const PluginVersion& version) const {
    return version.isCompatibleWith(minVersion) && 
           (maxVersion.major == 0 || version.major <= maxVersion.major);
}

// =====================================================
// PluginParameter Implementation
// =====================================================

JsonValue PluginParameter::toJson() const {
    JsonValue json;
    json["name"] = std::string(name);
    json["description"] = std::string(description);
    json["type"] = std::string(type);
    json["required"] = std::string(required ? "true" : "false");
    
    // Serialize default value based on type
    if (type == "string") {
        try {
            json["defaultValue"] = std::string(std::any_cast<std::string>(defaultValue));
        } catch (const std::bad_any_cast&) {
            json["defaultValue"] = std::string("");
        }
    } else if (type == "int") {
        try {
            json["defaultValue"] = std::string(std::to_string(std::any_cast<int>(defaultValue)));
        } catch (const std::bad_any_cast&) {
            json["defaultValue"] = std::string("0");
        }
    } else if (type == "bool") {
        try {
            json["defaultValue"] = std::string(std::any_cast<bool>(defaultValue) ? "true" : "false");
        } catch (const std::bad_any_cast&) {
            json["defaultValue"] = std::string("false");
        }
    }
    
    return json;
}

PluginParameter PluginParameter::fromJson(const JsonValue& json) {
    PluginParameter param;
    
    auto getString = [&](const std::string& key) -> std::string {
        auto it = json.find(key);
        if (it != json.end()) {
            try {
                return std::any_cast<std::string>(it->second);
            } catch (const std::bad_any_cast&) {
                return "";
            }
        }
        return "";
    };
    
    param.name = getString("name");
    param.description = getString("description");
    param.type = getString("type");
    param.required = getString("required") == "true";
    
    // Parse default value based on type
    std::string defaultStr = getString("defaultValue");
    if (param.type == "string") {
        param.defaultValue = defaultStr;
    } else if (param.type == "int") {
        param.defaultValue = std::stoi(defaultStr);
    } else if (param.type == "bool") {
        param.defaultValue = defaultStr == "true";
    }
    
    return param;
}

// =====================================================
// PluginMetadata Implementation
// =====================================================

JsonValue PluginMetadata::toJson() const {
    JsonValue json;
    json["name"] = std::string(name);
    json["displayName"] = std::string(displayName);
    json["description"] = std::string(description);
    json["author"] = std::string(author);
    json["website"] = std::string(website);
    json["license"] = std::string(license);
    json["version"] = std::string(version.toString());
    
    return json;
}

PluginMetadata PluginMetadata::fromJson(const JsonValue& json) {
    PluginMetadata metadata;
    
    auto getString = [&](const std::string& key) -> std::string {
        auto it = json.find(key);
        if (it != json.end()) {
            try {
                return std::any_cast<std::string>(it->second);
            } catch (const std::bad_any_cast&) {
                return "";
            }
        }
        return "";
    };
    
    metadata.name = getString("name");
    metadata.displayName = getString("displayName");
    metadata.description = getString("description");
    metadata.author = getString("author");
    metadata.website = getString("website");
    metadata.license = getString("license");
    metadata.version = PluginVersion::fromString(getString("version"));
    
    return metadata;
}

bool PluginMetadata::validate() const {
    return !name.empty() && !version.toString().empty() && !author.empty();
}

std::vector<std::string> PluginMetadata::getValidationErrors() const {
    std::vector<std::string> errors;
    
    if (name.empty()) {
        errors.push_back("Plugin name is required");
    }
    if (author.empty()) {
        errors.push_back("Plugin author is required");
    }
    if (version.major == 0 && version.minor == 0 && version.patch == 0) {
        errors.push_back("Plugin version is required");
    }
    
    return errors;
}

// =====================================================
// PluginResult Implementation
// =====================================================

JsonValue PluginResult::toJson() const {
    JsonValue json;
    json["success"] = std::string(success ? "true" : "false");
    json["message"] = std::string(message);
    json["executionTime"] = std::string(std::to_string(executionTime.count()) + "ms");
    
    return json;
}

// =====================================================
// PluginInterface Implementation
// =====================================================

PluginResult PluginInterface::handleHook(PluginHook hook, const PluginContext& context) {
    // Default implementation - do nothing
    PluginResult result;
    result.success = true;
    result.message = "Hook " + pluginHookToString(hook) + " handled";
    
    // Use context to avoid warning
    if (context.requestId.empty()) {
        result.message += " (no request ID)";
    }
    
    return result;
}

JsonValue PluginInterface::getStatus() const {
    JsonValue status;
    status["initialized"] = std::string(initialized_ ? "true" : "false");
    status["executionCount"] = std::string(std::to_string(executionCount_));
    status["totalExecutionTime"] = std::string(std::to_string(totalExecutionTime_.count()) + "ms");
    
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastExecution = std::chrono::duration_cast<std::chrono::seconds>(now - lastExecuted_).count();
    status["timeSinceLastExecution"] = std::string(std::to_string(timeSinceLastExecution) + "s");
    
    return status;
}

bool PluginInterface::validateConfiguration(const std::unordered_map<std::string, std::any>& config) const {
    // Default implementation - accept any configuration
    return !config.empty() || config.empty(); // Always true, but uses config to avoid warning
}

std::vector<PluginCapability> PluginInterface::getCapabilities() const {
    // Default implementation - return capabilities from metadata
    return getMetadata().capabilities;
}

// =====================================================
// SimplePlugin Implementation
// =====================================================

SimplePlugin::SimplePlugin(const PluginMetadata& metadata) : metadata_(metadata) {}

PluginMetadata SimplePlugin::getMetadata() const {
    return metadata_;
}

bool SimplePlugin::initialize(const std::unordered_map<std::string, std::any>& parameters) {
    parameters_ = parameters;
    initialized_ = true;
    return true;
}

void SimplePlugin::shutdown() {
    initialized_ = false;
    parameters_.clear();
}

std::vector<PluginCapability> SimplePlugin::getCapabilities() const {
    return metadata_.capabilities;
}

// =====================================================
// PluginRegistry Implementation
// =====================================================

PluginRegistry::PluginRegistry() = default;
PluginRegistry::~PluginRegistry() = default;

bool PluginRegistry::registerPlugin(std::shared_ptr<PluginInterface> plugin) {
    if (!validatePlugin(plugin)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    std::string pluginName = plugin->getMetadata().name;
    plugins_[pluginName] = plugin;
    
    return true;
}

bool PluginRegistry::unregisterPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    auto it = plugins_.find(pluginName);
    if (it != plugins_.end()) {
        // Shutdown the plugin
        it->second->shutdown();
        plugins_.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<PluginInterface> PluginRegistry::getPlugin(const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    auto it = plugins_.find(pluginName);
    if (it != plugins_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<PluginInterface>> PluginRegistry::getAllPlugins() const {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    std::vector<std::shared_ptr<PluginInterface>> result;
    
    for (const auto& pair : plugins_) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::vector<std::shared_ptr<PluginInterface>> PluginRegistry::getPluginsByCapability(PluginCapability capability) const {
    std::vector<std::shared_ptr<PluginInterface>> result;
    auto allPlugins = getAllPlugins();
    
    for (const auto& plugin : allPlugins) {
        auto capabilities = plugin->getCapabilities();
        if (std::find(capabilities.begin(), capabilities.end(), capability) != capabilities.end()) {
            result.push_back(plugin);
        }
    }
    
    return result;
}

std::vector<PluginMetadata> PluginRegistry::discoverPlugins(const std::string& directory) const {
    std::vector<PluginMetadata> discovered;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                // Try to load metadata from plugin file
                // This is a simplified implementation
                PluginMetadata metadata;
                metadata.name = entry.path().stem().string();
                metadata.author = "Unknown";
                metadata.version = PluginVersion{1, 0, 0};
                discovered.push_back(metadata);
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Directory doesn't exist or is not accessible
    }
    
    return discovered;
}

std::shared_ptr<PluginInterface> PluginRegistry::loadPlugin(const std::string& pluginPath) {
    // Simplified plugin loading - in a real implementation would use dlopen/dlsym
    // For now, return nullptr to indicate loading not supported
    
    // Use pluginPath to avoid warning
    if (pluginPath.empty()) {
        return nullptr;
    }
    
    return nullptr;
}

bool PluginRegistry::validateDependencies(const PluginMetadata& plugin) const {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    
    for (const auto& dependency : plugin.dependencies) {
        auto it = plugins_.find(dependency.pluginName);
        if (it == plugins_.end()) {
            if (dependency.required) {
                return false; // Required dependency not found
            }
        } else {
            // Check version compatibility
            auto depVersion = it->second->getMetadata().version;
            if (!dependency.isSatisfiedBy(depVersion)) {
                return false;
            }
        }
    }
    
    return true;
}

std::vector<std::string> PluginRegistry::getDependencyOrder() const {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    
    // Simple topological sort for dependency resolution
    std::vector<std::string> order;
    std::unordered_set<std::string> visited;
    
    std::function<void(const std::string&)> visit = [&](const std::string& pluginName) {
        if (visited.find(pluginName) != visited.end()) {
            return;
        }
        
        visited.insert(pluginName);
        
        auto it = plugins_.find(pluginName);
        if (it != plugins_.end()) {
            // Visit dependencies first
            for (const auto& dep : it->second->getMetadata().dependencies) {
                visit(dep.pluginName);
            }
        }
        
        order.push_back(pluginName);
    };
    
    for (const auto& pair : plugins_) {
        visit(pair.first);
    }
    
    return order;
}

JsonValue PluginRegistry::getStatistics() const {
    std::lock_guard<std::mutex> lock(pluginsMutex_);
    
    JsonValue stats;
    stats["totalPlugins"] = std::string(std::to_string(plugins_.size()));
    
    // Count by capability
    std::unordered_map<PluginCapability, int> capabilityCounts;
    for (const auto& pair : plugins_) {
        for (const auto& cap : pair.second->getCapabilities()) {
            capabilityCounts[cap]++;
        }
    }
    
    stats["capabilityCounts"] = std::string(std::to_string(capabilityCounts.size()));
    
    return stats;
}

bool PluginRegistry::validatePlugin(std::shared_ptr<PluginInterface> plugin) const {
    if (!plugin) {
        return false;
    }
    
    auto metadata = plugin->getMetadata();
    return metadata.validate();
}

// =====================================================
// PluginManager Implementation
// =====================================================

PluginManager::PluginManager() = default;
PluginManager::~PluginManager() = default;

void PluginManager::setRegistry(std::shared_ptr<PluginRegistry> registry) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    registry_ = registry;
}

bool PluginManager::initializeAll(const std::unordered_map<std::string, std::unordered_map<std::string, std::any>>& configurations) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    if (!registry_) {
        return false;
    }
    
    auto plugins = registry_->getAllPlugins();
    bool allSuccess = true;
    
    for (const auto& plugin : plugins) {
        std::string pluginName = plugin->getMetadata().name;
        
        // Get configuration for this plugin
        std::unordered_map<std::string, std::any> config;
        auto configIt = configurations.find(pluginName);
        if (configIt != configurations.end()) {
            config = configIt->second;
        }
        
        // Initialize plugin
        bool success = plugin->initialize(config);
        if (success) {
            enabledPlugins_[pluginName] = true;
            configurations_[pluginName] = config;
        } else {
            enabledPlugins_[pluginName] = false;
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

void PluginManager::shutdownAll() {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    if (!registry_) {
        return;
    }
    
    auto plugins = registry_->getAllPlugins();
    for (const auto& plugin : plugins) {
        plugin->shutdown();
        std::string pluginName = plugin->getMetadata().name;
        enabledPlugins_[pluginName] = false;
    }
}

std::vector<PluginResult> PluginManager::executeHook(PluginHook hook, const PluginContext& context) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    std::vector<PluginResult> results;
    
    if (!registry_) {
        return results;
    }
    
    auto plugins = registry_->getAllPlugins();
    for (const auto& plugin : plugins) {
        std::string pluginName = plugin->getMetadata().name;
        
        if (isPluginEnabled(pluginName)) {
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                PluginResult result = plugin->handleHook(hook, context);
                
                auto end = std::chrono::high_resolution_clock::now();
                result.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                results.push_back(result);
                
                // Update statistics
                executionCounts_[pluginName]++;
                executionTimes_[pluginName] += result.executionTime;
                
                if (!result.success) {
                    errorCounts_[pluginName]++;
                }
            } catch (const std::exception&) {
                PluginResult errorResult;
                errorResult.success = false;
                errorResult.message = "Plugin execution failed with exception";
                results.push_back(errorResult);
                
                errorCounts_[pluginName]++;
            }
        }
    }
    
    return results;
}

PluginResult PluginManager::executePlugin(const std::string& pluginName, const PluginContext& context) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    PluginResult result;
    
    if (!registry_) {
        result.success = false;
        result.message = "No plugin registry available";
        return result;
    }
    
    auto plugin = registry_->getPlugin(pluginName);
    if (!plugin) {
        result.success = false;
        result.message = "Plugin not found: " + pluginName;
        return result;
    }
    
    if (!isPluginEnabled(pluginName)) {
        result.success = false;
        result.message = "Plugin is disabled: " + pluginName;
        return result;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    try {
        result = plugin->execute(context);
        
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Update statistics
        executionCounts_[pluginName]++;
        executionTimes_[pluginName] += result.executionTime;
        
        if (!result.success) {
            errorCounts_[pluginName]++;
        }
    } catch (const std::exception&) {
        result.success = false;
        result.message = "Plugin execution failed with exception";
        errorCounts_[pluginName]++;
    }
    
    return result;
}

std::vector<PluginResult> PluginManager::executeByCapability(PluginCapability capability, const PluginContext& context) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    std::vector<PluginResult> results;
    
    if (!registry_) {
        return results;
    }
    
    auto plugins = registry_->getPluginsByCapability(capability);
    for (const auto& plugin : plugins) {
        std::string pluginName = plugin->getMetadata().name;
        
        if (isPluginEnabled(pluginName)) {
            // Unlock temporarily to avoid deadlock in executePlugin
            managerMutex_.unlock();
            PluginResult result = executePlugin(pluginName, context);
            managerMutex_.lock();
            
            results.push_back(result);
        }
    }
    
    return results;
}

JsonValue PluginManager::getExecutionStats() const {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    JsonValue stats;
    stats["totalPlugins"] = std::string(std::to_string(enabledPlugins_.size()));
    
    size_t totalExecutions = 0;
    size_t totalErrors = 0;
    
    for (const auto& pair : executionCounts_) {
        totalExecutions += pair.second;
    }
    
    for (const auto& pair : errorCounts_) {
        totalErrors += pair.second;
    }
    
    stats["totalExecutions"] = std::string(std::to_string(totalExecutions));
    stats["totalErrors"] = std::string(std::to_string(totalErrors));
    
    if (totalExecutions > 0) {
        double errorRate = static_cast<double>(totalErrors) / totalExecutions;
        stats["errorRate"] = std::string(std::to_string(errorRate));
    } else {
        stats["errorRate"] = std::string("0.0");
    }
    
    return stats;
}

bool PluginManager::setPluginEnabled(const std::string& pluginName, bool enabled) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    if (!registry_ || !registry_->getPlugin(pluginName)) {
        return false;
    }
    
    enabledPlugins_[pluginName] = enabled;
    return true;
}

bool PluginManager::isPluginEnabled(const std::string& pluginName) const {
    auto it = enabledPlugins_.find(pluginName);
    return it != enabledPlugins_.end() && it->second;
}

std::unordered_map<std::string, std::any> PluginManager::getPluginConfiguration(const std::string& pluginName) const {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    auto it = configurations_.find(pluginName);
    if (it != configurations_.end()) {
        return it->second;
    }
    
    return {};
}

bool PluginManager::updatePluginConfiguration(const std::string& pluginName, const std::unordered_map<std::string, std::any>& config) {
    std::lock_guard<std::mutex> lock(managerMutex_);
    
    if (!registry_) {
        return false;
    }
    
    auto plugin = registry_->getPlugin(pluginName);
    if (!plugin) {
        return false;
    }
    
    // Validate configuration
    if (!plugin->validateConfiguration(config)) {
        return false;
    }
    
    configurations_[pluginName] = config;
    return true;
}

// =====================================================
// PluginFactory Implementation  
// =====================================================

std::unordered_map<std::string, PluginFactory::PluginCreator> PluginFactory::creators_;
std::mutex PluginFactory::creatorsMutex_;

void PluginFactory::registerPlugin(const std::string& pluginName, PluginCreator creator) {
    std::lock_guard<std::mutex> lock(creatorsMutex_);
    creators_[pluginName] = creator;
}

std::shared_ptr<PluginInterface> PluginFactory::createPlugin(const std::string& pluginName) {
    std::lock_guard<std::mutex> lock(creatorsMutex_);
    
    auto it = creators_.find(pluginName);
    if (it != creators_.end()) {
        return it->second();
    }
    
    return nullptr;
}

std::vector<std::string> PluginFactory::getRegisteredPlugins() {
    std::lock_guard<std::mutex> lock(creatorsMutex_);
    
    std::vector<std::string> plugins;
    for (const auto& pair : creators_) {
        plugins.push_back(pair.first);
    }
    
    return plugins;
}

// =====================================================
// Utility Functions
// =====================================================

std::string pluginCapabilityToString(PluginCapability capability) {
    switch (capability) {
        case PluginCapability::ACTION_PROCESSING: return "action_processing";
        case PluginCapability::MESSAGE_HANDLING: return "message_handling";
        case PluginCapability::KNOWLEDGE_EXPANSION: return "knowledge_expansion";
        case PluginCapability::CONVERSATION_FLOW: return "conversation_flow";
        case PluginCapability::MEMORY_INTEGRATION: return "memory_integration";
        case PluginCapability::EXTERNAL_API: return "external_api";
        case PluginCapability::DATA_TRANSFORMATION: return "data_transformation";
        case PluginCapability::AUTHENTICATION: return "authentication";
        case PluginCapability::ANALYTICS: return "analytics";
        case PluginCapability::CUSTOM: return "custom";
        default: return "unknown";
    }
}

PluginCapability stringToPluginCapability(const std::string& capabilityStr) {
    if (capabilityStr == "action_processing") return PluginCapability::ACTION_PROCESSING;
    if (capabilityStr == "message_handling") return PluginCapability::MESSAGE_HANDLING;
    if (capabilityStr == "knowledge_expansion") return PluginCapability::KNOWLEDGE_EXPANSION;
    if (capabilityStr == "conversation_flow") return PluginCapability::CONVERSATION_FLOW;
    if (capabilityStr == "memory_integration") return PluginCapability::MEMORY_INTEGRATION;
    if (capabilityStr == "external_api") return PluginCapability::EXTERNAL_API;
    if (capabilityStr == "data_transformation") return PluginCapability::DATA_TRANSFORMATION;
    if (capabilityStr == "authentication") return PluginCapability::AUTHENTICATION;
    if (capabilityStr == "analytics") return PluginCapability::ANALYTICS;
    if (capabilityStr == "custom") return PluginCapability::CUSTOM;
    return PluginCapability::CUSTOM;
}

std::string pluginHookToString(PluginHook hook) {
    switch (hook) {
        case PluginHook::BEFORE_MESSAGE_PROCESSING: return "before_message_processing";
        case PluginHook::AFTER_MESSAGE_PROCESSING: return "after_message_processing";
        case PluginHook::BEFORE_RESPONSE_GENERATION: return "before_response_generation";
        case PluginHook::AFTER_RESPONSE_GENERATION: return "after_response_generation";
        case PluginHook::BEFORE_MEMORY_STORAGE: return "before_memory_storage";
        case PluginHook::AFTER_MEMORY_STORAGE: return "after_memory_storage";
        case PluginHook::BEFORE_ACTION_EXECUTION: return "before_action_execution";
        case PluginHook::AFTER_ACTION_EXECUTION: return "after_action_execution";
        case PluginHook::SESSION_START: return "session_start";
        case PluginHook::SESSION_END: return "session_end";
        case PluginHook::AGENT_STARTUP: return "agent_startup";
        case PluginHook::AGENT_SHUTDOWN: return "agent_shutdown";
        default: return "unknown";
    }
}

PluginHook stringToPluginHook(const std::string& hookStr) {
    if (hookStr == "before_message_processing") return PluginHook::BEFORE_MESSAGE_PROCESSING;
    if (hookStr == "after_message_processing") return PluginHook::AFTER_MESSAGE_PROCESSING;
    if (hookStr == "before_response_generation") return PluginHook::BEFORE_RESPONSE_GENERATION;
    if (hookStr == "after_response_generation") return PluginHook::AFTER_RESPONSE_GENERATION;
    if (hookStr == "before_memory_storage") return PluginHook::BEFORE_MEMORY_STORAGE;
    if (hookStr == "after_memory_storage") return PluginHook::AFTER_MEMORY_STORAGE;
    if (hookStr == "before_action_execution") return PluginHook::BEFORE_ACTION_EXECUTION;
    if (hookStr == "after_action_execution") return PluginHook::AFTER_ACTION_EXECUTION;
    if (hookStr == "session_start") return PluginHook::SESSION_START;
    if (hookStr == "session_end") return PluginHook::SESSION_END;
    if (hookStr == "agent_startup") return PluginHook::AGENT_STARTUP;
    if (hookStr == "agent_shutdown") return PluginHook::AGENT_SHUTDOWN;
    return PluginHook::AGENT_STARTUP;
}

} // namespace elizaos
