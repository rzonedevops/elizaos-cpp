#include "elizaos/the_org.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace elizaos;

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

void demonstrateCommunityManager() {
    printSeparator("ELI5 - COMMUNITY MANAGER AGENT DEMO");
    
    // Create agent configuration
    AgentConfig config;
    config.agentId = the_org_utils::generateAgentId(AgentRole::COMMUNITY_MANAGER);
    config.agentName = "Eli5";
    config.bio = "Friendly community manager who welcomes new users and moderates discussions";
    config.lore = "Expert at building positive community environments";
    
    // Create and initialize the agent
    CommunityManagerAgent eli5(config);
    eli5.initialize();
    
    std::cout << "🤖 Agent Created: " << eli5.getName() << " (ID: " << eli5.getId() << ")" << std::endl;
    std::cout << "📋 Role: " << the_org_utils::agentRoleToString(eli5.getRole()) << std::endl;
    std::cout << "📖 Bio: " << config.bio << std::endl;
    
    // Add platform configuration
    PlatformConfig discordConfig;
    discordConfig.type = PlatformType::DISCORD;
    discordConfig.applicationId = "demo_discord_app_id";
    discordConfig.apiToken = "demo_discord_token";
    eli5.addPlatform(discordConfig);
    
    std::cout << "\n✅ Added Discord platform integration" << std::endl;
    
    // Configure new user greeting
    eli5.enableNewUserGreeting("general", "Welcome to our amazing community, {user}! 🎉 Feel free to introduce yourself!");
    std::cout << "✅ Enabled new user greeting system" << std::endl;
    
    // Set up moderation rules
    eli5.addModerationRule("spam", ModerationAction::WARNING, "Spam content detected");
    eli5.addModerationRule("toxic", ModerationAction::TIMEOUT, "Toxic behavior not tolerated");
    eli5.addModerationRule("harassment", ModerationAction::BAN, "Harassment will result in ban");
    std::cout << "✅ Configured moderation rules" << std::endl;
    
    // Demonstrate greeting generation
    std::cout << "\n🎯 GREETING DEMONSTRATIONS:" << std::endl;
    std::cout << "New user 'Alice' joins:" << std::endl;
    std::cout << "  " << eli5.generateGreeting("Alice", "ElizaOS Community") << std::endl;
    
    std::cout << "New user 'Bob' joins:" << std::endl;
    std::cout << "  " << eli5.generateGreeting("Bob", "ElizaOS Community") << std::endl;
    
    // Demonstrate moderation
    std::cout << "\n🛡️  MODERATION DEMONSTRATIONS:" << std::endl;
    std::vector<std::pair<std::string, std::string>> testMessages = {
        {"user1", "Hello everyone, excited to be here!"},
        {"user2", "Check out this spam link for free crypto!"},
        {"user3", "This is some toxic garbage content"},
        {"user4", "Thanks for the helpful documentation!"}
    };
    
    for (const auto& [user, message] : testMessages) {
        bool acceptable = eli5.evaluateMessage(message, user, "general");
        std::cout << "  📝 \"" << message << "\" - " 
                  << (acceptable ? "✅ APPROVED" : "❌ MODERATED") << std::endl;
    }
    
    // Track user activity
    std::cout << "\n📊 ACTIVITY TRACKING:" << std::endl;
    eli5.trackUserActivity("alice", "message_sent");
    eli5.trackUserActivity("bob", "reaction_added");
    eli5.trackUserActivity("charlie", "voice_joined");
    eli5.trackUserActivity("alice", "message_sent");
    
    auto activeUsers = eli5.identifyActiveUsers(std::chrono::hours(24));
    std::cout << "  Active users in last 24 hours: " << activeUsers.size() << std::endl;
    for (const auto& user : activeUsers) {
        std::cout << "    - " << user << std::endl;
    }
    
    // Generate community metrics
    auto metrics = eli5.generateCommunityMetrics();
    std::cout << "\n📈 COMMUNITY METRICS:" << std::endl;
    std::cout << "  Total Members: " << metrics.totalMembers << std::endl;
    std::cout << "  Active Members: " << metrics.activeMembers << std::endl;
    std::cout << "  Messages/Day: " << metrics.messagesPerDay << std::endl;
    std::cout << "  Engagement Rate: " << (metrics.engagementRate * 100) << "%" << std::endl;
    
    auto topTopics = eli5.getTopTopics(std::chrono::hours(24));
    std::cout << "  Top Discussion Topics:" << std::endl;
    for (size_t i = 0; i < std::min(topTopics.size(), size_t(3)); ++i) {
        std::cout << "    " << (i + 1) << ". " << topTopics[i] << std::endl;
    }
}

