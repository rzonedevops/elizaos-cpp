#include <gtest/gtest.h>
#include "elizaos/the_org.hpp"
#include "elizaos/agentlogger.hpp"

using namespace elizaos;

class TheOrgTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for testing
        auto logger = Logger::getInstance();
        logger->setLevel("INFO");
        
        // Create test agent configurations
        eli5Config.agentId = the_org_utils::generateAgentId(AgentRole::COMMUNITY_MANAGER);
        eli5Config.agentName = "Eli5";
        eli5Config.bio = "Community Manager Agent for testing";
        eli5Config.lore = "Helpful community management agent";
        
        eddyConfig.agentId = the_org_utils::generateAgentId(AgentRole::DEVELOPER_RELATIONS);
        eddyConfig.agentName = "Eddy";
        eddyConfig.bio = "Developer Relations Agent for testing";
        eddyConfig.lore = "Technical support and documentation agent";
        
        jimmyConfig.agentId = the_org_utils::generateAgentId(AgentRole::PROJECT_MANAGER);
        jimmyConfig.agentName = "Jimmy";
        jimmyConfig.bio = "Project Manager Agent for testing";
        jimmyConfig.lore = "Project coordination and team management agent";
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    AgentConfig eli5Config, eddyConfig, jimmyConfig;
};

// ============================================================================
// TheOrgAgent Base Tests
// ============================================================================

TEST_F(TheOrgTest, AgentCreationAndBasicOperations) {
    CommunityManagerAgent eli5(eli5Config);
    
    EXPECT_EQ(eli5.getName(), "Eli5");
    EXPECT_EQ(eli5.getRole(), AgentRole::COMMUNITY_MANAGER);
    EXPECT_FALSE(eli5.isRunning());
    
    // Test memory creation and management
    auto memory = eli5.createMemory("Test memory content", MemoryType::MESSAGE);
    EXPECT_NE(memory, nullptr);
    EXPECT_EQ(memory->getContent(), "Test memory content");
    
    eli5.addMemory(memory);
    auto searchResults = eli5.searchMemories("Test");
    EXPECT_EQ(searchResults.size(), 1);
    EXPECT_EQ(searchResults[0]->getContent(), "Test memory content");
}

TEST_F(TheOrgTest, PlatformManagement) {
    CommunityManagerAgent eli5(eli5Config);
    
    // Add platform configuration
    PlatformConfig discordConfig;
    discordConfig.type = PlatformType::DISCORD;
    discordConfig.applicationId = "test_app_id";
    discordConfig.apiToken = "test_token";
    
    eli5.addPlatform(discordConfig);
    
    // Test message sending (mock)
    bool result = eli5.sendMessage(PlatformType::DISCORD, "test_channel", "Hello, world!");
    EXPECT_TRUE(result);
    
    // Test platform removal
    eli5.removePlatform(PlatformType::DISCORD);
    result = eli5.sendMessage(PlatformType::DISCORD, "test_channel", "Should fail");
    EXPECT_FALSE(result);
}

TEST_F(TheOrgTest, InterAgentCommunication) {
    CommunityManagerAgent eli5(eli5Config);
    
    // Test sending message to another agent
    eli5.sendToAgent("test_agent_id", "Test inter-agent message");
    
    // Test processing incoming message
    eli5.processMessage("Hello from another agent", "sender_id");
    auto messages = eli5.getIncomingMessages();
    EXPECT_FALSE(messages.empty());
    EXPECT_EQ(messages.front(), "From sender_id: Hello from another agent");
}

TEST_F(TheOrgTest, TaskManagement) {
    CommunityManagerAgent eli5(eli5Config);
    
    // Test task creation
    UUID taskId = eli5.createTask("Test Task", "Test task description", 1);
    EXPECT_FALSE(taskId.empty());
    
    // Test task completion
    bool completed = eli5.completeTask(taskId);
    EXPECT_TRUE(completed);
}

TEST_F(TheOrgTest, ConfigurationManagement) {
    CommunityManagerAgent eli5(eli5Config);
    
    // Test configuration updates
    std::unordered_map<std::string, std::string> settings = {
        {"greeting_enabled", "true"},
        {"moderation_level", "strict"}
    };
    
    eli5.updateConfig(settings);
    
    EXPECT_EQ(eli5.getConfigValue("greeting_enabled"), "true");
    EXPECT_EQ(eli5.getConfigValue("moderation_level"), "strict");
    EXPECT_EQ(eli5.getConfigValue("nonexistent_key"), "");
}

