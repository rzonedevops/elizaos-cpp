#include <gtest/gtest.h>
#include "elizaos/autonomous_starter.hpp"
#include "elizaos/agentlogger.hpp"
#include <thread>
#include <chrono>

using namespace elizaos;

class AutonomousStarterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize global logger for tests
        if (!globalLogger) {
            globalLogger = std::make_shared<AgentLogger>();
        }
        
        // Create test agent
        agent = createAutolizaAgent();
        ASSERT_NE(agent, nullptr);
    }
    
    void TearDown() override {
        if (agent) {
            agent->stop();
        }
    }
    
    std::shared_ptr<AutonomousStarter> agent;
};

TEST_F(AutonomousStarterTest, AgentCreation) {
    EXPECT_FALSE(agent->isRunning());
    EXPECT_EQ(agent->getConfig().agentName, "Autoliza");
    EXPECT_FALSE(agent->getConfig().agentId.empty());
    EXPECT_FALSE(agent->getConfig().bio.empty());
}

TEST_F(AutonomousStarterTest, StartStop) {
    EXPECT_FALSE(agent->isRunning());
    
    agent->start();
    EXPECT_TRUE(agent->isRunning());
    
    agent->stop();
    EXPECT_FALSE(agent->isRunning());
}

TEST_F(AutonomousStarterTest, ShellCommandExecution) {
    agent->start();
    
    // Test successful command
    auto result = agent->executeShellCommand("echo 'Hello World'");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_FALSE(result.output.empty());
    EXPECT_TRUE(result.output.find("Hello World") != std::string::npos);
    
    // Test command that should fail
    result = agent->executeShellCommand("nonexistentcommand12345");
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.exitCode, 0);
}

TEST_F(AutonomousStarterTest, ForbiddenCommands) {
    agent->start();
    
    // Test forbidden command filtering
    auto result = agent->executeShellCommand("rm -rf /");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.exitCode, -1);
    EXPECT_FALSE(result.error.empty());
    EXPECT_TRUE(result.error.find("forbidden") != std::string::npos);
}

TEST_F(AutonomousStarterTest, ShellAccessControl) {
    agent->start();
    
    // Test normal access
    auto result = agent->executeShellCommand("pwd");
    EXPECT_TRUE(result.success);
    
    // Disable shell access
    agent->enableShellAccess(false);
    result = agent->executeShellCommand("pwd");
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.error.find("disabled") != std::string::npos);
    
    // Re-enable shell access
    agent->enableShellAccess(true);
    result = agent->executeShellCommand("pwd");
    EXPECT_TRUE(result.success);
}

TEST_F(AutonomousStarterTest, AutonomousLoop) {
    agent->start();
    
    EXPECT_FALSE(agent->isAutonomousLoopRunning());
    
    // Start autonomous loop
    agent->startAutonomousLoop();
    EXPECT_TRUE(agent->isAutonomousLoopRunning());
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    
    // Stop autonomous loop
    agent->stopAutonomousLoop();
    EXPECT_FALSE(agent->isAutonomousLoopRunning());
}

TEST_F(AutonomousStarterTest, LoopIntervalConfiguration) {
    agent->start();
    
    // Test interval setting
    agent->setLoopInterval(std::chrono::milliseconds(500));
    EXPECT_EQ(agent->getLoopInterval().count(), 500);
    
    agent->setLoopInterval(std::chrono::milliseconds(2000));
    EXPECT_EQ(agent->getLoopInterval().count(), 2000);
}

TEST_F(AutonomousStarterTest, TaskBasedExecution) {
    agent->start();
    
    // Test task-based shell command execution
    UUID taskId = agent->executeShellCommandAsTask("echo 'Task Test'");
    EXPECT_FALSE(taskId.empty());
    
    // Give task time to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    // Task should have been processed by now
    // (In a real implementation, we'd check task status)
}

TEST_F(AutonomousStarterTest, MemorySystem) {
    agent->start();
    
    const auto& state = agent->getState();
    size_t initialMemoryCount = state.getRecentMessages().size();
    
    // Execute a command that should create a memory
    agent->executeShellCommand("echo 'Memory Test'");
    
    // Check that memory was created
    EXPECT_GT(state.getRecentMessages().size(), initialMemoryCount);
    
    // Check memory content
    const auto& recentMemories = state.getRecentMessages();
    bool foundCommandMemory = false;
    for (const auto& memory : recentMemories) {
        if (memory->getContent().find("echo 'Memory Test'") != std::string::npos) {
            foundCommandMemory = true;
            break;
        }
    }
    EXPECT_TRUE(foundCommandMemory);
}

TEST_F(AutonomousStarterTest, WorkingDirectoryTracking) {
    agent->start();
    
    // Get initial working directory
    auto result = agent->executeShellCommand("pwd");
    EXPECT_TRUE(result.success);
    std::string initialDir = result.output;
    
    // Should match getCurrentWorkingDirectory
    std::string currentDir = agent->getCurrentWorkingDirectory();
    EXPECT_FALSE(currentDir.empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}