#include "elizaos/registry.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace elizaos;

int main() {
    // Initialize logging
    logInfo("Registry Demo Application", "demo");
    
    // Create registry instance
    Registry registry;
    
    // Try to load local registry from the project's registry file
    logInfo("Loading registry from local file...", "demo");
    bool loaded = registry.loadLocalRegistry("../registry/index.json");
    
    if (!loaded) {
        logError("Failed to load registry from local file, trying current directory", "demo");
        loaded = registry.loadLocalRegistry("registry/index.json");
    }
    
    if (!loaded) {
        logError("Could not load local registry file", "demo");
        return 1;
    }
    
    // Display registry information
    logInfo("Registry loaded successfully", "demo");
    logInfo("Total plugins in registry: " + std::to_string(registry.getPluginCount()), "demo");
    logInfo("Last refresh time: " + registry.getLastRefreshTime(), "demo");
    
    // Get all plugins
    auto allPlugins = registry.getAllPlugins();
    logInfo("Listing first 10 plugins:", "demo");
    
    int count = 0;
    for (const auto& plugin : allPlugins) {
        if (count >= 10) break;
        std::cout << "- " << plugin.name << " -> " << plugin.repositoryUrl << std::endl;
        count++;
    }
    
    // Search for specific plugins
    logInfo("Searching for plugins containing 'solana':", "demo");
    auto solanaPlugins = registry.searchPlugins("solana");
    for (const auto& plugin : solanaPlugins) {
        std::cout << "- " << plugin.name << " -> " << plugin.repositoryUrl << std::endl;
    }
    
    // Get specific plugin
    logInfo("Looking for a specific plugin...", "demo");
    auto specificPlugin = registry.getPlugin("@elizaos-plugins/plugin-twitter");
    if (specificPlugin.has_value()) {
        std::cout << "Found plugin: " << specificPlugin->name 
                  << " -> " << specificPlugin->repositoryUrl << std::endl;
    } else {
        logInfo("Twitter plugin not found in registry", "demo");
    }
    
    // Demonstrate async refresh (will fail without network but shows the API)
    logInfo("Attempting to refresh registry from remote source...", "demo");
    auto refreshFuture = registry.refreshRegistry();
    
    // Wait a bit for the async operation
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Check if refresh completed
    if (refreshFuture.wait_for(std::chrono::milliseconds(500)) == std::future_status::ready) {
        bool refreshResult = refreshFuture.get();
        if (refreshResult) {
            logInfo("Registry refreshed from remote source", "demo");
        } else {
            logInfo("Failed to refresh from remote (expected in offline mode)", "demo");
        }
    } else {
        logInfo("Refresh still in progress or timed out", "demo");
    }
    
    // Access the integrated plugin registry
    logInfo("Accessing integrated plugin registry...", "demo");
    PluginRegistry& pluginReg = registry.getPluginRegistry();
    auto activePlugins = pluginReg.getActivePlugins();
    logInfo("Active plugins in plugin registry: " + std::to_string(activePlugins.size()), "demo");
    
    logInfo("Registry demo completed successfully", "demo");
    return 0;
}