// ============================================================================
// CommunityManagerAgent Tests
// ============================================================================

TEST_F(TheOrgTest, CommunityManagerInitialization) {
    CommunityManagerAgent eli5(eli5Config);
    eli5.initialize();
    
    // Test agent lifecycle
    EXPECT_FALSE(eli5.isRunning());
    eli5.start();
    EXPECT_TRUE(eli5.isRunning());
    
    eli5.pause();
    eli5.resume();
    
    eli5.stop();
    EXPECT_FALSE(eli5.isRunning());
}

TEST_F(TheOrgTest, NewUserGreeting) {
    CommunityManagerAgent eli5(eli5Config);
    eli5.initialize();
    
    // Test greeting functionality
    EXPECT_FALSE(eli5.shouldGreetNewUser("test_user"));
    
    eli5.enableNewUserGreeting("general_channel", "Welcome {user} to {server}!");
    EXPECT_TRUE(eli5.shouldGreetNewUser("test_user"));
    
    std::string greeting = eli5.generateGreeting("TestUser", "TestServer");
    EXPECT_EQ(greeting, "Welcome TestUser to TestServer!");
    
    eli5.disableNewUserGreeting();
    EXPECT_FALSE(eli5.shouldGreetNewUser("test_user"));
}

TEST_F(TheOrgTest, ModerationSystem) {
    CommunityManagerAgent eli5(eli5Config);
    eli5.initialize();
    
    // Test moderation rule management
    eli5.addModerationRule("badword", ModerationAction::WARNING, "Inappropriate language");
    
    // Test message evaluation
    bool isAcceptable = eli5.evaluateMessage("This is a normal message", "user1", "channel1");
    EXPECT_TRUE(isAcceptable);
    
    isAcceptable = eli5.evaluateMessage("This contains badword content", "user2", "channel1");
    EXPECT_FALSE(isAcceptable);
    
    // Test rule removal
    eli5.removeModerationRule("badword");
    isAcceptable = eli5.evaluateMessage("This contains badword content", "user3", "channel1");
    EXPECT_TRUE(isAcceptable); // Should pass after rule removal
}

TEST_F(TheOrgTest, CommunityMetrics) {
    CommunityManagerAgent eli5(eli5Config);
    eli5.initialize();
    
    // Test activity tracking
    eli5.trackUserActivity("user1", "message_sent");
    eli5.trackUserActivity("user2", "reaction_added");
    eli5.trackUserActivity("user1", "message_sent");
    
    auto activeUsers = eli5.identifyActiveUsers(std::chrono::hours(24));
    EXPECT_EQ(activeUsers.size(), 2);
    
    auto metrics = eli5.generateCommunityMetrics();
    EXPECT_NE(metrics.lastUpdated, Timestamp{});
    
    auto topTopics = eli5.getTopTopics(std::chrono::hours(24));
    EXPECT_FALSE(topTopics.empty());
}

// ============================================================================
// DeveloperRelationsAgent Tests
// ============================================================================

TEST_F(TheOrgTest, DeveloperRelationsInitialization) {
    DeveloperRelationsAgent eddy(eddyConfig);
    eddy.initialize();
    
    EXPECT_FALSE(eddy.isRunning());
    eddy.start();
    EXPECT_TRUE(eddy.isRunning());
    
    eddy.stop();
    EXPECT_FALSE(eddy.isRunning());
}

TEST_F(TheOrgTest, DocumentationManagement) {
    DeveloperRelationsAgent eddy(eddyConfig);
    eddy.initialize();
    
    // Test documentation indexing
    eddy.indexDocumentation("/docs/core.md", "1.0.0");
    eddy.indexDocumentation("/docs/agents.md", "1.0.0");
    
    auto searchResults = eddy.searchDocumentation("core");
    EXPECT_FALSE(searchResults.empty());
    EXPECT_TRUE(searchResults[0].find("core.md") != std::string::npos);
}

TEST_F(TheOrgTest, CodeExampleGeneration) {
    DeveloperRelationsAgent eddy(eddyConfig);
    eddy.initialize();
    
    // Test code example generation
    std::string cppExample = eddy.generateCodeExample("agent-creation", "cpp");
    EXPECT_FALSE(cppExample.empty());
    EXPECT_TRUE(cppExample.find("AgentConfig") != std::string::npos);
    
    std::string memoryExample = eddy.generateCodeExample("memory-management", "cpp");
    EXPECT_FALSE(memoryExample.empty());
    EXPECT_TRUE(memoryExample.find("createMemory") != std::string::npos);
}

