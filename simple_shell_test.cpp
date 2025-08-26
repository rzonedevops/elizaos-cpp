#include "elizaos/agentshell.hpp"
#include <iostream>

using namespace elizaos;

int main() {
    std::cout << "Simple AgentShell Test" << std::endl;
    
    // Test basic command execution without starting the shell
    auto result = executeShellCommandWithResult("version");
    std::cout << "Version command result: " << result.success << std::endl;
    std::cout << "Output: " << result.output << std::endl;
    
    // Test echo command
    result = executeShellCommandWithResult("echo test");
    std::cout << "Echo result: " << result.success << std::endl;
    std::cout << "Output: '" << result.output << "'" << std::endl;
    
    // Test unknown command
    result = executeShellCommandWithResult("unknown");
    std::cout << "Unknown command result: " << result.success << std::endl;
    std::cout << "Error: " << result.error << std::endl;
    
    std::cout << "Test completed" << std::endl;
    return 0;
}