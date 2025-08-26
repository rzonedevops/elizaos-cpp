#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

namespace elizaos {

/**
 * Command result structure
 */
struct ShellCommandResult {
    bool success;
    std::string output;
    std::string error;
    int exitCode;
    
    ShellCommandResult(bool success = true, const std::string& output = "", 
                      const std::string& error = "", int exitCode = 0)
        : success(success), output(output), error(error), exitCode(exitCode) {}
};

/**
 * Command handler function type
 */
using CommandHandler = std::function<ShellCommandResult(const std::vector<std::string>&)>;

/**
 * AgentShell - Interactive shell interface for agent control
 * 
 * Provides command-line interaction capabilities for controlling
 * and interacting with ElizaOS agents
 */
class AgentShell {
public:
    AgentShell();
    ~AgentShell();
    
    /**
     * Start the interactive shell
     * @param prompt Custom prompt string (optional)
     */
    void start(const std::string& prompt = "elizaos> ");
    
    /**
     * Stop the interactive shell
     */
    void stop();
    
    /**
     * Execute a single command
     * @param command Command string to execute
     * @return Command result
     */
    ShellCommandResult executeCommand(const std::string& command);
    
    /**
     * Register a custom command handler
     * @param commandName Name of the command
     * @param handler Function to handle the command
     */
    void registerCommand(const std::string& commandName, CommandHandler handler);
    
    /**
     * Unregister a command handler
     * @param commandName Name of the command to remove
     */
    void unregisterCommand(const std::string& commandName);
    
    /**
     * Get list of available commands
     */
    std::vector<std::string> getAvailableCommands() const;
    
    /**
     * Set shell prompt
     * @param prompt New prompt string
     */
    void setPrompt(const std::string& prompt);
    
    /**
     * Enable/disable command history
     * @param enabled Whether to keep command history
     */
    void setHistoryEnabled(bool enabled);
    
    /**
     * Get command history
     */
    const std::vector<std::string>& getHistory() const;
    
    /**
     * Clear command history
     */
    void clearHistory();
    
    /**
     * Check if shell is running
     */
    bool isRunning() const { return running_; }
    
private:
    /**
     * Main shell loop
     */
    void shellLoop();
    
    /**
     * Parse command line into tokens
     * @param command Command string
     * @return Vector of command tokens
     */
    std::vector<std::string> parseCommand(const std::string& command);
    
    /**
     * Initialize built-in commands
     */
    void initializeBuiltinCommands();
    
    /**
     * Built-in command handlers
     */
    ShellCommandResult helpCommand(const std::vector<std::string>& args);
    ShellCommandResult exitCommand(const std::vector<std::string>& args);
    ShellCommandResult historyCommand(const std::vector<std::string>& args);
    ShellCommandResult clearCommand(const std::vector<std::string>& args);
    ShellCommandResult echoCommand(const std::vector<std::string>& args);
    ShellCommandResult statusCommand(const std::vector<std::string>& args);
    ShellCommandResult versionCommand(const std::vector<std::string>& args);
    ShellCommandResult infoCommand(const std::vector<std::string>& args);
    
    // Member variables
    std::atomic<bool> running_;
    std::string prompt_;
    bool historyEnabled_;
    std::vector<std::string> commandHistory_;
    std::unordered_map<std::string, CommandHandler> commandHandlers_;
    std::unique_ptr<std::thread> shellThread_;
    mutable std::mutex commandsMutex_;
    mutable std::mutex historyMutex_;
};

/**
 * Global shell instance for convenience
 */
extern std::shared_ptr<AgentShell> globalShell;

/**
 * Convenience functions for shell operations
 */
void startInteractiveShell();
void stopInteractiveShell();
bool executeShellCommand(const std::string& command);
void registerShellCommand(const std::string& name, CommandHandler handler);

/**
 * Execute a command and get the result
 * @param command Command string to execute
 * @return Command result structure
 */
ShellCommandResult executeShellCommandWithResult(const std::string& command);

/**
 * Check if interactive shell is currently running
 * @return true if shell is running, false otherwise
 */
bool isShellRunning();

/**
 * Get list of all available commands
 * @return Vector of command names
 */
std::vector<std::string> getAvailableShellCommands();

} // namespace elizaos