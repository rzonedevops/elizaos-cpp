#include <gtest/gtest.h>
#include "elizaos/plugins_automation.hpp"
#include "elizaos/discord_summarizer.hpp"
#include "elizaos/discrub_ext.hpp"

using namespace elizaos;

// Test for Plugins Automation Module
class PluginsAutomationTest : public ::testing::Test {
protected:
    void SetUp() override {
        automation = std::make_shared<PluginsAutomation>();
    }
    
    void TearDown() override {
        automation.reset();
    }
    
    std::shared_ptr<PluginsAutomation> automation;
};

TEST_F(PluginsAutomationTest, PluginRegistryBasicOperations) {
    auto& registry = automation->getRegistry();
    
    // Test empty registry
    EXPECT_EQ(registry.getPluginNames().size(), 0);
    EXPECT_EQ(registry.getActivePlugins().size(), 0);
    EXPECT_EQ(registry.getFailedPlugins().size(), 0);
}

TEST_F(PluginsAutomationTest, AutomatedOperations) {
    // Test automated plugin setup
    bool result = automation->automatedPluginSetup("test_plugin", "basic_template");
    EXPECT_TRUE(result);
    
    // Test automated build and test
    result = automation->automatedBuildAndTest("/tmp/test_plugin");
    EXPECT_TRUE(result);
    
    // Test automated deployment
    result = automation->automatedDeployment("test_plugin", "/tmp/deploy");
    EXPECT_TRUE(result);
}

TEST_F(PluginsAutomationTest, ConfigurationManagement) {
    // Test configuration loading and saving
    EXPECT_NO_THROW(automation->loadConfiguration("/tmp/test_config.conf"));
    EXPECT_NO_THROW(automation->saveConfiguration("/tmp/test_config.conf"));
}

// Test for Discord Summarizer Module
class DiscordSummarizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        summarizer = std::make_shared<DiscordSummarizer>();
    }
    
    void TearDown() override {
        summarizer.reset();
    }
    
    std::shared_ptr<DiscordSummarizer> summarizer;
};

TEST_F(DiscordSummarizerTest, InitializationWithToken) {
    // Test initialization with valid token
    bool result = summarizer->initializeWithToken("test_token_123");
    EXPECT_TRUE(result);
    
    // Test initialization with empty token
    result = summarizer->initializeWithToken("");
    EXPECT_FALSE(result);
}

TEST_F(DiscordSummarizerTest, ChannelSummaryGeneration) {
    // Initialize first
    summarizer->initializeWithToken("test_token");
    
    auto now = std::chrono::system_clock::now();
    auto yesterday = now - std::chrono::hours(24);
    
    // Generate channel summary
    auto future = summarizer->generateChannelSummary("test_channel_123", yesterday, now);
    auto summary = future.get();
    
    EXPECT_EQ(summary.channelId, "test_channel_123");
    EXPECT_GT(summary.totalMessages, 0);
    EXPECT_GT(summary.uniqueUsers, 0);
    EXPECT_FALSE(summary.topUsers.empty());
}

TEST_F(DiscordSummarizerTest, MessageAnalyzer) {
    auto& analyzer = summarizer->getAnalyzer();
    
    // Create test message
    DiscordMessage message;
    message.id = "test_msg_123";
    message.content = "This is a great message about AI and machine learning!";
    message.authorName = "TestUser";
    
    // Analyze message
    auto analysis = analyzer.analyzeMessage(message);
    
    EXPECT_EQ(analysis.messageId, "test_msg_123");
    EXPECT_GT(analysis.sentiment, 0.0); // Should be positive due to "great"
    EXPECT_FALSE(analysis.keywords.empty());
    EXPECT_EQ(analysis.language, "en");
    EXPECT_FALSE(analysis.containsSpam);
}

TEST_F(DiscordSummarizerTest, MonitoringControl) {
    // Test monitoring start/stop
    EXPECT_FALSE(summarizer->isMonitoring());
    
    std::vector<std::string> channels = {"channel1", "channel2", "channel3"};
    summarizer->startMonitoring(channels);
    EXPECT_TRUE(summarizer->isMonitoring());
    
    summarizer->stopMonitoring();
    EXPECT_FALSE(summarizer->isMonitoring());
}

// Test for Discrub Extension Module
class DiscrubExtensionTest : public ::testing::Test {
protected:
    void SetUp() override {
        extension = std::make_shared<DiscrubExtension>();
    }
    
    void TearDown() override {
        extension.reset();
    }
    
    std::shared_ptr<DiscrubExtension> extension;
};

TEST_F(DiscrubExtensionTest, ContentScannerBasicOperations) {
    auto& scanner = extension->getScanner();
    
    // Test clean content
    auto result = scanner.scanContent("Hello, this is a normal message.");
    EXPECT_FALSE(result.violation);
    EXPECT_EQ(result.totalSeverity, 0);
    EXPECT_EQ(result.recommendedAction, FilterAction::NONE);
    
    // Test content with profanity
    result = scanner.scanContent("This damn message contains mild profanity.");
    EXPECT_TRUE(result.violation);
    EXPECT_GT(result.totalSeverity, 0);
    EXPECT_NE(result.recommendedAction, FilterAction::NONE);
}

