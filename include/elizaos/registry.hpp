#pragma once

#include "elizaos/core.hpp"
#include "elizaos/plugins_automation.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <future>
#include <optional>

namespace elizaos {

/**
 * Registry system for plugin discovery and management
 * Provides access to plugin repositories and metadata
 */

// Registry entry representing a plugin in the registry
struct RegistryEntry {
    std::string name;
    std::string repositoryUrl;
    std::string description;
    std::string version;
    std::string author;
    std::vector<std::string> tags;
    
    RegistryEntry() = default;
    RegistryEntry(const std::string& n, const std::string& repoUrl)
        : name(n), repositoryUrl(repoUrl) {}
};

// Registry configuration
struct RegistryConfig {
    std::string registryUrl;
    std::string cacheDirectory;
    int cacheTtlSeconds;
    bool enableRemoteRegistry;
    
    RegistryConfig()
        : registryUrl("https://raw.githubusercontent.com/elizaos-plugins/registry/refs/heads/main/index.json")
        , cacheDirectory("~/.elizaos/registry_cache")
        , cacheTtlSeconds(3600)  // 1 hour
        , enableRemoteRegistry(true) {}
};

// Main registry class for plugin discovery and management
class Registry {
public:
    Registry();
    explicit Registry(const RegistryConfig& config);
    ~Registry();
    
    // Registry data access
    std::future<bool> refreshRegistry();
    bool loadLocalRegistry(const std::string& registryFilePath = "");
    std::vector<RegistryEntry> getAllPlugins() const;
    std::vector<RegistryEntry> searchPlugins(const std::string& query) const;
    std::optional<RegistryEntry> getPlugin(const std::string& name) const;
    
    // Plugin management integration
    PluginRegistry& getPluginRegistry() { return pluginRegistry_; }
    const PluginRegistry& getPluginRegistry() const { return pluginRegistry_; }
    
    // Registry information
    bool isRegistryCached() const;
    std::string getLastRefreshTime() const;
    size_t getPluginCount() const;
    
    // Configuration
    void setConfig(const RegistryConfig& config);
    const RegistryConfig& getConfig() const { return config_; }

private:
    RegistryConfig config_;
    std::unordered_map<std::string, RegistryEntry> entries_;
    PluginRegistry pluginRegistry_;
    mutable std::mutex registryMutex_;
    std::string lastRefreshTime_;
    
    // Internal methods
    bool parseRegistryJson(const std::string& jsonContent);
    bool loadFromCache();
    bool saveToCache() const;
    std::string getCacheFilePath() const;
    bool isHttpUrl(const std::string& url) const;
    std::string downloadRegistryData(const std::string& url) const;
    std::string expandPath(const std::string& path) const;
    void updateLastRefreshTime();
};

// Global registry instance access
Registry& getGlobalRegistry();
void setGlobalRegistry(std::unique_ptr<Registry> registry);

} // namespace elizaos