TEST_F(TheOrgTest, TechnicalKnowledgeBase) {
    DeveloperRelationsAgent eddy(eddyConfig);
    eddy.initialize();
    
    // Test knowledge management
    eddy.addTechnicalKnowledge("custom-agents", "How to create custom agent types", {"agents", "customization"});
    
    std::string knowledge = eddy.retrieveKnowledge("custom-agents");
    EXPECT_EQ(knowledge, "How to create custom agent types");
    
    // Test partial matching
    knowledge = eddy.retrieveKnowledge("agents");
    EXPECT_TRUE(knowledge.find("agent") != std::string::npos);
    
    // Test non-existent knowledge
    knowledge = eddy.retrieveKnowledge("nonexistent-topic");
    EXPECT_TRUE(knowledge.find("not found") != std::string::npos);
}

// ============================================================================
// ProjectManagerAgent Tests
// ============================================================================

TEST_F(TheOrgTest, ProjectManagerInitialization) {
    ProjectManagerAgent jimmy(jimmyConfig);
    jimmy.initialize();
    
    EXPECT_FALSE(jimmy.isRunning());
    jimmy.start();
    EXPECT_TRUE(jimmy.isRunning());
    
    jimmy.stop();
    EXPECT_FALSE(jimmy.isRunning());
}

TEST_F(TheOrgTest, ProjectManagement) {
    ProjectManagerAgent jimmy(jimmyConfig);
    jimmy.initialize();
    
    // Test project creation
    UUID projectId = jimmy.createProject("Test Project", "A test project for unit testing");
    EXPECT_FALSE(projectId.empty());
    
    auto project = jimmy.getProject(projectId);
    EXPECT_TRUE(project.has_value());
    EXPECT_EQ(project->name, "Test Project");
    EXPECT_EQ(project->status, ProjectStatus::PLANNING);
    
    auto allProjects = jimmy.getActiveProjects();
    EXPECT_EQ(allProjects.size(), 1);
}

TEST_F(TheOrgTest, TeamMemberManagement) {
    ProjectManagerAgent jimmy(jimmyConfig);
    jimmy.initialize();
    
    // Test team member creation
    TeamMember member;
    member.name = "Test Developer";
    member.role = "Software Engineer";
    member.availability.workDays = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    member.availability.workHours.start = "09:00";
    member.availability.workHours.end = "17:00";
    member.availability.timeZone = "UTC";
    member.availability.hoursPerWeek = 40;
    member.availability.employmentStatus = TeamMemberAvailability::EmploymentStatus::FULL_TIME;
    
    UUID memberId = jimmy.addTeamMember(member);
    EXPECT_FALSE(memberId.empty());
    
    auto retrievedMember = jimmy.getTeamMember(memberId);
    EXPECT_TRUE(retrievedMember.has_value());
    EXPECT_EQ(retrievedMember->name, "Test Developer");
    EXPECT_EQ(retrievedMember->availability.hoursPerWeek, 40);
}

TEST_F(TheOrgTest, DailyUpdatesAndReporting) {
    ProjectManagerAgent jimmy(jimmyConfig);
    jimmy.initialize();
    
    // Create project and team member
    UUID projectId = jimmy.createProject("Test Project", "Test project");
    
    TeamMember member;
    member.name = "Test Developer";
    UUID memberId = jimmy.addTeamMember(member);
    
    // Test daily update recording
    DailyUpdate update;
    update.teamMemberId = memberId;
    update.projectId = projectId;
    update.date = "2024-01-15";
    update.summary = "Worked on unit tests and bug fixes";
    update.accomplishments = {"Fixed memory leak", "Added test coverage"};
    update.blockers = {"Waiting for API documentation"};
    
    jimmy.recordDailyUpdate(update);
    
    auto updates = jimmy.getDailyUpdates(projectId);
    EXPECT_EQ(updates.size(), 1);
    EXPECT_EQ(updates[0].summary, "Worked on unit tests and bug fixes");
    
    // Test report generation
    std::string report = jimmy.generateProjectStatusReport(projectId);
    EXPECT_FALSE(report.empty());
    EXPECT_TRUE(report.find("Test Project") != std::string::npos);
    EXPECT_TRUE(report.find("Planning") != std::string::npos);
}