void demonstrateDeveloperRelations() {
    printSeparator("EDDY - DEVELOPER RELATIONS AGENT DEMO");
    
    // Create agent configuration
    AgentConfig config;
    config.agentId = the_org_utils::generateAgentId(AgentRole::DEVELOPER_RELATIONS);
    config.agentName = "Eddy";
    config.bio = "Technical support specialist providing documentation and code examples";
    config.lore = "Expert at helping developers understand and use the platform";
    
    // Create and initialize the agent
    DeveloperRelationsAgent eddy(config);
    eddy.initialize();
    
    std::cout << "🤖 Agent Created: " << eddy.getName() << " (ID: " << eddy.getId() << ")" << std::endl;
    std::cout << "📋 Role: " << the_org_utils::agentRoleToString(eddy.getRole()) << std::endl;
    std::cout << "📖 Bio: " << config.bio << std::endl;
    
    // Index documentation
    std::cout << "\n📚 DOCUMENTATION INDEXING:" << std::endl;
    eddy.indexDocumentation("/docs/getting-started.md", "1.0.0");
    eddy.indexDocumentation("/docs/api-reference.md", "1.0.0");
    eddy.indexDocumentation("/docs/agent-development.md", "1.0.0");
    eddy.indexDocumentation("/docs/plugin-system.md", "1.0.0");
    std::cout << "✅ Indexed 4 documentation files" << std::endl;
    
    // Add technical knowledge
    std::cout << "\n🧠 KNOWLEDGE BASE SETUP:" << std::endl;
    eddy.addTechnicalKnowledge("agent-creation", 
        "Agents are created using AgentConfig and can be specialized for different roles", 
        {"agents", "core", "development"});
    eddy.addTechnicalKnowledge("memory-system", 
        "ElizaOS uses a sophisticated memory system with embeddings for context retrieval", 
        {"memory", "embeddings", "context"});
    eddy.addTechnicalKnowledge("plugin-development", 
        "Plugins extend agent capabilities and can be written in TypeScript or C++", 
        {"plugins", "extensions", "development"});
    std::cout << "✅ Added 3 knowledge base entries" << std::endl;
    
    // Demonstrate documentation search
    std::cout << "\n🔍 DOCUMENTATION SEARCH:" << std::endl;
    auto searchResults = eddy.searchDocumentation("agent");
    std::cout << "  Search for 'agent' found " << searchResults.size() << " results:" << std::endl;
    for (const auto& result : searchResults) {
        std::cout << "    - " << result << std::endl;
    }
    
    // Demonstrate knowledge retrieval
    std::cout << "\n🎯 KNOWLEDGE RETRIEVAL:" << std::endl;
    std::vector<std::string> queries = {"agent-creation", "memory-system", "plugins", "nonexistent-topic"};
    for (const auto& query : queries) {
        std::string knowledge = eddy.retrieveKnowledge(query);
        std::cout << "  Q: \"" << query << "\"" << std::endl;
        std::cout << "  A: " << knowledge.substr(0, 80) << (knowledge.length() > 80 ? "..." : "") << std::endl << std::endl;
    }
    
    // Demonstrate code example generation
    std::cout << "\n💻 CODE EXAMPLE GENERATION:" << std::endl;
    std::cout << "Agent Creation Example (C++):" << std::endl;
    std::string agentExample = eddy.generateCodeExample("agent-creation", "cpp");
    std::cout << agentExample << std::endl;
    
    std::cout << "\nMemory Management Example (C++):" << std::endl;
    std::string memoryExample = eddy.generateCodeExample("memory-management", "cpp");
    std::cout << memoryExample << std::endl;
    
    // Demonstrate technical assistance
    std::cout << "\n🆘 TECHNICAL ASSISTANCE SIMULATION:" << std::endl;
    std::vector<std::string> techQuestions = {
        "How do I create a new agent?",
        "What's the best way to handle memory in agents?",
        "Can you show me documentation about plugins?",
        "I'm getting a compilation error with my agent code"
    };
    
    for (const auto& question : techQuestions) {
        std::cout << "  Developer asks: \"" << question << "\"" << std::endl;
        // In a real scenario, this would trigger the processQuestion method
        if (question.find("agent") != std::string::npos) {
            std::cout << "  Eddy responds: Here's how to create agents..." << std::endl;
        } else if (question.find("memory") != std::string::npos) {
            std::cout << "  Eddy responds: For memory management, use the createMemory() method..." << std::endl;
        } else if (question.find("documentation") != std::string::npos || question.find("plugins") != std::string::npos) {
            std::cout << "  Eddy responds: Check out the plugin documentation..." << std::endl;
        } else {
            std::cout << "  Eddy responds: I'd be happy to help debug that error!" << std::endl;
        }
        std::cout << std::endl;
    }
}