TEST_F(DiscrubExtensionTest, FilterManagement) {
    auto& scanner = extension->getScanner();
    
    // Get initial filter count
    auto initialFilters = scanner.getFilters();
    size_t initialCount = initialFilters.size();
    
    // Add custom filter
    ContentFilter customFilter("test_filter", "\\btestword\\b", FilterAction::WARN, 2);
    scanner.addFilter(customFilter);
    
    // Check filter was added
    auto updatedFilters = scanner.getFilters();
    EXPECT_EQ(updatedFilters.size(), initialCount + 1);
    
    // Test the custom filter
    auto result = scanner.scanContent("This message contains testword!");
    EXPECT_TRUE(result.violation);
    EXPECT_TRUE(std::find(result.triggeredFilters.begin(), result.triggeredFilters.end(), 
                         "test_filter") != result.triggeredFilters.end());
    
    // Remove the filter
    scanner.removeFilter("test_filter");
    auto finalFilters = scanner.getFilters();
    EXPECT_EQ(finalFilters.size(), initialCount);
}

TEST_F(DiscrubExtensionTest, FilterEnableDisable) {
    auto& scanner = extension->getScanner();
    
    // Test enabling/disabling filters
    EXPECT_NO_THROW(scanner.enableProfanityFilter(false));
    EXPECT_NO_THROW(scanner.enableSpamFilter(false));
    EXPECT_NO_THROW(scanner.enablePhishingFilter(false));
    EXPECT_NO_THROW(scanner.enableInviteFilter(false));
    EXPECT_NO_THROW(scanner.enableMentionSpamFilter(false));
    
    // Re-enable filters
    EXPECT_NO_THROW(scanner.enableProfanityFilter(true));
    EXPECT_NO_THROW(scanner.enableSpamFilter(true));
    EXPECT_NO_THROW(scanner.enablePhishingFilter(true));
    EXPECT_NO_THROW(scanner.enableInviteFilter(true));
    EXPECT_NO_THROW(scanner.enableMentionSpamFilter(true, 5));
}

TEST_F(DiscrubExtensionTest, DefaultModerationSettings) {
    // Test applying default settings
    EXPECT_NO_THROW(extension->setDefaultModerationSettings());
}

TEST_F(DiscrubExtensionTest, MonitoringOperations) {
    // Test monitoring without Discord client
    EXPECT_FALSE(extension->initializeWithDiscord(nullptr));
    
    // Test monitoring control
    EXPECT_FALSE(extension->isMonitoring());
    
    std::vector<std::string> channels = {"channel1", "channel2"};
    extension->startMonitoring(channels);
    EXPECT_TRUE(extension->isMonitoring());
    
    extension->stopMonitoring();
    EXPECT_FALSE(extension->isMonitoring());
}

TEST_F(DiscrubExtensionTest, MessageProcessing) {
    // Create test message
    DiscordMessage message;
    message.id = "test_msg_456";
    message.content = "Normal message content";
    message.authorName = "TestUser";
    message.channelId = "test_channel";
    
    // Test message processing
    EXPECT_NO_THROW(extension->processIncomingMessage(message));
    EXPECT_NO_THROW(extension->processMessageEdit(message, message));
    EXPECT_NO_THROW(extension->processMessageDelete("test_channel", "test_msg_456"));
}

TEST_F(DiscrubExtensionTest, ConfigurationManagement) {
    // Test configuration operations
    EXPECT_NO_THROW(extension->loadConfiguration("/tmp/discrub_config.conf"));
    EXPECT_NO_THROW(extension->saveConfiguration("/tmp/discrub_config.conf"));
}

// Integration test for all modules
class Stage6IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize all modules
        automation = std::make_shared<PluginsAutomation>();
        summarizer = std::make_shared<DiscordSummarizer>();
        extension = std::make_shared<DiscrubExtension>();
    }
    
    void TearDown() override {
        automation.reset();
        summarizer.reset();
        extension.reset();
    }
    
    std::shared_ptr<PluginsAutomation> automation;
    std::shared_ptr<DiscordSummarizer> summarizer;
    std::shared_ptr<DiscrubExtension> extension;
};

TEST_F(Stage6IntegrationTest, GlobalInstancesAccessible) {
    // Test that global instances are accessible
    EXPECT_NE(globalPluginAutomation, nullptr);
    EXPECT_NE(globalDiscordSummarizer, nullptr);
    EXPECT_NE(globalDiscrubExtension, nullptr);
}

TEST_F(Stage6IntegrationTest, ModulesWorkTogether) {
    // Initialize Discord summarizer
    bool result = summarizer->initializeWithToken("test_token");
    EXPECT_TRUE(result);
    
    // Initialize discrub extension with Discord client
    auto& client = summarizer->getClient();
    extension->initializeWithDiscord(std::shared_ptr<DiscordClient>(&client, [](DiscordClient*){}));
    
    // Test that both can process messages
    DiscordMessage testMessage;
    testMessage.id = "integration_test_msg";
    testMessage.content = "This is a test message for integration testing.";
    testMessage.authorName = "IntegrationTester";
    testMessage.channelId = "integration_channel";
    
    // Process with both modules
    EXPECT_NO_THROW(extension->processIncomingMessage(testMessage));
    
    // Test plugin automation
    EXPECT_TRUE(automation->automatedPluginSetup("integration_plugin", "test_template"));
}