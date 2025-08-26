#include "elizaos/agentshell.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentcomms.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace elizaos;

int main() {
    std::cout << "AgentShell Comprehensive Demo" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << std::endl;
    
    // Initialize the logger
    globalLogger->printHeader("AgentShell Demo", LogColor::CYAN);
    
    // Demonstrate all built-in commands
    std::vector<std::string> testCommands = {
        "version",
        "info", 
        "status",
        "help",
        "echo Hello from AgentShell!",
        "unknown_command"  // Test error handling
    };
    
    logInfo("Testing built-in commands...", "demo");
    
    for (const auto& cmd : testCommands) {
        std::cout << std::endl;
        logInfo("Executing: " + cmd, "demo");
        std::cout << "elizaos> " << cmd << std::endl;
        
        auto result = executeShellCommandWithResult(cmd);
        
        if (result.success) {
            if (!result.output.empty()) {
                std::cout << result.output << std::endl;
            }
        } else {
            logError(result.error, "demo");
        }
        
        std::cout << "---" << std::endl;
    }
    
    // Test custom command registration
    std::cout << std::endl;
    logInfo("Testing custom command registration...", "demo");
    
    registerShellCommand("greet", [](const std::vector<std::string>& args) -> ShellCommandResult {
        std::string name = "World";
        if (args.size() > 1) {
            name = args[1];
        }
        return ShellCommandResult(true, "Hello, " + name + "! Welcome to ElizaOS C++.", "", 0);
    });
    
    registerShellCommand("calc", [](const std::vector<std::string>& args) -> ShellCommandResult {
        if (args.size() < 4) {
            return ShellCommandResult(false, "", "Usage: calc <num1> <op> <num2>", 1);
        }
        
        try {
            double a = std::stod(args[1]);
            std::string op = args[2];
            double b = std::stod(args[3]);
            double result = 0;
            
            if (op == "+") result = a + b;
            else if (op == "-") result = a - b;
            else if (op == "*") result = a * b;
            else if (op == "/") {
                if (b == 0) return ShellCommandResult(false, "", "Division by zero", 1);
                result = a / b;
            }
            else return ShellCommandResult(false, "", "Unknown operator: " + op, 1);
            
            return ShellCommandResult(true, std::to_string(result), "", 0);
        } catch (...) {
            return ShellCommandResult(false, "", "Invalid number format", 1);
        }
    });
    
    // Test the custom commands
    std::vector<std::string> customCommands = {
        "greet",
        "greet Alice",
        "calc 10 + 5",
        "calc 20 * 3",
        "calc 100 / 4",
        "calc 5 / 0"  // Test error handling
    };
    
    for (const auto& cmd : customCommands) {
        std::cout << std::endl;
        std::cout << "elizaos> " << cmd << std::endl;
        
        auto result = executeShellCommandWithResult(cmd);
        
        if (result.success) {
            if (!result.output.empty()) {
                std::cout << result.output << std::endl;
            }
        } else {
            logError(result.error, "demo");
        }
    }
    
    // Show available commands
    std::cout << std::endl;
    logInfo("All available commands:", "demo");
    auto commands = getAvailableShellCommands();
    for (const auto& cmd : commands) {
        std::cout << "  " << cmd << std::endl;
    }
    
    std::cout << std::endl;
    logSuccess("AgentShell demo completed successfully!", "demo");
    logInfo("To start interactive shell, use: startInteractiveShell()", "demo");
    
    return 0;
}