void demonstrateProjectManager() {
    printSeparator("JIMMY - PROJECT MANAGER AGENT DEMO");
    
    // Create agent configuration
    AgentConfig config;
    config.agentId = the_org_utils::generateAgentId(AgentRole::PROJECT_MANAGER);
    config.agentName = "Jimmy";
    config.bio = "Project coordination specialist managing teams and tracking progress";
    config.lore = "Expert at keeping projects on track and teams productive";
    
    // Create and initialize the agent
    ProjectManagerAgent jimmy(config);
    jimmy.initialize();
    
    std::cout << "🤖 Agent Created: " << jimmy.getName() << " (ID: " << jimmy.getId() << ")" << std::endl;
    std::cout << "📋 Role: " << the_org_utils::agentRoleToString(jimmy.getRole()) << std::endl;
    std::cout << "📖 Bio: " << config.bio << std::endl;
    
    // Create team members
    std::cout << "\n👥 TEAM MEMBER SETUP:" << std::endl;
    
    TeamMember alice;
    alice.name = "Alice Johnson";
    alice.role = "Senior Frontend Developer";
    alice.availability.workDays = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    alice.availability.workHours.start = "09:00";
    alice.availability.workHours.end = "17:00";
    alice.availability.timeZone = "EST";
    alice.availability.hoursPerWeek = 40;
    alice.availability.employmentStatus = TeamMemberAvailability::EmploymentStatus::FULL_TIME;
    alice.skills = {"React", "TypeScript", "CSS", "UI/UX"};
    
    UUID aliceId = jimmy.addTeamMember(alice);
    std::cout << "✅ Added team member: " << alice.name << " (" << alice.role << ")" << std::endl;
    
    TeamMember bob;
    bob.name = "Bob Smith";
    bob.role = "Backend Developer";
    bob.availability.workDays = {"Monday", "Tuesday", "Wednesday", "Thursday"};
    bob.availability.workHours.start = "10:00";
    bob.availability.workHours.end = "18:00";
    bob.availability.timeZone = "PST";
    bob.availability.hoursPerWeek = 32;
    bob.availability.employmentStatus = TeamMemberAvailability::EmploymentStatus::PART_TIME;
    bob.skills = {"Node.js", "Python", "PostgreSQL", "API Design"};
    
    UUID bobId = jimmy.addTeamMember(bob);
    std::cout << "✅ Added team member: " << bob.name << " (" << bob.role << ")" << std::endl;
    
    TeamMember charlie;
    charlie.name = "Charlie Davis";
    charlie.role = "DevOps Engineer";
    charlie.availability.workDays = {"Monday", "Wednesday", "Friday"};
    charlie.availability.workHours.start = "08:00";
    charlie.availability.workHours.end = "16:00";
    charlie.availability.timeZone = "UTC";
    charlie.availability.hoursPerWeek = 24;
    charlie.availability.employmentStatus = TeamMemberAvailability::EmploymentStatus::FREELANCE;
    charlie.skills = {"Docker", "Kubernetes", "AWS", "CI/CD"};
    
    UUID charlieId = jimmy.addTeamMember(charlie);
    std::cout << "✅ Added team member: " << charlie.name << " (" << charlie.role << ")" << std::endl;
    
    // Create projects
    std::cout << "\n📁 PROJECT SETUP:" << std::endl;
    
    UUID webAppProject = jimmy.createProject(
        "Community Web Application", 
        "Building a modern web application for community management",
        {aliceId, bobId}
    );
    std::cout << "✅ Created project: Community Web Application" << std::endl;
    
    UUID infrastructureProject = jimmy.createProject(
        "Infrastructure Modernization",
        "Updating deployment pipeline and cloud infrastructure",
        {charlieId, bobId}
    );
    std::cout << "✅ Created project: Infrastructure Modernization" << std::endl;
    
    // Record daily updates
    std::cout << "\n📝 DAILY UPDATES:" << std::endl;
    
    DailyUpdate aliceUpdate;
    aliceUpdate.teamMemberId = aliceId;
    aliceUpdate.projectId = webAppProject;
    aliceUpdate.date = "2024-01-15";
    aliceUpdate.summary = "Implemented user authentication flow and responsive navigation";
    aliceUpdate.accomplishments = {
        "Completed login/logout functionality",
        "Added responsive navigation menu",
        "Fixed CSS styling issues on mobile"
    };
    aliceUpdate.blockers = {"Waiting for API endpoints from backend team"};
    aliceUpdate.plannedWork = {"Implement user dashboard", "Add form validation"};
    
    jimmy.recordDailyUpdate(aliceUpdate);
    std::cout << "✅ Recorded daily update for " << alice.name << std::endl;
    
    DailyUpdate bobUpdate;
    bobUpdate.teamMemberId = bobId;
    bobUpdate.projectId = webAppProject;
    bobUpdate.date = "2024-01-15";
    bobUpdate.summary = "Developed REST API endpoints and database schema";
    bobUpdate.accomplishments = {
        "Created user authentication API",
        "Designed database schema",
        "Set up API documentation"
    };
    bobUpdate.blockers = {};
    bobUpdate.plannedWork = {"Implement data validation", "Add error handling"};
    
    jimmy.recordDailyUpdate(bobUpdate);
    std::cout << "✅ Recorded daily update for " << bob.name << std::endl;
    
    DailyUpdate charlieUpdate;
    charlieUpdate.teamMemberId = charlieId;
    charlieUpdate.projectId = infrastructureProject;
    charlieUpdate.date = "2024-01-15";
    charlieUpdate.summary = "Configured CI/CD pipeline and container orchestration";
    charlieUpdate.accomplishments = {
        "Set up Docker containers for all services",
        "Configured GitHub Actions workflow",
        "Deployed staging environment"
    };
    charlieUpdate.blockers = {"Need access to production AWS account"};
    charlieUpdate.plannedWork = {"Set up monitoring", "Configure auto-scaling"};
    
    jimmy.recordDailyUpdate(charlieUpdate);
    std::cout << "✅ Recorded daily update for " << charlie.name << std::endl;
    
    // Generate project status reports
    std::cout << "\n📊 PROJECT STATUS REPORTS:" << std::endl;
    
    std::string webAppReport = jimmy.generateProjectStatusReport(webAppProject);
    std::cout << "\n" << webAppReport << std::endl;
    
    std::string infrastructureReport = jimmy.generateProjectStatusReport(infrastructureProject);
    std::cout << "\n" << infrastructureReport << std::endl;
    
    // Generate weekly report
    std::cout << "\n📈 WEEKLY SUMMARY REPORT:" << std::endl;
    std::string weeklyReport = jimmy.generateWeeklyReport();
    std::cout << weeklyReport << std::endl;
    
    // Demonstrate team coordination
    std::cout << "\n🔔 CHECK-IN REMINDERS:" << std::endl;
    jimmy.sendCheckinReminder(aliceId, webAppProject);
    jimmy.sendCheckinReminder(bobId, webAppProject);
    jimmy.sendCheckinReminder(charlieId, infrastructureProject);
}

