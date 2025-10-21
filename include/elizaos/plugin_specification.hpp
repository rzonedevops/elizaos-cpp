#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <any>
#include <mutex>
#include <chrono>
#include "core.hpp"
#include "agentmemory.hpp"

namespace elizaos {

// JsonValue type definition (compatible with other modules)
using JsonValue = std::unordered_map<std::string, std::any>;

// Forward declarations
class PluginInterface;
class PluginManager;
class PluginRegistry;

/**
 * Plugin version information
 */
struct PluginVersion {
    int major = 1;
    int minor = 0;
    int patch = 0;
    std::string prerelease = "";
    std::string build = "";
    
    std::string toString() const;
    bool isCompatibleWith(const PluginVersion& other) const;
    static PluginVersion fromString(const std::string& versionStr);
};

/**
 * Plugin dependency specification
 */
struct PluginDependency {
    std::string pluginName;
    PluginVersion minVersion;
    PluginVersion maxVersion;
    bool required = true;
    
    bool isSatisfiedBy(const PluginVersion& version) const;
};

/**
 * Plugin capability definition
 */
enum class PluginCapability {
    ACTION_PROCESSING,      // Can process agent actions
    MESSAGE_HANDLING,       // Can handle message processing
    KNOWLEDGE_EXPANSION,    // Can expand knowledge base
    CONVERSATION_FLOW,      // Can influence conversation flow
    MEMORY_INTEGRATION,     // Can integrate with memory system
    EXTERNAL_API,          // Provides external API access
    DATA_TRANSFORMATION,   // Can transform data
    AUTHENTICATION,        // Provides authentication services
    ANALYTICS,            // Provides analytics capabilities
    CUSTOM               // Custom capability
};

/**
 * Plugin configuration parameter
 */
struct PluginParameter {
    std::string name;
    std::string description;
    std::string type;           // "string", "int", "float", "bool", "array", "object"
    std::any defaultValue;
    bool required = false;
    std::vector<std::string> allowedValues; // For enum-like parameters
    
    JsonValue toJson() const;
    static PluginParameter fromJson(const JsonValue& json);
};

/**
 * Plugin metadata
 */
struct PluginMetadata {
    std::string name;
    std::string displayName;
    std::string description;
    std::string author;
    std::string website;
    std::string license;
    PluginVersion version;
    std::vector<PluginDependency> dependencies;
    std::vector<PluginCapability> capabilities;
    std::vector<PluginParameter> parameters;
    std::unordered_map<std::string, std::string> customFields;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;
    
    JsonValue toJson() const;
    static PluginMetadata fromJson(const JsonValue& json);
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;
};

/**
 * Plugin execution context
 */
struct PluginContext {
    std::shared_ptr<State> agentState;
    std::shared_ptr<AgentMemoryManager> memory;
    std::unordered_map<std::string, std::any> parameters;
    std::unordered_map<std::string, std::any> sessionData;
    std::string requestId;
    std::chrono::system_clock::time_point timestamp;
    
    // Convenience methods
    template<typename T>
    T getParameter(const std::string& name, const T& defaultValue = T{}) const;
    
    template<typename T>
    void setSessionData(const std::string& key, const T& value);
    
    template<typename T>
    T getSessionData(const std::string& key, const T& defaultValue = T{}) const;
};

/**
 * Plugin execution result
 */
struct PluginResult {
    bool success = true;
    std::string message;
    std::any data;
    std::unordered_map<std::string, std::any> metadata;
    std::chrono::milliseconds executionTime{0};
    
    template<typename T>
    T getData() const;
    
    template<typename T>
    T getMetadata(const std::string& key, const T& defaultValue = T{}) const;
    
    JsonValue toJson() const;
};

/**
 * Plugin hook points for lifecycle events
 */
enum class PluginHook {
    BEFORE_MESSAGE_PROCESSING,
    AFTER_MESSAGE_PROCESSING,
    BEFORE_RESPONSE_GENERATION,
    AFTER_RESPONSE_GENERATION,
    BEFORE_MEMORY_STORAGE,
    AFTER_MEMORY_STORAGE,
    BEFORE_ACTION_EXECUTION,
    AFTER_ACTION_EXECUTION,
    SESSION_START,
    SESSION_END,
    AGENT_STARTUP,
    AGENT_SHUTDOWN
};

/**
 * Base plugin interface that all plugins must implement
 */
class PluginInterface {
public:
    virtual ~PluginInterface() = default;
    
