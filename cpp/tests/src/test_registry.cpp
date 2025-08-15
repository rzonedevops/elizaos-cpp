#include <gtest/gtest.h>
#include "elizaos/registry.hpp"
#include "elizaos/agentlogger.hpp"
#include <fstream>
#include <filesystem>

using namespace elizaos;

class RegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test registry file
        testRegistryContent = R"({
            "@elizaos/plugin-test1": "github:elizaos-plugins/plugin-test1",
            "@elizaos/plugin-test2": "github:elizaos-plugins/plugin-test2",
            "__v2": {
                "version": "2.0.0",
                "packages": {
                    "@elizaos/plugin-v2": "packages/plugin-v2.json"
                }
            }
        })";
        
        testRegistryFile = "test_registry.json";
        std::ofstream file(testRegistryFile);
        file << testRegistryContent;
        file.close();
    }
    
    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(testRegistryFile)) {
            std::filesystem::remove(testRegistryFile);
        }
    }
    
    std::string testRegistryContent;
    std::string testRegistryFile;
};

TEST_F(RegistryTest, DefaultConstructor) {
    Registry registry;
    EXPECT_EQ(registry.getPluginCount(), 0);
    EXPECT_EQ(registry.getLastRefreshTime(), "");
}

TEST_F(RegistryTest, LoadLocalRegistry) {
    Registry registry;
    
    bool loaded = registry.loadLocalRegistry(testRegistryFile);
    EXPECT_TRUE(loaded);
    EXPECT_GT(registry.getPluginCount(), 0);
    
    auto plugins = registry.getAllPlugins();
    EXPECT_GE(plugins.size(), 2); // Should have at least the two test plugins
    
    // Check that specific plugins exist
    auto plugin1 = registry.getPlugin("@elizaos/plugin-test1");
    EXPECT_TRUE(plugin1.has_value());
    EXPECT_EQ(plugin1->name, "@elizaos/plugin-test1");
    EXPECT_EQ(plugin1->repositoryUrl, "github:elizaos-plugins/plugin-test1");
}

TEST_F(RegistryTest, SearchPlugins) {
    Registry registry;
    registry.loadLocalRegistry(testRegistryFile);
    
    // Search for plugins containing "test1"
    auto results = registry.searchPlugins("test1");
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].name, "@elizaos/plugin-test1");
    
    // Search for plugins containing "test" (should match both)
    results = registry.searchPlugins("test");
    EXPECT_GE(results.size(), 2);
    
    // Search for non-existent plugin
    results = registry.searchPlugins("nonexistent");
    EXPECT_EQ(results.size(), 0);
}

TEST_F(RegistryTest, GetPlugin) {
    Registry registry;
    registry.loadLocalRegistry(testRegistryFile);
    
    // Get existing plugin
    auto plugin = registry.getPlugin("@elizaos/plugin-test1");
    EXPECT_TRUE(plugin.has_value());
    EXPECT_EQ(plugin->name, "@elizaos/plugin-test1");
    
    // Get non-existent plugin
    plugin = registry.getPlugin("nonexistent");
    EXPECT_FALSE(plugin.has_value());
}

TEST_F(RegistryTest, RegistryConfig) {
    RegistryConfig config;
    config.cacheDirectory = "/tmp/test_cache";
    config.cacheTtlSeconds = 7200;
    config.enableRemoteRegistry = false;
    
    Registry registry(config);
    
    const auto& retrievedConfig = registry.getConfig();
    EXPECT_EQ(retrievedConfig.cacheDirectory, "/tmp/test_cache");
    EXPECT_EQ(retrievedConfig.cacheTtlSeconds, 7200);
    EXPECT_FALSE(retrievedConfig.enableRemoteRegistry);
}

TEST_F(RegistryTest, PluginRegistryIntegration) {
    Registry registry;
    
    // Access the internal plugin registry
    PluginRegistry& pluginReg = registry.getPluginRegistry();
    
    // This is mainly checking that the integration works
    // Actual plugin loading would require more complex setup
    auto activePlugins = pluginReg.getActivePlugins();
    EXPECT_EQ(activePlugins.size(), 0); // No plugins loaded initially
}

TEST_F(RegistryTest, GlobalRegistryAccess) {
    // Test global registry access
    Registry& global = getGlobalRegistry();
    EXPECT_EQ(global.getPluginCount(), 0);
    
    // Set custom global registry
    auto customRegistry = std::make_unique<Registry>();
    customRegistry->loadLocalRegistry(testRegistryFile);
    size_t pluginCount = customRegistry->getPluginCount();
    
    setGlobalRegistry(std::move(customRegistry));
    Registry& newGlobal = getGlobalRegistry();
    EXPECT_EQ(newGlobal.getPluginCount(), pluginCount);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}