void demonstrateTheOrgManager() {
    printSeparator("THE ORG MANAGER - MULTI-AGENT COORDINATION DEMO");
    
    // Create TheOrgManager
    TheOrgManager manager;
    std::cout << "🏢 Created TheOrg Manager for multi-agent coordination" << std::endl;
    
    // Create agent configurations
    AgentConfig eli5Config;
    eli5Config.agentId = the_org_utils::generateAgentId(AgentRole::COMMUNITY_MANAGER);
    eli5Config.agentName = "Eli5";
    eli5Config.bio = "Community Manager";
    
    AgentConfig eddyConfig;
    eddyConfig.agentId = the_org_utils::generateAgentId(AgentRole::DEVELOPER_RELATIONS);
    eddyConfig.agentName = "Eddy";
    eddyConfig.bio = "Developer Relations";
    
    AgentConfig jimmyConfig;
    jimmyConfig.agentId = the_org_utils::generateAgentId(AgentRole::PROJECT_MANAGER);
    jimmyConfig.agentName = "Jimmy";
    jimmyConfig.bio = "Project Manager";
    
    // Create and add agents
    auto eli5 = std::make_shared<CommunityManagerAgent>(eli5Config);
    auto eddy = std::make_shared<DeveloperRelationsAgent>(eddyConfig);
    auto jimmy = std::make_shared<ProjectManagerAgent>(jimmyConfig);
    
    manager.addAgent(eli5);
    manager.addAgent(eddy);
    manager.addAgent(jimmy);
    
    std::cout << "\n👥 AGENT REGISTRATION:" << std::endl;
    std::cout << "✅ Registered " << eli5->getName() << " as Community Manager" << std::endl;
    std::cout << "✅ Registered " << eddy->getName() << " as Developer Relations" << std::endl;
    std::cout << "✅ Registered " << jimmy->getName() << " as Project Manager" << std::endl;
    
    // Initialize and start all agents
    std::vector<AgentConfig> configs = {eli5Config, eddyConfig, jimmyConfig};
    manager.initializeAllAgents(configs);
    std::cout << "\n🚀 Initialized all agents" << std::endl;
    
    manager.startAllAgents();
    std::cout << "🟢 Started all agents and coordination system" << std::endl;
    
    // Wait a moment for agents to start up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Demonstrate cross-agent communication
    std::cout << "\n💬 CROSS-AGENT COMMUNICATION:" << std::endl;
    
    manager.broadcastMessage(
        "🎉 Welcome to the daily standup! Please share your updates.", 
        "system",
        {}  // Empty means all agents
    );
    std::cout << "📢 Broadcasted standup announcement to all agents" << std::endl;
    
    manager.broadcastMessage(
        "📚 New documentation has been published - please review when possible",
        eddy->getId(),
        {AgentRole::COMMUNITY_MANAGER, AgentRole::PROJECT_MANAGER}
    );
    std::cout << "📢 Eddy notified relevant agents about new documentation" << std::endl;
    
    manager.broadcastMessage(
        "⚠️  High volume of new users joining - please monitor closely",
        eli5->getId(),
        {AgentRole::DEVELOPER_RELATIONS}
    );
    std::cout << "📢 Eli5 alerted developer relations about increased activity" << std::endl;
    
    // Demonstrate agent retrieval by role
    std::cout << "\n🔍 AGENT LOOKUP BY ROLE:" << std::endl;
    auto cmAgent = manager.getAgentByRole(AgentRole::COMMUNITY_MANAGER);
    if (cmAgent) {
        std::cout << "✅ Found Community Manager: " << cmAgent->getName() << std::endl;
    }
    
    auto drAgent = manager.getAgentByRole(AgentRole::DEVELOPER_RELATIONS);
    if (drAgent) {
        std::cout << "✅ Found Developer Relations: " << drAgent->getName() << std::endl;
    }
    
    auto pmAgent = manager.getAgentByRole(AgentRole::PROJECT_MANAGER);
    if (pmAgent) {
        std::cout << "✅ Found Project Manager: " << pmAgent->getName() << std::endl;
    }
    
    // Display system metrics
    std::cout << "\n📊 SYSTEM METRICS:" << std::endl;
    auto metrics = manager.getSystemMetrics();
    std::cout << "  Total Agents: " << metrics.totalAgents << std::endl;
    std::cout << "  Active Agents: " << metrics.activeAgents << std::endl;
    std::cout << "  System Load: " << (metrics.systemLoad * 100) << "%" << std::endl;
    std::cout << "  Avg Response Time: " << metrics.averageResponseTime.count() << "ms" << std::endl;
    std::cout << "  Last Updated: " << the_org_utils::formatTimestamp(metrics.lastUpdated) << std::endl;
    
    // Wait a moment to show coordination in action
    std::cout << "\n⏱️  Coordination system running..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Get updated metrics
    metrics = manager.getSystemMetrics();
    std::cout << "📈 Updated system metrics after coordination cycle" << std::endl;
    
    // Stop all agents
    std::cout << "\n🛑 SHUTDOWN SEQUENCE:" << std::endl;
    manager.stopAllAgents();
    std::cout << "✅ All agents stopped gracefully" << std::endl;
    std::cout << "✅ Coordination system shutdown complete" << std::endl;
}