    /**
     * Get plugin metadata
     */
    virtual PluginMetadata getMetadata() const = 0;
    
    /**
     * Initialize the plugin with given parameters
     */
    virtual bool initialize(const std::unordered_map<std::string, std::any>& parameters) = 0;
    
    /**
     * Shutdown the plugin and cleanup resources
     */
    virtual void shutdown() = 0;
    
    /**
     * Execute plugin functionality with given context
     */
    virtual PluginResult execute(const PluginContext& context) = 0;
    
    /**
     * Handle plugin hook events
     */
    virtual PluginResult handleHook(PluginHook hook, const PluginContext& context);
    
    /**
     * Get plugin status and health information
     */
    virtual JsonValue getStatus() const;
    
    /**
     * Validate plugin configuration
     */
    virtual bool validateConfiguration(const std::unordered_map<std::string, std::any>& config) const;
    
    /**
     * Get plugin capabilities
     */
    virtual std::vector<PluginCapability> getCapabilities() const;
    
protected:
    bool initialized_ = false;
    std::chrono::system_clock::time_point lastExecuted_;
    size_t executionCount_ = 0;
    std::chrono::milliseconds totalExecutionTime_{0};
};

/**
 * Plugin discovery and loading system
 */
class PluginRegistry {
public:
    PluginRegistry();
    ~PluginRegistry();
    
    /**
     * Register a plugin instance
     */
    bool registerPlugin(std::shared_ptr<PluginInterface> plugin);
    
    /**
     * Unregister a plugin by name
     */
    bool unregisterPlugin(const std::string& pluginName);
    
    /**
     * Get a plugin by name
     */
    std::shared_ptr<PluginInterface> getPlugin(const std::string& pluginName) const;
    
    /**
     * Get all registered plugins
     */
    std::vector<std::shared_ptr<PluginInterface>> getAllPlugins() const;
    
    /**
     * Find plugins by capability
     */
    std::vector<std::shared_ptr<PluginInterface>> getPluginsByCapability(PluginCapability capability) const;
    
    /**
     * Discover plugins from directory
     */
    std::vector<PluginMetadata> discoverPlugins(const std::string& directory) const;
    
    /**
     * Load plugin from file
     */
    std::shared_ptr<PluginInterface> loadPlugin(const std::string& pluginPath);
    
    /**
     * Validate plugin dependencies
     */
    bool validateDependencies(const PluginMetadata& plugin) const;
    
    /**
     * Get plugin dependency graph
     */
    std::vector<std::string> getDependencyOrder() const;
    
    /**
     * Get registry statistics
     */
    JsonValue getStatistics() const;
    
private:
    std::unordered_map<std::string, std::shared_ptr<PluginInterface>> plugins_;
    mutable std::mutex pluginsMutex_;
    
    bool validatePlugin(std::shared_ptr<PluginInterface> plugin) const;
};

/**
 * Plugin manager for orchestrating plugin lifecycle and execution
 */
class PluginManager {
public:
    PluginManager();
    ~PluginManager();
    
    /**
     * Set plugin registry
     */
    void setRegistry(std::shared_ptr<PluginRegistry> registry);
    
    /**
     * Initialize all plugins
     */
    bool initializeAll(const std::unordered_map<std::string, std::unordered_map<std::string, std::any>>& configurations = {});
    
    /**
     * Shutdown all plugins
     */
    void shutdownAll();
    
    /**
     * Execute hook for all plugins that support it
     */
    std::vector<PluginResult> executeHook(PluginHook hook, const PluginContext& context);
    
    /**
     * Execute specific plugin
     */
    PluginResult executePlugin(const std::string& pluginName, const PluginContext& context);
    