// ============================================================================
// TheOrgManager Tests
// ============================================================================

TEST_F(TheOrgTest, TheOrgManagerInitialization) {
    TheOrgManager manager;
    
    // Test agent creation and management
    auto eli5 = std::make_shared<CommunityManagerAgent>(eli5Config);
    auto eddy = std::make_shared<DeveloperRelationsAgent>(eddyConfig);
    auto jimmy = std::make_shared<ProjectManagerAgent>(jimmyConfig);
    
    manager.addAgent(eli5);
    manager.addAgent(eddy);
    manager.addAgent(jimmy);
    
    auto allAgents = manager.getAllAgents();
    EXPECT_EQ(allAgents.size(), 3);
    
    auto cmAgent = manager.getAgentByRole(AgentRole::COMMUNITY_MANAGER);
    EXPECT_NE(cmAgent, nullptr);
    EXPECT_EQ(cmAgent->getName(), "Eli5");
}

TEST_F(TheOrgTest, TheOrgManagerCoordination) {
    TheOrgManager manager;
    
    auto eli5 = std::make_shared<CommunityManagerAgent>(eli5Config);
    auto eddy = std::make_shared<DeveloperRelationsAgent>(eddyConfig);
    
    manager.addAgent(eli5);
    manager.addAgent(eddy);
    
    // Test initialization and startup
    std::vector<AgentConfig> configs = {eli5Config, eddyConfig};
    manager.initializeAllAgents(configs);
    
    manager.startAllAgents();
    
    // Test broadcasting
    manager.broadcastMessage("System announcement", "system", {AgentRole::COMMUNITY_MANAGER});
    
    // Test system metrics
    auto metrics = manager.getSystemMetrics();
    EXPECT_EQ(metrics.totalAgents, 2);
    EXPECT_EQ(metrics.activeAgents, 2);
    
    manager.stopAllAgents();
}

// ============================================================================
// Utility Function Tests
// ============================================================================