int main() {
    try {
        // Initialize logging
        AgentLogger logger;
        logger.log("Starting TheOrg demo", "Demo", "Info", LogLevel::INFO);
        
        std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║                     THE ORG - MULTI-AGENT SYSTEM             ║
║                          DEMO APPLICATION                     ║
║                                                              ║
║  A comprehensive demonstration of the ElizaOS C++           ║
║  implementation featuring specialized AI agents for         ║
║  community management, developer relations, and project     ║
║  coordination.                                               ║
╚══════════════════════════════════════════════════════════════╝
        )";
        
        // Demonstrate individual agents
        demonstrateCommunityManager();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demonstrateDeveloperRelations();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        demonstrateProjectManager();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Demonstrate multi-agent coordination
        demonstrateTheOrgManager();
        
        printSeparator("DEMO COMPLETE");
        std::cout << "🎉 TheOrg multi-agent system demonstration completed successfully!" << std::endl;
        std::cout << "💡 This demo showcased:" << std::endl;
        std::cout << "   • Community management with Eli5" << std::endl;
        std::cout << "   • Developer relations with Eddy" << std::endl;
        std::cout << "   • Project management with Jimmy" << std::endl;
        std::cout << "   • Multi-agent coordination system" << std::endl;
        std::cout << "   • Cross-agent communication" << std::endl;
        std::cout << "   • Platform integrations" << std::endl;
        std::cout << "   • Comprehensive reporting" << std::endl;
        std::cout << "\n🚀 Ready for production deployment!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed with error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}