    /**
     * Execute all plugins with specific capability
     */
    std::vector<PluginResult> executeByCapability(PluginCapability capability, const PluginContext& context);
    
    /**
     * Get plugin execution statistics
     */
    JsonValue getExecutionStats() const;
    
    /**
     * Enable/disable plugin
     */
    bool setPluginEnabled(const std::string& pluginName, bool enabled);
    
    /**
     * Check if plugin is enabled
     */
    bool isPluginEnabled(const std::string& pluginName) const;
    
    /**
     * Get plugin configuration
     */
    std::unordered_map<std::string, std::any> getPluginConfiguration(const std::string& pluginName) const;
    
    /**
     * Update plugin configuration
     */
    bool updatePluginConfiguration(const std::string& pluginName, const std::unordered_map<std::string, std::any>& config);
    
private:
    std::shared_ptr<PluginRegistry> registry_;
    std::unordered_map<std::string, bool> enabledPlugins_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::any>> configurations_;
    mutable std::mutex managerMutex_;
    
    // Execution statistics
    std::unordered_map<std::string, size_t> executionCounts_;
    std::unordered_map<std::string, std::chrono::milliseconds> executionTimes_;
    std::unordered_map<std::string, size_t> errorCounts_;
};

/**
 * Plugin factory for creating plugin instances
 */
class PluginFactory {
public:
    using PluginCreator = std::function<std::shared_ptr<PluginInterface>()>;
    
    /**
     * Register a plugin creator function
     */
    static void registerPlugin(const std::string& pluginName, PluginCreator creator);
    
    /**
     * Create plugin instance
     */
    static std::shared_ptr<PluginInterface> createPlugin(const std::string& pluginName);
    
    /**
     * Get all registered plugin names
     */
    static std::vector<std::string> getRegisteredPlugins();
    
private:
    static std::unordered_map<std::string, PluginCreator> creators_;
    static std::mutex creatorsMutex_;
};

/**
 * Macro for easy plugin registration
 */
#define REGISTER_PLUGIN(PluginClass, PluginName) \
    namespace { \
        struct PluginRegistrar_##PluginClass { \
            PluginRegistrar_##PluginClass() { \
                elizaos::PluginFactory::registerPlugin(PluginName, []() -> std::shared_ptr<elizaos::PluginInterface> { \
                    return std::make_shared<PluginClass>(); \
                }); \
            } \
        }; \
        static PluginRegistrar_##PluginClass registrar_##PluginClass; \
    }

/**
 * Base class for simple plugins
 */
class SimplePlugin : public PluginInterface {
public:
    SimplePlugin(const PluginMetadata& metadata);
    
    PluginMetadata getMetadata() const override;
    bool initialize(const std::unordered_map<std::string, std::any>& parameters) override;
    void shutdown() override;
    std::vector<PluginCapability> getCapabilities() const override;
    
protected:
    PluginMetadata metadata_;
    std::unordered_map<std::string, std::any> parameters_;
};

// Template method implementations
template<typename T>
T PluginContext::getParameter(const std::string& name, const T& defaultValue) const {
    auto it = parameters.find(name);
    if (it != parameters.end()) {
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

template<typename T>
void PluginContext::setSessionData(const std::string& key, const T& value) {
    sessionData[key] = value;
}

template<typename T>
T PluginContext::getSessionData(const std::string& key, const T& defaultValue) const {
    auto it = sessionData.find(key);
    if (it != sessionData.end()) {
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

template<typename T>
T PluginResult::getData() const {
    try {
        return std::any_cast<T>(data);
    } catch (const std::bad_any_cast&) {
        return T{};
    }
}

template<typename T>
T PluginResult::getMetadata(const std::string& key, const T& defaultValue) const {
    auto it = metadata.find(key);
    if (it != metadata.end()) {
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

// Utility functions
std::string pluginCapabilityToString(PluginCapability capability);
PluginCapability stringToPluginCapability(const std::string& capabilityStr);
std::string pluginHookToString(PluginHook hook);
PluginHook stringToPluginHook(const std::string& hookStr);

// Global plugin manager instance
extern std::shared_ptr<PluginManager> globalPluginManager;

} // namespace elizaos