TEST_F(TheOrgTest, UtilityFunctions) {
    // Test agent ID generation
    std::string cmId = the_org_utils::generateAgentId(AgentRole::COMMUNITY_MANAGER);
    EXPECT_TRUE(cmId.find("cm_") == 0);
    
    std::string drId = the_org_utils::generateAgentId(AgentRole::DEVELOPER_RELATIONS);
    EXPECT_TRUE(drId.find("dr_") == 0);
    
    // Test platform type conversions
    EXPECT_EQ(the_org_utils::platformTypeToString(PlatformType::DISCORD), "Discord");
    EXPECT_EQ(the_org_utils::stringToPlatformType("Discord"), PlatformType::DISCORD);
    
    // Test role conversions
    EXPECT_EQ(the_org_utils::agentRoleToString(AgentRole::COMMUNITY_MANAGER), "Community Manager");
    EXPECT_EQ(the_org_utils::stringToAgentRole("Community Manager"), AgentRole::COMMUNITY_MANAGER);
    
    // Test hashtag parsing
    std::vector<std::string> hashtags = the_org_utils::parseHashtags("Check out #elizaos and #agents for more info!");
    EXPECT_EQ(hashtags.size(), 2);
    EXPECT_EQ(hashtags[0], "#elizaos");
    EXPECT_EQ(hashtags[1], "#agents");
    
    // Test similarity calculation
    std::vector<std::string> list1 = {"a", "b", "c"};
    std::vector<std::string> list2 = {"b", "c", "d"};
    double similarity = the_org_utils::calculateSimilarity(list1, list2);
    EXPECT_GT(similarity, 0.0);
    EXPECT_LT(similarity, 1.0);
    
    // Test text sanitization
    std::string longText(3000, 'a');
    std::string sanitized = the_org_utils::sanitizeForPlatform(longText, PlatformType::DISCORD);
    EXPECT_LT(sanitized.length(), longText.length());
    EXPECT_TRUE(sanitized.find("...") != std::string::npos);
    
    // Test URL validation
    EXPECT_TRUE(the_org_utils::validateUrl("https://example.com"));
    EXPECT_TRUE(the_org_utils::validateUrl("http://test.org/path"));
    EXPECT_FALSE(the_org_utils::validateUrl("not-a-url"));
    
    // Test domain extraction
    EXPECT_EQ(the_org_utils::extractDomain("https://example.com/path"), "example.com");
    EXPECT_EQ(the_org_utils::extractDomain("invalid-url"), "");
    
    // Test text splitting
    std::vector<std::string> parts = the_org_utils::splitText("This is a long text that needs to be split", 10);
    EXPECT_GT(parts.size(), 1);
    for (const auto& part : parts) {
        EXPECT_LE(part.length(), 10);
    }
    
    // Test text joining
    std::vector<std::string> words = {"Hello", "world", "test"};
    std::string joined = the_org_utils::joinText(words, " ");
    EXPECT_EQ(joined, "Hello world test");
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(TheOrgTest, IntegrationWorkflow) {
    // Test a complete workflow with multiple agents
    TheOrgManager manager;
    
    auto eli5 = std::make_shared<CommunityManagerAgent>(eli5Config);
    auto eddy = std::make_shared<DeveloperRelationsAgent>(eddyConfig);
    auto jimmy = std::make_shared<ProjectManagerAgent>(jimmyConfig);
    
    manager.addAgent(eli5);
    manager.addAgent(eddy);
    manager.addAgent(jimmy);
    
    // Initialize and start all agents
    std::vector<AgentConfig> configs = {eli5Config, eddyConfig, jimmyConfig};
    manager.initializeAllAgents(configs);
    manager.startAllAgents();
    
    // Simulate community management workflow
    eli5->enableNewUserGreeting("general", "Welcome {user}!");
    eli5->addModerationRule("spam", ModerationAction::WARNING, "No spam allowed");
    
    // Simulate developer relations workflow
    eddy->indexDocumentation("/docs/getting-started.md", "1.0.0");
    eddy->addTechnicalKnowledge("setup", "How to set up the development environment", {"setup", "dev"});
    
    // Simulate project management workflow
    UUID projectId = jimmy->createProject("Community Platform", "Building the community platform");
    
    TeamMember dev;
    dev.name = "Alice Developer";
    dev.role = "Full Stack Developer";
    UUID devId = jimmy->addTeamMember(dev);
    
    jimmy->addTeamMemberToProject(projectId, devId);
    
    DailyUpdate update;
    update.teamMemberId = devId;
    update.projectId = projectId;
    update.date = "2024-01-15";
    update.summary = "Implemented user authentication system";
    jimmy->recordDailyUpdate(update);
    
    // Test cross-agent communication
    manager.broadcastMessage("Daily standup starting in 5 minutes", jimmy->getId(), 
                           {AgentRole::COMMUNITY_MANAGER, AgentRole::DEVELOPER_RELATIONS});
    
    // Verify system metrics
    auto metrics = manager.getSystemMetrics();
    EXPECT_EQ(metrics.totalAgents, 3);
    EXPECT_EQ(metrics.activeAgents, 3);
    
    // Generate reports
    std::string projectReport = jimmy->generateProjectStatusReport(projectId);
    EXPECT_FALSE(projectReport.empty());
    EXPECT_TRUE(projectReport.find("Community Platform") != std::string::npos);
    
    auto communityMetrics = eli5->generateCommunityMetrics();
    EXPECT_NE(communityMetrics.lastUpdated, Timestamp{});
    
    // Clean shutdown
    manager.stopAllAgents();
}

TEST_F(TheOrgTest, ErrorHandlingAndEdgeCases) {
    // Test error handling scenarios
    CommunityManagerAgent eli5(eli5Config);
    
    // Test with invalid platform
    bool result = eli5.sendMessage(PlatformType::DISCORD, "test", "Should fail - no platform configured");
    EXPECT_FALSE(result);
    
    // Test empty search
    auto results = eli5.searchMemories("nonexistent query");
    EXPECT_TRUE(results.empty());
    
    // Test null/empty configurations
    eli5.updateConfig({});
    EXPECT_EQ(eli5.getConfigValue("any_key"), "");
    
    // Test project manager edge cases
    ProjectManagerAgent jimmy(jimmyConfig);
    auto nonExistentProject = jimmy.getProject("invalid-id");
    EXPECT_FALSE(nonExistentProject.has_value());
    
    auto nonExistentMember = jimmy.getTeamMember("invalid-id");
    EXPECT_FALSE(nonExistentMember.has_value());
    
    // Test developer relations edge cases
    DeveloperRelationsAgent eddy(eddyConfig);
    std::string knowledge = eddy.retrieveKnowledge("completely-unknown-topic");
    EXPECT_TRUE(knowledge.find("not found") != std::string::npos);
}

} // namespace elizaos