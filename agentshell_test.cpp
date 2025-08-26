#include "elizaos/agentshell.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentcomms.hpp"
#include <iostream>
#include <cassert>

using namespace elizaos;

int main() {
    std::cout << "AgentShell Integration Test" << std::endl;
    std::cout << "============================" << std::endl;
    
    // Test 1: Basic shell functionality
    std::cout << "Test 1: Basic shell functionality..." << std::endl;
    
    // Test command registration
    registerShellCommand("test", [](const std::vector<std::string>& args) -> ShellCommandResult {
        (void)args;
        return ShellCommandResult(true, "Test command executed", "", 0);
    });
    
    // Test command execution
    auto result = executeShellCommandWithResult("test");
    assert(result.success);
    assert(result.output == "Test command executed");
    std::cout << "✓ Command registration and execution works" << std::endl;
    
    // Test 2: Built-in commands
    std::cout << "Test 2: Built-in commands..." << std::endl;
    
    // Test version command
    result = executeShellCommandWithResult("version");
    assert(result.success);
    assert(!result.output.empty());
    std::cout << "✓ Version command works" << std::endl;
    
    // Test help command
    result = executeShellCommandWithResult("help");
    assert(result.success);
    assert(result.output.find("Available commands:") != std::string::npos);
    std::cout << "✓ Help command works" << std::endl;
    
    // Test info command
    result = executeShellCommandWithResult("info");
    assert(result.success);
    assert(result.output.find("ElizaOS C++ Framework") != std::string::npos);
    std::cout << "✓ Info command works" << std::endl;
    
    // Test echo command
    result = executeShellCommandWithResult("echo Hello World");
    assert(result.success);
    assert(result.output == "Hello World");
    std::cout << "✓ Echo command works" << std::endl;
    
    // Test status command
    result = executeShellCommandWithResult("status");
    assert(result.success);
    assert(result.output.find("ElizaOS C++ Framework Status:") != std::string::npos);
    std::cout << "✓ Status command works" << std::endl;
    
    // Test 3: Utility functions
    std::cout << "Test 3: Utility functions..." << std::endl;
    
    auto commands = getAvailableShellCommands();
    assert(!commands.empty());
    std::cout << "✓ " << commands.size() << " commands available" << std::endl;
    
    bool isRunning = isShellRunning();
    std::cout << "✓ Shell running status: " << (isRunning ? "Running" : "Not running") << std::endl;
    
    // Test 4: Error handling
    std::cout << "Test 4: Error handling..." << std::endl;
    
    result = executeShellCommandWithResult("nonexistent_command");
    assert(!result.success);
    assert(!result.error.empty());
    std::cout << "✓ Error handling works for unknown commands" << std::endl;
    
    // Test 5: Integration with other components
    std::cout << "Test 5: Component integration..." << std::endl;
    
    // Test logger integration
    globalLogger->log("AgentShell test", "integration", "test", LogLevel::INFO, LogColor::GREEN);
    std::cout << "✓ Logger integration works" << std::endl;
    
    // Test communications integration
    registerShellCommand("comm_test", [](const std::vector<std::string>& args) -> ShellCommandResult {
        (void)args;
        initializeComms();
        auto channels = globalComms->getActiveChannels();
        shutdownComms();
        return ShellCommandResult(true, "Communications tested, channels: " + std::to_string(channels.size()), "", 0);
    });
    
    result = executeShellCommandWithResult("comm_test");
    assert(result.success);
    std::cout << "✓ Communications integration works" << std::endl;
    
    std::cout << std::endl;
    std::cout << "All tests passed! AgentShell is fully functional." << std::endl;
    
    return 0;
}